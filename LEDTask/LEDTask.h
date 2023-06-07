#ifndef _LEDTASK_h
#define _LEDTASK_h
#include <Task.h>
#include <driver/ledc.h>
#include "Led.h"


//uint8_t band_pins[4];
// = {GPIO_NUM_25, GPIO_NUM_33, GPIO_NUM_32, 0};
const ledc_channel_t channels[] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2, LEDC_CHANNEL_3, LEDC_CHANNEL_4};

#define FADES_SIZE 32
#define SUM_MULT 450//for SUNRICE and SUNSET (пропуск тиков таймера)

class LEDTask: public Task{
public:   
    LEDTask(const char *name, uint32_t stack,QueueHandle_t q,const uint8_t pns[], bool lv=LOW):Task(name, stack){que=q;_level=lv;memcpy(pins,pns,LEDS_COUNT);}
    blinkmode_t get_blinkmode(uint8_t idx) {return led[idx]->getMode();}
protected:
    void save(uint8_t idx);
    void setLedMode(uint8_t ledN, blinkmode_t bm,bool sav=true);
    void setLedBrightness(uint8_t ledN, uint8_t br,bool sav=true);
    static uint8_t getStepValue(uint8_t idx, uint8_t max);
    void reinitDuty(uint8_t idx, uint8_t d);
    void cleanup() override;
    void setup() override;
    void loop() override;
    void timerCallback();
    static void timerCb(TimerHandle_t tm);
    //int32_t last_time;
    uint16_t _step;
    Led * led[LEDS_COUNT]; 
    uint8_t pins[LEDS_COUNT];
    //esp_timer_handle_t _timer;
    TimerHandle_t _timer;
    //bool need_timer;
    uint8_t _level;
    QueueHandle_t que;
};
#endif 