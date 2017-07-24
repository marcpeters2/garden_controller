#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include <SPI.h>
#include <WiFi101.h>
#include "Util.h"

#define MAC_ADDRESS_NUM_BYTES 6

class WifiService {
  public:
    static bool isConnected();
    static void connectToWiFi(const char*, const char*);
    static void printWifiSignalQuality(int);
    static void printWiFiData();
    static void printCurrentNet();
};

#endif
