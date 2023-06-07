#ifndef _RELTASK_h
#define _RELTASK_h
#include <Task.h>
//#include <Arduino.h>
//#include <esp_timer.h>
#include <GlobalSettings.h>
#include "Relay.h"


//#define LED_PIN GPIO_NUM_32
//const ledc_channel_t channels[]={LEDC_CHANNEL_0,LEDC_CHANNEL_1,LEDC_CHANNEL_2,LEDC_CHANNEL_3};




class RELTask: public Task{
public:   
    RELTask(const char *name, uint32_t stack,QueueHandle_t q,const uint8_t rpn [], bool lv=LOW):Task(name, stack){que=q;_level=lv;memcpy(rpins,rpn,RELAYS_COUNT);}
    
   
    //blinkmode_t get_blinkmode(uint8_t idx) {return led[idx]->getMode();}
protected:
    //void setLedMode(uint8_t ledN, blinkmode_t bm);
    void cleanup() override;
    void setup() override;
    void loop() override;
    void timerCallback();
    static void timerCb(TimerHandle_t tm);
    void arm(uint8_t i);
    void save(uint8_t idx);
    int32_t last_time;
    uint16_t _step;
    Relay  relay[RELAYS_COUNT]; 
    uint8_t rpins[RELAYS_COUNT];
    TimerHandle_t _timer;
    QueueHandle_t que;
    //bool need_timer;
    bool _level;
    
};


#endif 