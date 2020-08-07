#include <core/Response.h>

Response::Response(void){
}
Response::~Response(void){
}

void Response::setHeader(std::pair<String, String> header){
  this->headers.push_back(header);
}
void Response::setContent(String content){
  this->content = content;
}

String Response::getHeader(){
  String headerString;
  for(std::pair<String, String> header : this->headers){
    headerString += header.first + ": " + header.second + "\r\n";
  }
  return headerString;
}

String Response::getContent(){
  return this->content;
}

void Response::setStatusCode(String statusCode){
  this->statusCode = statusCode;
}
String Response::getStatusCode(void){
  return this->statusCode;
}
