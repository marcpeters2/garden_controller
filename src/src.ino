#include <string.h>
#include <SPI.h>
#include "GCHttpClient.h"
#include "WifiService.h"
#include "Util.h"

#define GET "GET"
#define POST "POST"

#define MAC_ADDRESS_NUM_BYTES 6

#define MAX_COMMANDS_PER_OUTLET 5


httpServer_t server = {
  "192.168.43.124",
  8000,
};

httpEndpoint_t postControllers = {
  POST,
  "/controllers",
};

httpEndpoint_t getTimeRequest = {
  GET,
  "/time",
};

enum controllerState_t {
  INITIAL,
  RECEIVED_ID,
  SYNCED_TIME,
  RECEIVED_COMMANDS,
  FETCH_COMMANDS_FAILURE
};

controllerState_t controllerState = INITIAL;
int myId;
unsigned long long timeOffset;

enum outletType_t {
  ELECTRIC,
  HYDRAULIC
};

enum outletState_t {
  OFF = 0,
  ON = 1
};

struct command_t {
  outletState_t outletState;
  unsigned long long executeAt;
};

struct outlet_t {
  String internalName;
  int pin;
  int numCommands;
  int numNextCommands;
  command_t commands[MAX_COMMANDS_PER_OUTLET];
  command_t nextCommands[MAX_COMMANDS_PER_OUTLET];
};

outlet_t outlets[] = {
  { "A", 0, 0, 0 },
  { "B", 1, 0, 0 },
  { "C", 2, 0, 0 },
  { "D", 3, 0, 0 },
};

const int numOutlets = sizeof(outlets) / sizeof(outlet_t);

enum commandStringParserState_t {
  PARSING_OUTLET_INTERNAL_ID,
  PARSING_TIMESTAMP,
  PARSING_OUTLET_STATE
};

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Util::printWelcomeMessage();
  Serial.print("Number of outlets: ");
  Serial.println(numOutlets);

  initializeOutletPins();

  delay(3000);

  //WifiService::connectToWiFi("Pudding", "vanilla864");
  WifiService::connectToWiFi("OnePlus", "qwertyui");
}

void loop() {

  int requestDelay;
  
  switch(controllerState) {
    case INITIAL:
      myId = getMyId();
      break;
    case RECEIVED_ID:
      timeOffset = getServerTime();
      timeOffset -= Util::now(0);
      break;
    case SYNCED_TIME:
      getCommands();
      break;
    case RECEIVED_COMMANDS:
      getCommands();
      executeCommands();
      break;
    case FETCH_COMMANDS_FAILURE:
      //TODO: Aggressively retry fetching commands here
      getCommands();
      break;
  }

  //requestDelay = rand() % 3000 + 1000;
  requestDelay = 1000;
  Serial.println();
  Serial.print(">>> Delay: ");
  Serial.print(requestDelay);
  Serial.println("ms");
  delay(requestDelay);
}

