#include "GCHttpClient.h"

#define CONNECTION_RETRY_DELAY 2000

WiFiClient client;
bool _DEBUG_HTTP_CALLS = true;

void GCHttpClient::debugHttpCalls(bool val) {
  _DEBUG_HTTP_CALLS = val;
}

void GCHttpClient::httpRequest(httpServer_t* server, httpEndpoint_t* endpoint, String payload, httpResponse_t* httpResponse, const char** error, unsigned int maxRetries = 1) {
  bool payloadExists = payload.length() > 0;
  HttpParser* httpParser = new HttpParser(httpResponse);;
  bool parseError;
  bool timeout;

  bool sentRequest;
  bool receivedResponse;
  bool done;

  unsigned int numConnectionAttempts = 0;
  unsigned long long lastRequestTime = 0;            // last time you connected to the server, in milliseconds
  const unsigned long requestTimeout = 4L * 1000L;

  //client.setTimeout(1);

  while (!done) {

    sentRequest = false;
    receivedResponse = false;
    parseError = false;
    timeout = false;

    while (!sentRequest) {

      if(_DEBUG_HTTP_CALLS) {
        Serial.println();
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
        lastRequestTime = TimeService::ucNow();
      }
      else {
        // if you couldn't make a connection:
        Serial.println();
        Serial.print("!!! HTTP request ");
        Serial.print(endpoint->method);
        Serial.print(" ");
        Serial.print(endpoint->path);
        Serial.println(" failed.  Couldn't connect.");
        Serial.print("Free memory: ");
        Serial.print(Util::freeRAM());
        Serial.println("B");

        numConnectionAttempts++;

        if (numConnectionAttempts >= maxRetries) {
          Serial.println("Max retries reached.  Cancelling HTTP request.");
          *error = "Unable to connect to server";
          goto cleanup;
        }
        
        delay(CONNECTION_RETRY_DELAY);
      }
    }

    httpParser->reset();
  
    while (!(TimeService::ucNow() - lastRequestTime > requestTimeout)) {
      while (client.available() && !parseError) {
        receivedResponse = true;
        char c = client.read();
        parseError = httpParser->parse(c);
      }

      if ((receivedResponse && !client.connected()) || parseError) {
        break;
      }

      delay(10); //Allow other code ie. software serial time to run
    }

    if (TimeService::ucNow() - lastRequestTime > requestTimeout) {
      timeout = true;
      Serial.println();
      Serial.println("!!! Request timeout");
      Serial.print("Free memory: ");
      Serial.print(Util::freeRAM());
      Serial.println("B");
    }

    if (parseError || timeout) {
      numConnectionAttempts++;
      
      if (numConnectionAttempts >= maxRetries) {
        Serial.println("Max retries reached.  Cancelling HTTP request.");
          *error = "Error retrieving response from server (parse error or timeout)";
          goto cleanup;
      }
      
      delay(CONNECTION_RETRY_DELAY);
    }
    else {
      done = true;
      break;
    }
  }

  if(_DEBUG_HTTP_CALLS) {
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
  }

cleanup:
  delete httpParser;
  // This will free the socket on the WiFi shield
  client.stop();

  return;
}
