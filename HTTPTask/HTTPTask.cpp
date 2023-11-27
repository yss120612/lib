#include "HTTPTask.h"
#include <Update.h>
#include <Wire.h>

void HTTPTask::cleanup(){
	server->end();
	SPIFFS.end();
	delete server;
};
void HTTPTask::setup(){
SPIFFS.begin(true);
server = new AsyncWebServer(80);
if (!server){
    Serial.println("Error creating HTTP server");
    return;
}
server->on("/", std::bind(&HTTPTask::handleRoot, this, std::placeholders::_1));
server->on("/reboot", std::bind(&HTTPTask::handleReboot, this, std::placeholders::_1));
server->on("/upd", std::bind(&HTTPTask::handleUpd, this, std::placeholders::_1));
server->on("/main", std::bind(&HTTPTask::handleMain, this, std::placeholders::_1));
server->onNotFound(std::bind(&HTTPTask::handleNotFound, this, std::placeholders::_1));
server->serveStatic("/js/jquery.min.js",SPIFFS,"/js/jquery.min.js");
server->serveStatic("/js/bootstrap.min.js",SPIFFS,"/js/bootstrap.min.js");
server->serveStatic("/js/timepicker.min.js",SPIFFS,"/js/timepicker.min.js");
//server->serveStatic("/css/font-awesome.min.css",SPIFFS,"/css/font-awesome.min.css");
server->serveStatic("/css/bootstrap.min.css",SPIFFS,"/css/bootstrap.min.css");
server->on(
	"/update", 
	HTTP_POST, 
	[](AsyncWebServerRequest *request){
	  request->redirect("/");
    }, 
	std::bind(&HTTPTask::handleUpdate, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)
	);

server->on(
	"/spiffs", 
	HTTP_POST, 
	[](AsyncWebServerRequest *request){
	    request->send(200);
    }, std::bind(&HTTPTask::handleSpiffs, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)
	);
};


void HTTPTask::loop(){
  vTaskDelay(pdTICKS_TO_MS(100)); 
}
    

void HTTPTask::handleRoot(AsyncWebServerRequest * request) {
	
	if (!request->authenticate("Yss1", "bqt3"))
		return request->requestAuthentication();
		//Serial.println(SPIFFS.open("/index.htm").readString());
		handleFile("/index.htm","text/html", request);
}


void HTTPTask::handleReboot(AsyncWebServerRequest * request) {
	
	if (!request->authenticate("Yss1", "bqt3"))
		return request->requestAuthentication();
	request->redirect("/");
	ESP.restart();
}


void HTTPTask::handleMain(AsyncWebServerRequest * request) {
	
	if (!request->authenticate("Yss1", "bqt3"))
		return request->requestAuthentication();
		handleFile("/main.htm","text/html", request);
}


void HTTPTask::handleLog(AsyncWebServerRequest * request) {
	
	if (!request->authenticate("Yss1", "bqt3"))
		return request->requestAuthentication();
		handleFile("/log.htm","text/html", request);
}

void HTTPTask::handleFile(String path,String type, AsyncWebServerRequest *request){
	request->send(SPIFFS,path,type);
}

void HTTPTask::handleNotFound(AsyncWebServerRequest * request) {
	request->send(200, "text/plain", "404 PAGE NOT FOUND!!!+");
}

void HTTPTask::handleUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
 uint32_t free_space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
 uint32_t result;
  if (!index){
	request->redirect("/");
    if (!Update.begin(free_space,U_FLASH)) {
    }
	else{
		result=800;
		xQueueSend(que,&result,portMAX_DELAY);
	}
  }

  if (Update.write(data, len) != len) {
  }else{
	  
  }

  if (final) {
    if (!Update.end(true)){
	  
    } else {
		
		ESP.restart();
    }
  }
}

void HTTPTask::handleSpiffs(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final){
  uint32_t result;
  if (!index){
	request->redirect("/");
    if (!Update.begin(UPDATE_SIZE_UNKNOWN,U_SPIFFS)) {
    }
	else{
		result=800;
		xQueueSend(que,&result,portMAX_DELAY);
	}
  }
  if (Update.write(data, len) != len) {
  }else{
	//  if (counter++==9) {Serial.print(".");counter=0;}
  }
  if (final) {
    if (!Update.end(true)){
    } else {
	  ESP.restart();
    }
  }
}


void HTTPTask::handleUpd(AsyncWebServerRequest * request) {
    if (!request->authenticate("Yss1", "bqt3"))
		return request->requestAuthentication();
		handleFile("/upd.htm","text/html",request);
	
}


