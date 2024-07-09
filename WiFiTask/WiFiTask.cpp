#include "WiFiTask.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "time.h"

// #include "Events.h"

// #define NETWORKS 2
// #define CONFIG_EXAMPLE_CONNECT_WIFI 1

// #define WIFI_SCAN_METHOD WIFI_FAST_SCAN
// #define WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
// #define CONFIG_WIFI_CONN_MAX_RETRY 3
//#define CONFIG_WIFI_SSID "Yss_GIGA"
//#define CONFIG_WIFI_PASSWORD "bqt3bqt3"


const char * WIFI_SSID[2] = {"Yss_GIGA","academy"};
const char * WIFI_PSWD[2] = {"bqt3bqt3","123qweasdzxc"};
const char * TAGWIFI = "YssWiFi";
const char * NETIF_DESC_STA = "yss_netif_sta";

esp_err_t WiFiTask::wifi_connect()
{
    ESP_LOGI(TAGWIFI, "Start connect.");
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;//WIFI_FAST_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.rssi=75;
    wifi_config.sta.threshold.authmode=WIFI_AUTH_WPA2_PSK;
    strcpy(reinterpret_cast<char*>(wifi_config.sta.ssid), WIFI_SSID[index]);
    strcpy(reinterpret_cast<char*>(wifi_config.sta.password),WIFI_PSWD[index]);
    // memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
    // memcpy(wifi_config.sta.ssid, WIFI_SSID[index],strlen(WIFI_SSID[index]));
    // memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
    // memcpy(wifi_config.sta.password, WIFI_PSWD[index],strlen(WIFI_PSWD[index]));
    return wifi_sta_do_connect(wifi_config, true);
 }

void WiFiTask::setup()
{
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_STA);
  // WiFi.onEvent(std::bind(&WiFiTask::wifiOnEvent, this, std::placeholders::_1));
  // WiFi.disconnect();
  index=0;
  //vSemaphoreCreateBinary(s_semph_get_ip_addrs);
  wifi_start();
  xEventGroupClearBits(flg, FLAG_WIFI);
  setenv("TZ","IRKT-8",1);
  tzset();
  //success_syncro=false;
  //index=0;

}

void WiFiTask::cleanup()
{
   wifi_shutdown();
}

void WiFiTask::wifi_start()
{
    
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    
}

void WiFiTask::wifi_stop()
{
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(sta_netif));
    esp_netif_destroy(sta_netif);
    sta_netif = NULL;
}

esp_err_t WiFiTask::wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
{
    // if (s_semph_get_ip_addrs==NULL) xSemaphoreCreateBinary();
    // if (s_semph_get_ip_addrs == NULL) {
    //     return ESP_ERR_NO_MEM;
    // }
    
     
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_sta_got_ip, this));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_connect, this));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,&on_disconnect,this));
    

    ESP_LOGE(TAGWIFI, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect(); 
    if (ret != ESP_OK) {
        ESP_LOGE(TAGWIFI, "WiFi connect failed! ret:%x", ret);
        return ret;
        
    }

    //xEventGroupSetBits(flg,FLAG_WIFI);
    
    
     //xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
    // {    
    //    return ESP_FAIL;
    // }
     
    return ESP_OK;
}

esp_err_t WiFiTask::wifi_sta_do_disconnect()
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_sta_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_connect));
    // if (s_semph_get_ip_addrs) {
    //     vSemaphoreDelete(s_semph_get_ip_addrs);
    // }
    //vSemaphoreDelete(s_semph_get_ip_addrs);
    return esp_wifi_disconnect();
}

void WiFiTask::on_disconnect(esp_event_base_t event_base,int32_t event_id, void *event_data){
  
  if (flg) xEventGroupClearBits(flg,FLAG_WIFI);
}

void WiFiTask::on_connect(esp_event_base_t event_base,int32_t event_id, void *event_data){
    
}

