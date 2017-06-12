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


WiFiClient client;

const char server[] = "thawing-journey-12821.herokuapp.com";
//const char server[] = "httpbin.org";

enum controllerState_t {
  INITIAL,
  RECEIVED_ID
};

controllerState_t controllerState = INITIAL;
int myId;

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  WifiService::connectToWiFi("Pudding", "vanilla864");
}

void loop() {

  switch(controllerState) {
    case INITIAL:
      myId = getMyId();
      break;
    case RECEIVED_ID:

      break;
  }

  Serial.println("Delay: ");
  Serial.println(rand() % 1000 + 1000);
  delay(rand() % 2000 + 2000);
}

int getMyId() {

  httpResponse_t httpResponse;
  httpRequest(GET, "/", &httpResponse);

  return 1;
}

// this method makes a HTTP connection to the server:
void httpRequest(const char* method, const char* path, httpResponse_t* httpResponse) {
  HttpParser* httpParser;
  bool parseError;
  bool timeout;

  bool sentRequest;
  bool receivedResponse;
  bool done;

  unsigned long lastRequestTime = 0;            // last time you connected to the server, in milliseconds
  const unsigned long requestTimeout = 10L * 1000L; // delay between updates, in milliseconds

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
      Serial.print(method);
      Serial.print(" ");
      Serial.print(server);
      Serial.println(path);
      Serial.println();
    
      // if there's a successful connection:
      if (client.connect(server, 80)) {
        client.print(method);
        client.print(" ");
        client.print(path);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(server);
        client.println("User-Agent: ArduinoWiFi/1.1");
        client.println("Connection: close");
        client.println();
  
        sentRequest = true;
        lastRequestTime = millis();
      }
      else {
        // if you couldn't make a connection:
        Serial.print("HTTP request ");
        Serial.print(method);
        Serial.print(" ");
        Serial.print(path);
        Serial.println(" failed.  Couldn't connect.");
        Serial.print("-----------------------");
        Serial.println();
        delay(2000);
      }
    }

    httpParser = new HttpParser(httpResponse);
  
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
  Serial.print("-----------------------");
  Serial.println();

  return;
}

