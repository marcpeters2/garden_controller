#include "GCHttpClient.h"

WiFiClient client;

void GCHttpClient::httpRequest(httpServer_t* server, httpEndpoint_t* endpoint, String payload, httpResponse_t* httpResponse) {
  bool payloadExists = payload.length() > 0;
  HttpParser* httpParser = new HttpParser(httpResponse);;
  bool parseError;
  bool timeout;

  bool sentRequest;
  bool receivedResponse;
  bool done;

  unsigned long long lastRequestTime = 0;            // last time you connected to the server, in milliseconds
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
//      Serial.print("-----------------------");
//      Serial.println();
//      Serial.print("Making HTTP request ");
      Serial.print(">>> ");
      Serial.print(endpoint->method);
      Serial.print(" ");
      Serial.print(server->host);
      Serial.print(":");
      Serial.print(server->port);
      Serial.println(endpoint->path);
      if (payloadExists) {
        Serial.println(payload);
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
        lastRequestTime = Util::ucNow();
      }
      else {
        // if you couldn't make a connection:
        Serial.println();
        Serial.print("!!! HTTP request ");
        Serial.print(endpoint->method);
        Serial.print(" ");
        Serial.print(endpoint->path);
        Serial.println(" failed.  Couldn't connect.");
        delay(2000);
      }
    }

    httpParser->reset();
  
    while (!(Util::ucNow() - lastRequestTime > requestTimeout)) {
      while (client.available() && !parseError) {
        receivedResponse = true;
        char c = client.read();
        parseError = httpParser->parse(c);
      }

      if ((receivedResponse && !client.connected()) || parseError) {
        break;
      }
    }

    if (Util::ucNow() - lastRequestTime > requestTimeout) {
      timeout = true;
      Serial.println();
      Serial.println("!!! Request timeout");
      Serial.print("Free memory: ");
      Serial.print(Util::freeRAM());
      Serial.println("B");
    }

    if (parseError || timeout) {
      delay(2000);
    }
    else {
      done = true;
      break;
    }
  }

  Serial.println("### Response: ");
  Serial.print("Status ");
  Serial.println(httpResponse->statusCode);
  Serial.print(httpResponse->response);
  Serial.println();
  Serial.print("### Response buffer used: ");
  Serial.print((float)httpResponse->responseSize / (float)sizeof(httpResponse->response) * 100);
  Serial.println("%");
  Serial.print("### Free memory: ");
  Serial.print(Util::freeRAM());
  Serial.println("B");

  delete httpParser;

  return;
}
