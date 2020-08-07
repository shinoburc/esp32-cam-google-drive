#ifndef __RESPONSE_H__
#define __RESPONSE_H__
/**
 * @file Response.h
 * @brief Response
 */

#include <vector>
#include <WString.h>

class Response {
  public:
    explicit Response(void);
    virtual ~Response(void);

    void setHeader(std::pair<String, String>);
    void setContent(String);
    void setStatusCode(String);
    String getHeader(void);
    String getContent(void);
    String getStatusCode(void);
  private:
    std::vector<std::pair<String, String>> headers;
    String content;
    String statusCode;
};

#endif
