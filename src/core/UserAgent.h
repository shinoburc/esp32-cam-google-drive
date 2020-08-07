#ifndef __USERAGENT_H__
#define __USERAGENT_H__
/**
 * @file UserAgent.h
 * @brief UserAgent
 */

#include <vector>

#include <core/Request.h>
#include <core/Response.h>

class UserAgent {
  public:
    explicit UserAgent(String, String);
    virtual ~UserAgent(void);

    void setProxy(String);
    Response request(Request);
  private:
    String ssid;
    String password;
    std::vector<String> split(const String&, const String&);
};

#endif
