#ifndef GCHTTPCLIENT_H
#define GCHTTPCLIENT_H

#include <string.h>
#include <SPI.h>
#include <WiFi101.h>
#include "HttpParser.h"
#include "Util.h"
#include "TimeService.h"

#define GET "GET"
#define POST "POST"

struct httpServer_t {
  String host;
  int port;
};

struct httpEndpoint_t {
  String method;
  String path;
};

class GCHttpClient {
  public:
    static void debugHttpCalls(bool);
    static void httpRequest(httpServer_t*, httpEndpoint_t*, String, httpResponse_t*, const char** error, unsigned int maxRetries);
};

#endif
