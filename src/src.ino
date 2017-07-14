/*

 This example connects to an unencrypted WiFi network.
 Then it prints the  MAC address of the WiFi shield,
 the IP address obtained, and other network details.

 Circuit:
 * WiFi shield attached

 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 31 May 2012
 by Tom Igoe
 */
#include <string.h>
#include <SPI.h>
#include <WiFi101.h>
#include "HttpParser.h"
#include "WifiService.h"

#define GET "GET"
#define POST "POST"

#define ENDPOINT_REGISTER_CONTROLLER "/controllers"

#define MAC_ADDRESS_NUM_BYTES 6

struct httpServer_t {
  String host;
  int port;
};

httpServer_t server = {
  "192.168.1.193",
  8000,
};

struct httpEndpoint_t {
  String method;
  String path;
};

httpEndpoint_t postControllers = {
  POST,
  "/controllers",
};

httpEndpoint_t getTimeRequest = {
  GET,
  "/time",
};

WiFiClient client;

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

struct outlet_t {
  String internalName;
};

const outlet_t outlets[] = {
  { "A" },
  { "B" },
  { "C" },
  { "D" },
};

extern "C" char *sbrk(int i);
size_t freeRAM(void)
{
  char stack_dummy = 0;
  return(&stack_dummy - sbrk(0));
}

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  delay(3000);

  WifiService::connectToWiFi("Pudding", "vanilla864");
  //WifiService::connectToWiFi("OnePlus", "qwertyui");
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

  requestDelay = rand() % 3000 + 1000;
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
    httpRequest(&server, &postControllers, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  id = parseIntFromString(httpResponse.response);
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
    httpRequest(&server, &getTimeRequest, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  unsigned long requestStop = millis();
  unsigned long estimatedLag = (requestStop - requestStart) / 2;

  serverTimestamp = parseULongLongFromString(httpResponse.response) + estimatedLag;
  Serial.print("Received current timestamp: ");
  Serial.println(httpResponse.response);
  Serial.print("Estimated lag: ");
  Serial.println(estimatedLag);
//  Serial.print("Adjusted timestamp: ");
//  Serial.println((String)"" + serverTimestamp);

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
    httpRequest(&server, &getCommandRequest, payload, &httpResponse);
  } while(httpResponse.statusCode != 200);

  Serial.print("Received commands: ");
  Serial.println(httpResponse.response);

  return;
}

// this method makes a HTTP connection to the server:
void httpRequest(httpServer_t* server, httpEndpoint_t* endpoint, String payload, httpResponse_t* httpResponse) {
  bool payloadExists = payload.length() > 0;
  HttpParser* httpParser = new HttpParser(httpResponse);;
  bool parseError;
  bool timeout;

  bool sentRequest;
  bool receivedResponse;
  bool done;

  unsigned long lastRequestTime = 0;            // last time you connected to the server, in milliseconds
  const unsigned long requestTimeout = 4L * 1000L; // delay between updates, in milliseconds

  while (!done) {

    sentRequest = false;
    receivedResponse = false;
    parseError = false;
    timeout = false;

    while (!sentRequest) {
      // close any connection before send a new request.
      // This will free the socket on the WiFi shield
      client.stop();

      Serial.println();
      Serial.print("-----------------------");
      Serial.println();
      Serial.print("Making HTTP request ");
      Serial.print(endpoint->method);
      Serial.print(" ");
      Serial.print(server->host);
      Serial.print(":");
      Serial.print(server->port);
      Serial.println(endpoint->path);
      Serial.println();
      if (payloadExists) {
        Serial.println(payload);
        Serial.println();
      }
    
      // if there's a successful connection:
      if (client.connect(server->host.c_str(), server->port)) {
        client.print(endpoint->method);
        client.print(" ");
        client.print(endpoint->path);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.print(server->host);
        client.print(":");
        client.println(server->port);
        client.println("Content-Type: application/json");
        client.println("Accept: text/csv");
        if (payloadExists) {
          client.print("Content-Length: ");
          client.println(payload.length());
        }
        client.println("User-Agent: ArduinoWiFi/1.1");
        client.println("Connection: close");
        client.println();
        if (payloadExists) {
          client.println(payload.c_str());
        }
  
        sentRequest = true;
        lastRequestTime = millis();
      }
      else {
        // if you couldn't make a connection:
        Serial.print("HTTP request ");
        Serial.print(endpoint->method);
        Serial.print(" ");
        Serial.print(endpoint->path);
        Serial.println(" failed.  Couldn't connect.");
        Serial.print("-----------------------");
        Serial.println();
        delay(2000);
      }
    }

    httpParser->reset();
  
    while (!(millis() - lastRequestTime > requestTimeout)) {
      while (client.available() && !parseError) {
        receivedResponse = true;
        char c = client.read();
        parseError = httpParser->parse(c);
      }

      if ((receivedResponse && !client.connected()) || parseError) {
        break;
      }
    }

    if (millis() - lastRequestTime > requestTimeout) {
      timeout = true;
      Serial.println("Request timeout");
      Serial.print("Free memory: ");
      Serial.print(freeRAM());
      Serial.println("B");
      Serial.print("-----------------------");
      Serial.println();
    }

    if (parseError || timeout) {
      delay(2000);
    }
    else {
      done = true;
      break;
    }
  }

  Serial.println("Response: ");
  Serial.print("Status ");
  Serial.println(httpResponse->statusCode);
  Serial.print(httpResponse->response);
  Serial.println();
  Serial.print("Response buffer used: ");
  Serial.print((float)httpResponse->responseSize / (float)sizeof(httpResponse->response) * 100);
  Serial.println("%");
  Serial.print("Free memory: ");
  Serial.print(freeRAM());
  Serial.println("B");
  Serial.print("-----------------------");
  Serial.println();

  delete httpParser;

  return;
}


int parseIntFromString(char* buf) {
  int index = 0;
  int charAsInt;
  int bufferSize = sizeof(buf) / sizeof(char);
  int result = 0;

  while (index < bufferSize) {
    charAsInt = charToInt(buf[index]);

    if (charAsInt < 0 || charAsInt > 9) {
      break;
    }

    result = result * 10 + charAsInt;
    index++;
  }
  return result;
}

unsigned long long parseULongLongFromString(char* buf) {
  int index = 0;
  int charAsInt;
  int bufferSize = sizeof(buf) / sizeof(char);
  unsigned long long result = 0;

  while (index < bufferSize) {
    charAsInt = charToInt(buf[index]);

    if (charAsInt < 0 || charAsInt > 9) {
      break;
    }

    result = result * 10 + charAsInt;
    index++;
  }
  return result;
}


int charToInt(char c) {
  return c - '0';
}

