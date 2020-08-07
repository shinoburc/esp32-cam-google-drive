#ifndef __FILEMULTIPARTCONTENT_H__
#define __FILEMULTIPARTCONTENT_H__
/**
 * @file FileMultipartContent.h
 * @brief FileMultipartContent
 */

#include "FS.h"
#include "MultipartContent.h"

class FileMultipartContent : public MultipartContent {
  public:
    explicit FileMultipartContent(File *file, String contentType) : file(file), position(0)
    {
      this->setContentType(contentType);
    }

    virtual ~FileMultipartContent(void)
    {
      this->file->close();
    }

    virtual int available(void)
    {
      return this->file->available();
    }

    virtual uint8_t read()
    {
      return this->file->read();
    }

    virtual size_t read(uint8_t *buf, size_t size)
    {
      return this->file->read(buf, size);
    }

    virtual size_t length()
    {
      return this->file->size();
    }

  private:
    File *file;
    int position;
};

#endif
