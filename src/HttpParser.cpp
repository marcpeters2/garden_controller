#include "HttpParser.h"
#include <SPI.h>

HttpParser::HttpParser(httpResponse_t* _httpResponse) {
  Serial.println("Instantiating new HttpParser");
  httpResponse = _httpResponse;
  responseBufferSize = sizeof(httpResponse->response);
  responseBufferIndex = 0;
  charsParsed = 0;
  httpResponse->response[responseBufferIndex] = '\0';
}

bool HttpParser::parse(char c) {
//  Serial.print(c);
//  Serial.println(responseBufferIndex);
  if (responseBufferIndex < responseBufferSize - 1) {
    httpResponse->response[responseBufferIndex] = c;
    httpResponse->response[responseBufferIndex + 1] = '\0';
    responseBufferIndex++;
//    Serial.println(responseBufferIndex);
    charsParsed++;
    httpResponse->responseSize = responseBufferIndex;
    return false;
  }
  else {
    // Avoid buffer overflow - signal an error
    Serial.println("Error: HTTP response buffer is full");
    return true;
  }
}

