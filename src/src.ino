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

enum wl_tcp_state {
  CLOSED      = 0,
  LISTEN      = 1,
  SYN_SENT    = 2,
  SYN_RCVD    = 3,
  ESTABLISHED = 4,
  FIN_WAIT_1  = 5,
  FIN_WAIT_2  = 6,
  CLOSE_WAIT  = 7,
  CLOSING     = 8,
  LAST_ACK    = 9,
  TIME_WAIT   = 10
};

WiFiClient client;

char ssid[] = "BELL123";     //  your network SSID (name)
char pass[] = "2ECADD44D569";  // your network password
int status = WL_IDLE_STATUS;     // the WiFi radio's status

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 1L * 1000L; // delay between updates, in milliseconds

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

  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  delay(100);

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (!client.connected() && millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
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
void httpRequest() {

  char server[] = "httpbin.org";
  
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.println("GET /anything HTTP/1.1");
    client.print("Host: ");
    client.println(server);
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

//    Serial.print("Connection status: ");
//    Serial.println(clientStatus(client.status()));

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
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

String clientStatus(int statusVal) {
  if (statusVal == CLOSED) {
    return "CLOSED";
  }
  else if (statusVal == LISTEN) {
    return "LISTEN";
  }
  else if (statusVal == SYN_SENT) {
    return "SYN_SENT";
  }
  else if (statusVal == SYN_RCVD) {
    return "SYN_RCVD";
  }
  else if (statusVal == ESTABLISHED) {
    return "ESTABLISHED";
  }
  else if (statusVal == FIN_WAIT_1) {
    return "FIN_WAIT_1";
  }
  else if (statusVal == FIN_WAIT_2) {
    return "FIN_WAIT_2";
  }
  else if (statusVal == CLOSE_WAIT) {
    return "CLOSE_WAIT";
  }
  else if (statusVal == CLOSING) {
    return "CLOSING";
  }
  else if (statusVal == LAST_ACK) {
    return "LAST_ACK";
  }
  else if (statusVal == TIME_WAIT) {
    return "TIME_WAIT";
  }
  else {
    return "UNKNOWN";
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

