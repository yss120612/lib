#ifndef _IRTASK_h
#define _IRTASK_h
#include <Task.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
//#include "Settings.h"
//#include "Events.h"


class IRTask: public Task{
public:    
IRTask(const char *name, uint32_t stack, QueueHandle_t q, uint8_t p, uint16_t d):Task(name, stack){que=q;pin=p;device=d;};

protected:
void cleanup() override;
void setup() override;
void loop() override;
QueueHandle_t que;
IRrecv * irrecv;
decode_results dres;
uint32_t old_command;    
uint8_t pin;
uint16_t device;
};


#endif 