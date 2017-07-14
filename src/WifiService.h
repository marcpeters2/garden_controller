#ifndef WIFISERVICE_H
#define WIFISERVICE_H

#include "Arduino.h"
//#include <Wire.h>
#include <WiFi101.h>

class WifiService {
  public:
    static void connectToWiFi(const char*, const char*);
    static const char* wifiSignalQuality(int);
    static void printWiFiData();
    static void printCurrentNet();
};

#endif
