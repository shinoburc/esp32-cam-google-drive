#define PORTAL_PRINT Serial

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include "core/UserAgent.h"
#include "core/MultipartContent.h"
#include "core/GenericMultipartContent.h"

#include "esp_camera.h"
#include "driver/rtc_io.h"

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid     = "<<YOUR_SSID>>";
const char* password = "<<YOUR_PASSWORD>>";

String getAccessToken()
{
  String refresh_token = "<<YOUR_REFRESH_TOKEN>>";
  String client_id = "<<YOUR_CLIENT_ID>>";
  String client_secret = "<<YOUR_CLIENT_SECRET>>";

  String token_uri = "/oauth2/v4/token";
  String host = "www.googleapis.com";
  String httpsPort = "443";

  String postData = "";
  postData += "refresh_token=" + refresh_token;
  postData += "&client_id=" + client_id;
  postData += "&client_secret=" + client_secret;
  postData += "&grant_type=refresh_token";

  UserAgent ua(ssid, password);

  Request request;
  request.setMethod("POST");
  request.setUri(host, token_uri);
  request.setHeader(std::make_pair("Content-Type", "application/x-www-form-urlencoded"));
  request.setContent(postData);
  Response response = ua.request(request);

  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, response.getContent());

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  return doc["access_token"];
}

String findFolder(String access_token, String name)
{
  String uri = "/drive/v3/files";
  String host = "www.googleapis.com";
  String httpsPort = "443";

  // urlencode
  // = -> %3D
  // ' -> %27
  // / -> %2F
  String query = "q=(name%3D%27" + name + "%27)" + "and(mimeType%3D%27application%2Fvnd.google-apps.folder%27)and(trashed%3Dfalse)";

  UserAgent ua(ssid, password);

  Request request;
  request.setMethod("GET");
  request.setUri(host, uri + "?" + query);
  request.setHeader(std::make_pair("Accept", "application/json"));
  request.setHeader(std::make_pair("Authorization", "Bearer " + access_token));
  Response response = ua.request(request);

  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, response.getContent());

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return ""; 
  }

  if(doc["files"].size() == 0){
    return "";
  }

  String folderId = doc["files"][0]["id"];
  Serial.println(folderId);
  return folderId;
}

String createFolder(String access_token, String name)
{
  String uri = "/upload/drive/v3/files";
  String query = "uploadType=multipart";
  String host = "www.googleapis.com";
  String httpsPort = "443";

  UserAgent ua(ssid, password);

  Request request;
  request.setMethod("POST");
  request.setUri(host, uri + "?" + query);
  //request.setHeader(std::make_pair("Accept", "application/json"));
  request.setHeader(std::make_pair("Authorization", "Bearer " + access_token));
  request.setHeader(std::make_pair("Content-Type", "multipart/related; boundary=foo_bar_baz"));

  String metadataJson = "{name: '" + name + "', mimeType: 'application/vnd.google-apps.folder'}";
  MultipartContent* metadataContent = new GenericMultipartContent<String>(metadataJson, metadataJson.length(), "application/json; charset=UTF-8");

  request.addMultipartContent(metadataContent);

  Response response = ua.request(request);

  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, response.getContent());

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  String folderId = doc["id"];

  return folderId;
}

String uploadFile(String access_token, String name, String folderId = "")
{
  camera_fb_t * fb = NULL;

  // Take Picture with Camera
  fb = esp_camera_fb_get();
  if(!fb) {
    Serial.println("Camera capture failed");
    //return "";
  }

  String uri = "/upload/drive/v3/files";
  String query = "uploadType=multipart";
  String host = "www.googleapis.com";
  String httpsPort = "443";

  UserAgent ua(ssid, password);

  Request request;
  request.setMethod("POST");
  request.setUri(host, uri + "?" + query);
  //request.setHeader(std::make_pair("Accept", "application/json"));
  request.setHeader(std::make_pair("Authorization", "Bearer " + access_token));
  request.setHeader(std::make_pair("Content-Type", "multipart/related; boundary=foo_bar_baz"));

  String metaJson;
  if(folderId == ""){
    metaJson = "{\"name\":\"" + name + "\"}";
  } else {
    metaJson = "{\"name\":\"" + name + "\", \"parents\":[\"" + folderId + "\"]}";
  }

  String text = "Hello";

  MultipartContent* jsonContent = new GenericMultipartContent<String>(metaJson, metaJson.length(), "application/json; charset=UTF-8");
  MultipartContent* jpegContent = new GenericMultipartContent<uint8_t*>(fb->buf, fb->len, "image/jpeg");
  //MultipartContent* textContent = new GenericMultipartContent<String>(text, text.length(), "text/plain");

  request.addMultipartContent(jsonContent);
  request.addMultipartContent(jpegContent);
  //request.addMultipartContent(textContent);
  Response response = ua.request(request);
  esp_camera_fb_return(fb);

  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc, response.getContent());

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
  }

  String fileId = doc["id"];


  return fileId;
}

void takePicture(){
  String access_token = getAccessToken();
  String folderId = findFolder(access_token, "Images");
  if(folderId == ""){
    folderId = createFolder(access_token, "Images");
  }
  if(folderId == ""){
    return;
  }
  Serial.print(WiFi.localIP());
  Serial.println("/");
  //fileId = uploadFile(access_token, "hello.jpg", folderId);

  //IMG_20190318090011
  String filename = "IMG_" + datetime() + ".jpg";
  String fileId = uploadFile(access_token, filename, folderId);
  Serial.println("fileId: " + fileId);
}


String datetime() {
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo)) {
    Serial.println("Failed to obtain time");
    return String();
  }
  //Serial.println(&timeinfo, "%Y %m %d %a %H:%M:%S");
  char now[15];
  sprintf(now, "%04d%02d%02d%02d%02d%02d",
            timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
                      timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  return String(now);
}

void setup()
{
  Serial.begin(115200);

  camera_config_t config;
  //config.ledc_channel = LEDC_CHANNEL_0;
  //config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //config.pixel_format = PIXFORMAT_RGB565;


  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    //return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.print(WiFi.localIP());
  Serial.println("/");

  const char* ntpServer1 =  "ntp.jst.mfeed.ad.jp";
  const char* ntpServer2 =  "ntp.nict.jp";
  const char* ntpServer3 =  "ntp.ring.gr.jp";
  const long  gmtOffset_sec = 9 * 3600;
  const int   daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);

  takePicture();
}

void loop()
{
  delay(1000);
}
