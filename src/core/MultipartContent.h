#ifndef __MULTIPARTCONTENT_H__
#define __MULTIPARTCONTENT_H__
/**
 * @file MultipartContent.h
 * @brief MultipartContent
 */

#include "FS.h"

class MultipartContent {
  public:
    explicit MultipartContent() {}
    virtual ~MultipartContent(void) {}

    virtual int available(void) = 0;
    virtual uint8_t read() = 0;
    virtual size_t read(uint8_t *buf, size_t size) = 0;
    virtual size_t length() = 0;

    void setContentType(String contentType)
    {
      this->contentType = contentType;
    }

    String getContentType(void)
    {
      return this->contentType;
    }

  protected:
    String contentType;
};

#endif
