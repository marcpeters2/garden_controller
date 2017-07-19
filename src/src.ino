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
  RECEIVED_COMMANDS
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
  command_t commands[MAX_COMMANDS_PER_OUTLET];
};

const outlet_t outlets[] = {
  { "A", 1 },
  { "B", 2 },
  { "C", 3 },
  { "D", 4 },
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
  Serial.println();

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
      break;
    case SYNCED_TIME:
      getCommands();
      break;
  }

  //requestDelay = rand() % 3000 + 1000;
  requestDelay = 2000;
  Serial.print("Delay: ");
  Serial.println(requestDelay);
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
  Serial.print("Received controller id: ");
  Serial.println(id);

  controllerState = RECEIVED_ID;

  return id;
}

unsigned long long getServerTime() {

  unsigned long requestStart = millis();
  unsigned long long serverTimestamp;
  String payload = "";

  httpResponse_t httpResponse;

  do {
    GCHttpClient::httpRequest(&server, &getTimeRequest, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  unsigned long requestStop = millis();
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

  Serial.print("Received commands: ");
  Serial.println(httpResponse.response);

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
            Serial.print("Encountered unexpected character in command string while parsing OUTLET_INTERNAL_ID: ");
            Serial.println(c);
            error = true;
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
            Serial.print("Encountered unexpected character in command string while parsing TIMESTAMP: ");
            Serial.println(c);
            error = true;
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
              Serial.print("Encountered invalid outlet state in command string: ");
              Serial.println(outletStateInt);
              error = true;
            }
            stringBuf = "";
            state = PARSING_TIMESTAMP;
            // TODO: Add command to internal data structure here
            Serial.print("Parsed command for outletInternalId: ");
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
              Serial.print("Encountered invalid outlet state in command string: ");
              Serial.println(outletStateInt);
              error = true;
            }
            stringBuf = "";
            state = PARSING_OUTLET_INTERNAL_ID;
            // TODO: Add command to internal data structure here
            Serial.print("Parsed command for outletInternalId: ");
            Serial.print(outletPin);
            Serial.print(", timestamp: ");
            Serial.print(Util::toString(timestamp));
            Serial.print(", state: ");
            Serial.println(outletStateInt);
            commandNum = 0;
          } 
          else {
            Serial.print("Encountered unexpected character in command string while parsing OUTLET_STATE: ");
            Serial.println(c);
            error = true;
          }
        break;
    }

    if(stringBuf.length() > maxBufferSize) {
      Serial.println("Error parsing command string.  Temp buffer is full.");
      error = true;
    }

    if(error) {
      break;
    }

    index++;
  }
  
}




