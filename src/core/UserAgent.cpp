
#ifdef ESP32
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <time.h>

#include <core/UserAgent.h>

#define connect(host, port) connect(host.c_str(), port)

UserAgent::UserAgent(String ssid, String password){
  this->ssid = ssid;
  this->password = password;
  //Wifi
  if(WiFi.status() != WL_CONNECTED){
    if(WiFi.status() != WL_CONNECTED){
      WiFi.mode(WIFI_STA);
      WiFi.begin(this->ssid.c_str(), this->password.c_str());
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
    }
    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
  const char* ntpServer1 =  "ntp.jst.mfeed.ad.jp";
  const char* ntpServer2 =  "ntp.nict.jp";
  const char* ntpServer3 =  "ntp.ring.gr.jp";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
#ifdef ESP32
  struct tm timeinfo;
  while(!getLocalTime(&timeinfo)) {
    delay(100);
    Serial.println("Reconnect NTP Server.");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
  }
#endif
}

UserAgent::~UserAgent(void){
}

void UserAgent::setProxy(String proxy){
  Serial.println("TODO: proxy implement not yet.");
}

Response UserAgent::request(Request request){
  Response response;
  WiFiClientSecure client;
  if (!client.connect(request.getHost(), 443)) {
    Serial.println("connection failed");
    return response;
  }

  request.writeTo(&client);

  bool isChunked = false;
  bool isContentLength = false;
  int contentLength = 0;
  while (client.connected()) {
    if (client.available()){
      String line = client.readStringUntil('\n');

      if (line == "\r") { // HTTP Content
        if(isChunked){
          String content;
          while(1){
            line = client.readStringUntil('\n');
            line.replace("\r", "");
            if(line == "0"){
              break;
            }
            long bufSize = strtol(line.c_str(), NULL, 16);
            char *buffer;
            buffer = (char*)calloc(sizeof(char*), bufSize);
            client.readBytes(buffer, bufSize);
            content += String(buffer);
            client.readStringUntil('\n');
            free(buffer);
          }
          response.setContent(content);
        } else if(isContentLength){
          String content;
          char *buffer;
          buffer = (char*)calloc(sizeof(char*), contentLength);
          client.readBytes(buffer, contentLength);
          response.setContent(buffer);
          free(buffer);
        } else {
          line = client.readString();
          response.setContent(line);
        }
        break;
      } else if(line.indexOf(": ") == -1){ //HTTP Status
        // HTTP Status StatusMessage
        // HTTP/1.1 200 OK
        std::vector<String> status = this->split(line, " "); 
        if(status.size() == 3){
          response.setStatusCode(status.at(1));
        }
      } else if(line.indexOf(": ") != -1){ //HTTP Header
        // HTTP Header
        // Content-Length: 133
        std::vector<String> header = this->split(line, ": "); 
        if(header.size() == 2){
          response.setHeader(std::make_pair(header.at(0), header.at(1)));
        }
        if(header.at(0).indexOf("Transfer-Encoding") != -1 && header.at(1).indexOf("chunked") != -1){
          isChunked = true;
        }
        if(header.at(0) == "Content-Length"){
          isContentLength = true;
          contentLength = header.at(1).toInt();
        }
      }
    } else {
      delay(100);
    }
  }
  client.flush();
  client.stop();

  return response;
}

std::vector<String> UserAgent::split(const String& s, const String& delim){
  std::vector<String> result;
  int pos = 0;
  while(1){
    if(s.substring(pos).indexOf(delim) != -1){
      result.push_back(s.substring(pos).substring(0, s.substring(pos).indexOf(delim)));
      pos = pos + s.substring(pos).indexOf(delim) + delim.length();
    } else {
      result.push_back(s.substring(pos));
      break;
    }
  }
  return result;
}
