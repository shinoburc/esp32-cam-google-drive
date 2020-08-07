#ifndef __REQUEST_H__
#define __REQUEST_H__
/**
 * @file Request.h
 * @brief Request
 */

#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <vector>
#include <WString.h>

#include "MultipartContent.h"

class Request {
  public:
    explicit Request(void);
    virtual ~Request(void);

    void setMethod(String);
    void setUri(String, String);
    String getHost();
    String getUrl();
    void setHeader(std::pair<String, String>);
    void setContent(String);
    void addMultipartContent(MultipartContent*);
    void writeTo(WiFiClientSecure*);
    void writeContent(WiFiClientSecure*);

    String getHeader(void);
    String getContent(void);
  private:
    String method;
    String host;
    String url;
    std::vector<std::pair<String, String>> headers;
    std::vector<MultipartContent*> multipartContents;
    String content;

    String getContentLength(void);
    String getMultipartContentLength(void);
};

#endif
