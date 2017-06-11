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
    char lastCharParsed;
  public:
    HttpParser(httpResponse_t*);
    bool parse(char);
};
