#ifndef _IRTASK_h
#define _IRTASK_h
#include <Task.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
//#include "Settings.h"
//#include "Events.h"
#define IR_PIN GPIO_NUM_34 // pin for IR receiver
#define IR_DEVICE 162

class IRTask: public Task{
public:    
IRTask(const char *name, uint32_t stack, QueueHandle_t q):Task(name, stack){que=q;};

protected:
void cleanup() override;
void setup() override;
void loop() override;
QueueHandle_t que;
IRrecv * irrecv;
decode_results dres;
uint32_t old_command;    
};


#endif 