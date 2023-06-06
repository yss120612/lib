#ifndef _ENCTASK_h
#define _ENCTASK_h
#include <Task.h>
#include <Arduino.h>
//#include "Settings.h"

#define ENCBTN GPIO_NUM_23
#define ENCS1 GPIO_NUM_27
#define ENCS2 GPIO_NUM_26

struct button_t {
  uint8_t pin;
  uint8_t xdbl;
  bool level;
  bool pressed;
  long presstime;
};


class ENCTask: public Task{
public:    
ENCTask(const char *name, uint32_t stack, QueueHandle_t q,  bool lvl):Task(name, stack){que=q;btn.level=lvl;};

protected:
void cleanup() override;
void setup() override;
void loop() override;
QueueHandle_t que;
SemaphoreHandle_t btn_semaphore;
void IRAM_ATTR btnISR();
void IRAM_ATTR encISR();
button_t btn;
bool is_enc;
esp_timer_handle_t _timer;
void timerCallback();
    

};

#endif