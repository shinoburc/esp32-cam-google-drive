#include <stdio.h>
#include <core/Request.h>

Request::Request(void){
}
Request::~Request(void){
}

void Request::setMethod(String method){
  this->method = method;
}

void Request::setUri(String host, String url){
  this->host = host;
  this->url = url;
}

String Request::getHost(){
  return this->host;
}

String Request::getUrl(){
  return this->url;
}

void Request::setHeader(std::pair<String, String> header){
  this->headers.push_back(header);
}

void Request::setContent(String content){
  this->content = content;
}

void Request::addMultipartContent(MultipartContent* multipartContent){
  this->multipartContents.push_back(multipartContent);
}

String Request::getHeader(void){
  String headerString;
  headerString += this->method + " " + this->url.c_str() + " HTTP/1.1\r\n";
  headerString += "Host: " + this->host + "\r\n";
             
  for(std::pair<String, String> header : this->headers){
    headerString += header.first + ": " + header.second + "\r\n";
  }

  if(this->multipartContents.size() == 0){
    headerString += "Content-Length: " + this->getContentLength() + "\r\n";
  } else {
    headerString += "Content-Length: " + this->getMultipartContentLength() + "\r\n";
  }

  // TODO: add Host Header
  // headerString += "Host: " + host + "\n";

  //header end mark
  headerString += "\r\n";

  return headerString;
}

String Request::getContent(void){
  if(this->multipartContents.size() == 0){
    return this->content;
  } else {
    //return this->getMultipartContent();
    return "";
  }
}

void Request::writeContent(WiFiClientSecure* client){
  String boundary = "foo_bar_baz";
  String boundaryLine = "--" + boundary + "\r\n";
  String boundaryFooter = "--" + boundary + "--\r\n";

  for(MultipartContent* multipartContent : this->multipartContents){
    client->print(boundaryLine);
    client->print("Content-Type: " + multipartContent->getContentType() + "\r\n");
    client->print("\r\n");
    int readSize= 0;
    size_t w = 0;
    uint8_t buf[2048];
    while(multipartContent->available()){
      //Serial.print("Sending...");
      //Serial.println(w);
      //w += client->write(multipartContent.second->read());
      readSize = multipartContent->read(buf, 2048);
      w += client->write(buf, readSize);
      if((w % 512) == 0){
        client->flush();
        Serial.println("flush");
      }
    }
    client->print("\r\n");
    client->print("\r\n");
  }

  client->print(boundaryFooter);
}

String Request::getContentLength(void){
  return String(this->content.length());
}

String Request::getMultipartContentLength(void){
  int length = 0;
  String boundary = "foo_bar_baz";
  String boundaryLine = "--" + boundary + "\r\n";
  String boundaryFooter = "--" + boundary + "--\r\n";

  for(MultipartContent* content : this->multipartContents){
    length += boundaryLine.length();
    length += String("Content-Type: ").length() + content->getContentType().length() + String("\r\n").length();
    length += String("\r\n").length();
    length += content->length() + String("\r\n").length();
    length += String("\r\n").length();
  }

  length += boundaryFooter.length();

  return String(length);
}

void Request::writeTo(WiFiClientSecure* client){
  if(this->multipartContents.size() == 0){
    client->print((this->getHeader() + this->getContent()));
  } else {
    client->print(this->getHeader());
    this->writeContent(client);
  }
  client->flush();
}