int getMyId() {

  int id;
  byte mac[6];
  char macStringBuffer [MAC_ADDRESS_NUM_BYTES * 2 + 1];  
  macStringBuffer[MAC_ADDRESS_NUM_BYTES * 2] = 0;
  WiFi.macAddress(mac);

  String payload = "{";
  payload += "\"MAC\":\"";
  for (short i = 0; i < MAC_ADDRESS_NUM_BYTES; i++) {
    sprintf(&macStringBuffer[2*i], "%02X", mac[MAC_ADDRESS_NUM_BYTES - 1 - i]);
  }
  payload += macStringBuffer;
  payload += "\",";
  payload += "\"electric\":{";
  for (short i = 0; i < sizeof(outlets)/sizeof(*outlets); i++) {
    if (i > 0) {
      payload += ",";
    }
    payload += "\"";
    payload += i;
    payload += "\":\"";
    payload += outlets[i].internalName;
    payload += "\"";
  }
  payload += "}}";
  
  httpResponse_t httpResponse;

  do {
    GCHttpClient::httpRequest(&server, &postControllers, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  id = Util::parseIntFromString(httpResponse.response);
  Serial.println();
  Serial.print(">>> Setting my id to ");
  Serial.println(id);

  controllerState = RECEIVED_ID;

  return id;
}

unsigned long long getServerTime() {

  unsigned long requestStart = Util::now(0);
  unsigned long long serverTimestamp;
  String payload = "";

  httpResponse_t httpResponse;

  do {
    GCHttpClient::httpRequest(&server, &getTimeRequest, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  unsigned long requestStop = Util::now(0);
  unsigned long long estimatedLag = (requestStop - requestStart) / 2;

  serverTimestamp = (unsigned long long) Util::parseULongLongFromString(httpResponse.response) + estimatedLag;
  Serial.print("Received current timestamp: ");
  Serial.println(httpResponse.response);
  Serial.print("Estimated lag: ");
  Serial.println(Util::toString(estimatedLag));
  Serial.print("Adjusted timestamp: ");
  Serial.println(Util::toString(serverTimestamp));

  controllerState = SYNCED_TIME;
  
  return serverTimestamp;
}

void getCommands() {

  String payload = "";
  String path = (String) "/controllers/" + myId + "/commands";

  httpEndpoint_t getCommandRequest = {
    GET,
    path,
  };

  httpResponse_t httpResponse;

  do {
    GCHttpClient::httpRequest(&server, &getCommandRequest, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

//  Serial.print("Received commands: ");
//  Serial.println(httpResponse.response);

  parseCommandString(httpResponse.response);

  return;
}


void parseCommandString(char* commandString) {
  const int maxBufferSize = 25;
  String stringBuf = "";
  char charArrayBuf[maxBufferSize + 1];
  char c;

  int outletPin;
  unsigned long long timestamp;
  int outletStateInt;
  outletState_t outletState;
  
  int index = 0;
  int commandNum = 0;
  commandStringParserState_t state = PARSING_OUTLET_INTERNAL_ID;
  bool error = false;

  resetNextCommandsForAllOutlets();

  while(index <= strlen(commandString) && !error) {
    c = commandString[index];
    
    switch(state) {
      case PARSING_OUTLET_INTERNAL_ID:
          if(Util::charIsNumeric(c)) {
            stringBuf = stringBuf + c;
          }
          else if(c == ',') {
            stringBuf.toCharArray(charArrayBuf, sizeof(charArrayBuf));
            outletPin = Util::parseIntFromString(charArrayBuf);
            stringBuf = "";
            state = PARSING_TIMESTAMP;
          } 
          else {
            Serial.print("!!! Encountered unexpected character in command string while parsing OUTLET_INTERNAL_ID: ");
            Serial.println(c);
            error = true;
            break;
          }
        break;
        
      case PARSING_TIMESTAMP:
        if(Util::charIsNumeric(c)) {
            stringBuf = stringBuf + c;
          }
          else if(c == ',') {
            stringBuf.toCharArray(charArrayBuf, sizeof(charArrayBuf));
            timestamp = Util::parseULongLongFromString(charArrayBuf);
            stringBuf = "";
            state = PARSING_OUTLET_STATE;
          } 
          else {
            Serial.print("!!! Encountered unexpected character in command string while parsing TIMESTAMP: ");
            Serial.println(c);
            error = true;
            break;
          }
        break;
        
      case PARSING_OUTLET_STATE:
          if(Util::charIsNumeric(c)) {
            stringBuf = stringBuf + c;
          }
          else if(c == ',') {
            stringBuf.toCharArray(charArrayBuf, sizeof(charArrayBuf));
            outletStateInt = Util::parseIntFromString(charArrayBuf);
            if(outletStateInt == OFF) {
              outletState = OFF;
            }
            else if(outletStateInt == ON) {
              outletState = ON;
            }
            else {
              Serial.print("!!! Encountered invalid outlet state in command string: ");
              Serial.println(outletStateInt);
              error = true;
              break;
            }
            stringBuf = "";
            state = PARSING_TIMESTAMP;
            error = appendOutletNextCommand(outletPin, {outletState, timestamp});
            if(error) { break; }
            
            Serial.print("### Parsed command for outletInternalId: ");
            Serial.print(outletPin);
            Serial.print(", timestamp: ");
            Serial.print(Util::toString(timestamp));
            Serial.print(", state: ");
            Serial.println(outletStateInt);
            commandNum++;
          } 
          else if(c == '\n' || c == '\0') {
            stringBuf.toCharArray(charArrayBuf, sizeof(charArrayBuf));
            outletStateInt = Util::parseIntFromString(charArrayBuf);
            if(outletStateInt == OFF) {
              outletState = OFF;
            }
            else if(outletStateInt == ON) {
              outletState = ON;
            }
            else {
              Serial.print("!!! Encountered invalid outlet state in command string: ");
              Serial.println(outletStateInt);
              error = true;
              break;
            }
            
            stringBuf = "";
            state = PARSING_OUTLET_INTERNAL_ID;\
            error = appendOutletNextCommand(outletPin, {outletState, timestamp});
            if(error) { break; }
            
            Serial.print("### Parsed command for outletInternalId: ");
            Serial.print(outletPin);
            Serial.print(", timestamp: ");
            Serial.print(Util::toString(timestamp));
            Serial.print(", state: ");
            Serial.println(outletStateInt);
            commandNum = 0;
          } 
          else {
            Serial.print("!!! Encountered unexpected character in command string while parsing OUTLET_STATE: ");
            Serial.println(c);
            error = true;
            break;
          }
        break;
    }

    if(stringBuf.length() > maxBufferSize) {
      Serial.println("!!! Error parsing command string.  Temp buffer is full.");
      error = true;
      break;
    }

    if(error) {
      break;
    }

    index++;
  }

  if(error) {
    controllerState = FETCH_COMMANDS_FAILURE;
  }
  else {
    useNextCommands();
    controllerState = RECEIVED_COMMANDS;  
  }
  
}

void executeCommands() {
  unsigned long long now = Util::now(timeOffset);
  Serial.print("Now: ");
  Serial.println(Util::toString(now));
  int outletPin;
  outletState_t desiredOutletState;
  int pinState = LOW;
  command_t command;

  Serial.println();
  
  for(int i = 0; i < numOutlets; i++) {
    outletPin = outlets[i].pin;
    desiredOutletState = OFF;
    
    for(int j = 0; j < outlets[i].numCommands; j++) {
      command = outlets[i].commands[j];
      if(command.executeAt < now) {
        desiredOutletState = command.outletState;
      }
    }

    if(desiredOutletState == ON) {
      pinState = HIGH;
    }
    else {
      pinState = LOW;
    }

    
    Serial.print(">>> Pin ");
    Serial.print(outletPin);
    if(pinState == HIGH) {
      Serial.println(" ON");
    }
    else {
      Serial.println(" OFF");
    }
    
    digitalWrite(outletPin, pinState);
  }
}

void resetNextCommandsForOutlet(int outletPin) {
  for(int i = 0; i < numOutlets; i++) {
    if(outlets[i].pin == outletPin) {
      outlets[i].numNextCommands = 0;
    }
  }
}

void resetNextCommandsForAllOutlets() {
  for(int i = 0; i < numOutlets; i++) {
    outlets[i].numNextCommands = 0;
  }
}

bool appendOutletNextCommand(int outletPin, command_t command) {
  int numNextCommands;
  bool error = false;
  
  for(int i = 0; i < numOutlets; i++) {
    if(outlets[i].pin == outletPin) {
      numNextCommands = outlets[i].numNextCommands;
//      Serial.print("### Current outlet commands: ");
//      Serial.println(numCommands);

      if(numNextCommands >= MAX_COMMANDS_PER_OUTLET) {
        Serial.print("!!! Can't append next command to outlet ");
        Serial.print(outletPin);
        Serial.println("; max commands reached");
        error = true;
        break;
      }
      
      outlets[i].nextCommands[numNextCommands] = command;
      outlets[i].numNextCommands++;
    }
  }

  return error;
}

void useNextCommands() {
  for(int i = 0; i < numOutlets; i++) {
    for(int j = 0; j < outlets[i].numNextCommands; j++) {
      outlets[i].commands[j] = outlets[i].nextCommands[j];
    }
    outlets[i].numCommands = outlets[i].numNextCommands;
  }
}

void initializeOutletPins() {
  Serial.println();
  Serial.println(">>> Initializing outlet pins");
  for(int i = 0; i < numOutlets; i++) {
    Serial.print("Setting pin ");
    Serial.print(outlets[i].pin);
    Serial.println(" to OUTPUT");
    pinMode(outlets[i].pin, OUTPUT);
    digitalWrite(outlets[i].pin, LOW);
  }
  Serial.println();
}




