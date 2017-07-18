#ifndef WIFISERVICE_H
#define WIFISERVICE_H

class WifiService {
  public:
    static void connectToWiFi(const char*, const char*);
    static const char* wifiSignalQuality(int);
    static void printWiFiData();
    static void printCurrentNet();
};

#endif