void WiFiTask::on_sta_got_ip(esp_event_base_t event_base,int32_t event_id, void *event_data){
  ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (strncmp(NETIF_DESC_STA, esp_netif_get_desc(event->esp_netif), strlen(NETIF_DESC_STA) - 1) != 0) {
        return;
    }
    
    ESP_LOGE(TAGWIFI, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    //xSemaphoreGive(s_semph_get_ip_addrs);//free samaphore
    //xSemaphoreGive(s_semph_get_ip_addrs);
    xEventGroupSetBits(flg,FLAG_WIFI);
    timeNTP();
    // } else {
    //     ESP_LOGE(TAGWIFI, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    // }
}

void WiFiTask::on_disconnect(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
  WiFiTask *wt = static_cast<WiFiTask * >(arg);
  wt->on_disconnect(event_base,event_id,event_data);
}

void WiFiTask::on_connect(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
  WiFiTask *wt = static_cast<WiFiTask * >(arg);
  wt->on_connect(event_base,event_id,event_data);
}

void WiFiTask::on_sta_got_ip(void *arg, esp_event_base_t event_base,int32_t event_id, void *event_data)
{
  WiFiTask *wt = static_cast<WiFiTask * >(arg);
  wt->on_sta_got_ip(event_base,event_id,event_data);
}

void WiFiTask::wifi_shutdown(void)
{
    wifi_sta_do_disconnect();
    wifi_stop();
}


void WiFiTask::loop()
{
  event_t event;
  vTaskDelay(pdMS_TO_TICKS(10));
  const uint32_t WIFI_CONNECT_WAIT = 5000; // 5sec.
  const uint32_t WIFI_TIMEOUT = 30000;     // 30 sec.
  event_t result;
  result.state = LED_EVENT;
  if (xEventGroupGetBits(flg) & FLAG_WIFI){
    vTaskDelay(pdMS_TO_TICKS(60000));//every minute
    if (WiFiTask::success_syncro){
      WiFiTask::success_syncro=false;
      time_t t=time(nullptr);
      
      tm ti;
      localtime_r(&t,&ti);
      ESP_LOGE(TAGWIFI, "Updated time from inet! %d:%d length=%d",ti.tm_hour,ti.tm_min,sizeof(t));
      event.state=RTC_EVENT;
      event.button=RTCTIMEADJUST;
      event.data=t;
      xQueueSend(que,&event,portMAX_DELAY);
    }
    
  }else{
    vTaskDelay(pdMS_TO_TICKS(3500));
    wifi_connect();
  }
//   if (!WiFi.isConnected())
//   {
//     WiFi.begin(WIFI_SSID[index], WIFI_PSWD[index]);
// #ifdef DEBUGG
//     //Serial.printf("Connecting to SSID \"%s\"...\n", WIFI_SSID);
// #endif
//     result.button = LED_CONNECTING;
//     //xQueueSend(que, &result, portMAX_DELAY);

//      // wait connection WIFI_CONNECT_WAIT
//       uint32_t start = millis();
//       while ((!WiFi.isConnected()) && (millis() - start < WIFI_CONNECT_WAIT))
//       {
//         vTaskDelay(pdMS_TO_TICKS(1000));
//       }
    
//     if (WiFi.isConnected())
//     {
// //#ifdef DEBUGG
//       portENTER_CRITICAL(&_mutex);
//       Serial.print("Connected to WiFi with IP ");
//       Serial.println(WiFi.localIP());
//       portEXIT_CRITICAL(&_mutex);
// //#endif
//       xEventGroupSetBits(flg, FLAG_WIFI);
//       result.button = LED_CONNECTED;
//       //xQueueSend(que, &result, portMAX_DELAY);
//     }
//     else
//     {
//       WiFi.disconnect();
// #ifdef DEBUGG
//       Serial.println("Failed to connect to WiFi!");
// #endif
//       xEventGroupClearBits(flg, FLAG_WIFI);
//       result.button = LED_CONNECT_FAILED;
//       xQueueSend(que, &result, portMAX_DELAY);
//       vTaskDelay(pdMS_TO_TICKS(WIFI_TIMEOUT));
//       index++;
//       if (index>=NETWORKS) index=0;
//     }
//   }
//   else
//   { // all OK
//     vTaskDelay(pdMS_TO_TICKS(20000));
//   }
}

void WiFiTask::timeNTP()
{
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0,"pool.ntp.org");
  sntp_setservername(0,"time.nist.gov");
  sntp_set_sync_interval(3600000*24);//one hour * 24
  sntp_set_time_sync_notification_cb(&timesync_cb);
  sntp_init();
}

void WiFiTask::timesync_cb(struct timeval *tv){
  struct tm timeinfo;
  localtime_r(&tv->tv_sec,&timeinfo);
  if (timeinfo.tm_year>=124){
    success_syncro=true;
    //ESP_LOGE(TAGWIFI, "Updated time from inet! %d:%d",timeinfo.tm_hour,timeinfo.tm_min);
  }
}

bool WiFiTask::success_syncro=false;