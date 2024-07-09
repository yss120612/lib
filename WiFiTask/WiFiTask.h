#ifndef __WIFITASK__
#define __WIFITASK__
#include <Task.h>
#include "esp_wifi_types.h"
#include "esp_netif_types.h"

//#include <WiFi.h>

//#include "Settings.h"

//const char WIFI_SSID[] = "Yss_GIGA";
//const char WIFI_PSWD[] = "bqt3bqt3";





class WiFiTask: public Task
{
public:
WiFiTask(const char *name, uint32_t stack,QueueHandle_t q,EventGroupHandle_t f):Task(name, stack,1,CORE_0){que=q;flg=f;retry_num=0;}

static bool success_syncro;

protected:
void wifi_start();
void wifi_stop();
void wifi_shutdown();
esp_err_t wifi_sta_do_disconnect();
esp_err_t wifi_sta_do_connect(wifi_config_t wifi_config, bool wait);
esp_err_t wifi_connect();
void cleanup() override;
void setup() override;
void loop() override;
void timeNTP();
static void timesync_cb(struct timeval *tv);
void on_disconnect(esp_event_base_t event_base,int32_t event_id, void *event_data);
void on_connect(esp_event_base_t event_base,int32_t event_id, void *event_data);
void on_sta_got_ip(esp_event_base_t event_base,int32_t event_id, void *event_data);

static void on_connect(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);
static void on_disconnect(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);
static void on_sta_got_ip(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data);

QueueHandle_t que;
esp_netif_t * sta_netif;
//SemaphoreHandle_t s_semph_get_ip_addrs;
EventGroupHandle_t flg;
uint8_t retry_num,index;

};
#endif