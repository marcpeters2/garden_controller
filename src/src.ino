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
#include <SPI.h>
#include <WiFi101.h>

#define GET "GET"
#define POST "POST"

#define HTTP_RESPONSE_BUFFER_SIZE 1024

WiFiClient client;

char ssid[] = "BELL123";     //  your network SSID (name)
char pass[] = "2ECADD44D569";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status

char server[] = "httpbin.org";

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 5L * 1000L; // delay between updates, in milliseconds

enum controllerState_t {
  INITIAL,
  RECEIVED_ID
};

controllerState_t controllerState = INITIAL;
int myId;

struct httpResponse_t {
  int statusCode;
  char response[HTTP_RESPONSE_BUFFER_SIZE];
};

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  connectToWiFi();
}

void loop() {
  // check the network connection once every 10 seconds:
  //  delay(10000);
  //  printCurrentNet();

  switch(controllerState) {
    case INITIAL:
      myId = getMyId();
      break;
    case RECEIVED_ID:

      break;
  }

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
//  while (client.available()) {
//    char c = client.read();
//    Serial.write(c);
//  }

  delay(5000);

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
//  if (!client.connected() && millis() - lastConnectionTime > postingInterval) {
//    httpRequest();
//  }
}

int getMyId() {

  //char response[HTTP_RESPONSE_BUFFER_SIZE];
  httpResponse_t httpResponse;
  httpRequest(GET, "/anything", &httpResponse);

  Serial.println("Received response: ");
  Serial.print(httpResponse.response);

  return 1;
}

void connectToWiFi() {
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(1000);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network");
  printCurrentNet();
  printWiFiData();
}

// this method makes a HTTP connection to the server:
void httpRequest(char* method, char* path, httpResponse_t* httpResponse) {
  int index = 0;
  int responseBufferSize = sizeof(httpResponse->response);

  bool sentRequest = false;
  bool receivedResponse = false;
  bool overflow = false;
  bool done = false;

  while (!done) {

    sentRequest = false;
    receivedResponse = false;
    overflow = false;

    while (!sentRequest) {
      // close any connection before send a new request.
      // This will free the socket on the WiFi shield
      client.stop();

      Serial.println();
      Serial.print("Making HTTP request ");
      Serial.print(method);
      Serial.print(" ");
      Serial.print(server);
      Serial.println(path);
      Serial.print("Buffer size ");
      Serial.println(responseBufferSize);
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
    
        // note the time that the connection was made:
        //lastConnectionTime = millis();
      }
      else {
        // if you couldn't make a connection:
        Serial.print("HTTP request ");
        Serial.print(method);
        Serial.print(" ");
        Serial.print(path);
        Serial.println(" failed.  Couldn't connect.");
        Serial.println();
      }
    }
  
    while (!receivedResponse) {
      
      while (client.available()) {
        receivedResponse = true;
        char c = client.read();
  
        if (index < responseBufferSize - 1) {
          httpResponse->response[index] = c;
          ++index;
        }
        else {
          overflow = true;
          Serial.println("Error: HTTP response buffer is full");
          break;
        }
      }
      
    }

    if (!overflow) {
      done = true;
      httpResponse->response[index] = '\0';
      break;
    }

    delay(2000);

  }

  return;

  //delay(100);

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
//  if (!client.connected() && millis() - lastConnectionTime > postingInterval) {
//    httpRequest();
//  }
}

String wifiSignalQuality(int rssi) {
  if (rssi > -60) {
    return "High";
  }
  else if (rssi > -80) {
    return "Medium";
  } 
  else if (rssi > -90) {
    return "Low";
  }
  else {
    return "Unuseable";
  }
}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  Serial.print(bssid[5], HEX);
  Serial.print(":");
  Serial.print(bssid[4], HEX);
  Serial.print(":");
  Serial.print(bssid[3], HEX);
  Serial.print(":");
  Serial.print(bssid[2], HEX);
  Serial.print(":");
  Serial.print(bssid[1], HEX);
  Serial.print(":");
  Serial.println(bssid[0], HEX);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI): ");
  Serial.println(rssi);
  Serial.print("signal quality: ");
  Serial.println(wifiSignalQuality(rssi));

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
  Serial.println();
}

