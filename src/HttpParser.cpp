#include "HttpParser.h"
#include <SPI.h>

HttpParser::HttpParser(httpResponse_t* _httpResponse) {
  httpResponse = _httpResponse;
  responseBufferSize = sizeof(httpResponse->response);
  HttpParser::reset();
}

void HttpParser::reset() {
  responseBufferIndex = 0;
  charsParsed = 0;
  httpResponse->response[responseBufferIndex] = '\0';
  responseParserState = FINDING_STATUS_CODE;
  statusCode = 0;
  successiveNewlines = 0;
  previousChar = '\0';
}

bool HttpParser::parse(char c) {
  int charAsInt;
  bool err = false;

  switch(c) {
    case '\n':
      ++successiveNewlines;
      break;
    case '\r':
      // Ignore carriage returns for the purpose of calculating successive newlines
      break;
    default:
      successiveNewlines = 0;
      break;
  }
  

  switch (responseParserState) {
    case FINDING_STATUS_CODE:
      switch(c) {
        case ' ':
          responseParserState = PARSING_STATUS_CODE;
          break;
      }
      break;
    case PARSING_STATUS_CODE:
      switch(c) {
        case ' ':
          responseParserState = FINDING_BODY;
          httpResponse->statusCode = statusCode;
          break;
        default:
          charAsInt = c - '0';
          if (charAsInt < 0 || charAsInt > 9) {
            Serial.print("Error: Encountered non-numeric character while parsing HTTP status code: ");
            Serial.println(c);
            err = true;
          }
          statusCode = statusCode * 10 + charAsInt;
          break;
      }
      break;
    case FINDING_BODY:
      if (successiveNewlines > 1) {
        responseParserState = PARSING_BODY;
      }
      break;
    case PARSING_BODY:
      if (responseBufferIndex < responseBufferSize - 1) {
        httpResponse->response[responseBufferIndex] = c;
        httpResponse->response[responseBufferIndex + 1] = '\0';
        responseBufferIndex++;
        charsParsed++;
        httpResponse->responseSize = responseBufferIndex;
      }
      else {
        // Avoid buffer overflow - signal an error
        Serial.println("Error: HTTP response buffer is full");
        err = true;
      }
      break;
  }

  previousChar = c;
  return err;
}

