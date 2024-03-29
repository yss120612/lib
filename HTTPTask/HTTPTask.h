#ifndef _HTTPTASK_h
#define _HTTPTASK_h
#include <Task.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <GlobalSettings.h>

class HTTPTask: public Task{
public:
HTTPTask(const char *name, uint32_t stack,QueueHandle_t q,EventGroupHandle_t f,MessageBufferHandle_t web_m):Task(name, stack,1,CORE_0){que=q;flg=f;web_mess=web_m;}

protected:
void cleanup() override;
void setup() override;
void loop() override;
void handleRoot(AsyncWebServerRequest * request);
void handleReboot(AsyncWebServerRequest * request);
void handleMain(AsyncWebServerRequest * request);
void handleFile(String path,String type, AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest * request);

void handleUpd(AsyncWebServerRequest * request);
void handleUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleSpiffs(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleLog(AsyncWebServerRequest * request);

QueueHandle_t que;
EventGroupHandle_t flg;
AsyncWebServer * server;
MessageBufferHandle_t web_mess;
};
#endif

