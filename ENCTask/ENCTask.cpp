#include "ENCTask.h"
#include <functional>
#include <FunctionalInterrupt.h>
//#include "Events.h"

void ENCTask::setup(){
 gpio_set_direction((gpio_num_t)btn_pin, GPIO_MODE_INPUT);
 btn_semaphore=xSemaphoreCreateBinary();
 attachInterrupt(digitalPinToInterrupt(btn_pin),std::bind(&ENCTask::btnISR, this),CHANGE);
 if (enc1_pin>0 && enc2_pin>0)
 {
 gpio_set_direction((gpio_num_t)enc1_pin, GPIO_MODE_INPUT);
 gpio_set_direction((gpio_num_t)enc2_pin, GPIO_MODE_INPUT);
 attachInterrupt(digitalPinToInterrupt(enc1_pin),std::bind(&ENCTask::encISR, this),CHANGE);
 }
 
 btn.xdbl=0;
 btn.pressed=false;
 esp_timer_create_args_t timer_cfg = {
      .callback = (esp_timer_cb_t)&ENCTask::timerCallback,
      .arg = this};

  if (esp_timer_create(&timer_cfg, &_timer) != ESP_OK)
  {
    _timer = NULL;
    //ESP_LOGE(TAG, "Error creating blinker timer!\r\n");
    return;
  }
};


void ENCTask::btnISR(){
  is_enc=false;  
  xSemaphoreGiveFromISR(btn_semaphore,NULL);
}

void ENCTask::encISR(){
  is_enc=true;
  xSemaphoreGiveFromISR(btn_semaphore,NULL);
}

void ENCTask::loop(){

    if (xSemaphoreTake(btn_semaphore,portMAX_DELAY)){
        event_t ev;
        if (is_enc){//encoder semaphore
        bool s1=gpio_get_level((gpio_num_t)enc1_pin);
        bool s2=gpio_get_level((gpio_num_t)enc2_pin);
        if (s1){
        ev.state=ENCODER_EVENT;    
        ev.button=1;
        if(s2) ev.data=1; else ev.data=2;
        xQueueSend(que,&ev,portMAX_DELAY);
        }
        }
        else {//button semaphore
        bool btnNow=gpio_get_level((gpio_num_t)btn_pin)==btn.level;
        unsigned long ms=millis();
        if (btn.pressed!=btnNow) 
        {
        btn.pressed=btnNow;
        if (btn.pressed){
            btn.presstime=ms;
        }else{
            
            if (ms-btn.presstime >= LONGCLICK){
                //if(btn.xdbl>0) esp_timer_stop(_timer);
                esp_timer_stop(_timer);
                ev.state=BTN_LONGCLICK;
                ev.count=btn.xdbl;
                ev.button=0;
                xQueueSend(que,&ev,portMAX_DELAY);
                btn.xdbl=0;
            }else if (ms-btn.presstime>BOUNCE){
                esp_timer_stop(_timer);
                btn.xdbl++;
                esp_timer_start_once(_timer, DOUBLECLICK*1000);
            }
            
        }
        }    
        
    }
    }
   delay(10);
};

void ENCTask::timerCallback(){
    if (btn.pressed){
        esp_timer_stop(_timer);
        esp_timer_start_once(_timer, DOUBLECLICK*1000);
        return;
    }
    event_t ev;
    ev.state=BTN_CLICK;
    ev.count=btn.xdbl;
    ev.button=0;
    xQueueSend(que,&ev,portMAX_DELAY);    
    btn.xdbl=0;
}

void ENCTask::cleanup(){
if (enc1_pin>0) detachInterrupt((gpio_num_t)enc1_pin);
detachInterrupt((gpio_num_t)btn_pin);
esp_timer_stop(_timer);
esp_timer_delete(_timer);
};

