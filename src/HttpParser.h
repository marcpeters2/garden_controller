#define HTTP_RESPONSE_BUFFER_SIZE 1024

struct httpResponse_t {
  int statusCode;
  char response[HTTP_RESPONSE_BUFFER_SIZE];
  int responseSize;
};

class HttpParser {
    httpResponse_t* httpResponse;
    int responseBufferIndex;
    int responseBufferSize;
    int charsParsed;
  public:
    HttpParser(httpResponse_t*);
    bool parse(char);
  private:
    int statusCode;
    int successiveNewlines;
    char previousChar;
    enum responseParserState_t {
      FINDING_STATUS_CODE,
      PARSING_STATUS_CODE,
      FINDING_BODY,
      PARSING_BODY
    };
    responseParserState_t responseParserState;
};
