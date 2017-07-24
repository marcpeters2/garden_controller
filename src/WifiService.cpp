#include "WifiService.h"

#define WIFI_CONNECT_TIMEOUT_MS 2000

unsigned long long _wifiLastConnectTimestamp = 0;

bool WifiService::isConnected() {
  static int lastStatus = WL_IDLE_STATUS;
  int status = WiFi.status();

  if(lastStatus != WL_CONNECTED && status == WL_CONNECTED) {
    Serial.println("### WiFi Connected!");
    printCurrentNet();
    printWiFiData();
  }

  lastStatus = status;
  return status == WL_CONNECTED;
}

void WifiService::connectToWiFi(const char* ssid, const char* password) {
  //int status = WL_IDLE_STATUS;     // the WiFi radio's status

  int status = WiFi.status();
  unsigned long long now = Util::now(0);
  static bool firstCall = true;

  if(status == WL_CONNECTED) {
    return;
  }
  else if(firstCall || now - _wifiLastConnectTimestamp > WIFI_CONNECT_TIMEOUT_MS) {
    _wifiLastConnectTimestamp = now;
    Serial.println();
    Serial.print(">>> Attempt connection to WiFi SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
  }
}

void WifiService::printWifiSignalQuality(int rssi) {
  Serial.print("signal quality: ");
  
  if (rssi > -60) {
    Serial.println("High");
  }
  else if (rssi > -80) {
    Serial.println("Medium");
  } 
  else if (rssi > -90) {
    Serial.println("Low");
  }
  else {
    Serial.println("Unuseable");
  }
}

void WifiService::printWiFiData() {
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

void WifiService::printCurrentNet() {
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
  printWifiSignalQuality(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type: ");
  Serial.println(encryption, HEX);
}

