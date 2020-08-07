#ifndef __GENERICMULTIPARTCONTENT_H__
#define __GENERICMULTIPARTCONTENT_H__
/**
 * @file GenericMultipartContent.h
 * @brief GenericMultipartContent
 */

#include "FS.h"
#include "MultipartContent.h"

template <class T>
class GenericMultipartContent : public MultipartContent {
  public:
    explicit GenericMultipartContent(T buf, size_t length, String contentType) : content(buf), len(length), position(0)
    {
       this->contentType = contentType;
    }

    virtual ~GenericMultipartContent(void)
    {
    }

    virtual int available(void)
    {
      return this->length() - position;
    }

    virtual uint8_t read()
    {
      if(position < this->length())
      {
        return content[position++];
      } else {
        return -1;
      }
    }

    virtual size_t read(uint8_t *buf, size_t size)
    {
      for(size_t i = 0; i < size; i++)
      {
        if(this->available())
        {
          buf[i] = this->read();
        }
        else
        {
          return i;
        }
      }

      return size;
    }

    virtual size_t length()
    {
      return this->len;
    }

  private:
    T content;
    size_t len;
    int position;
};

#endif
