#ifndef _BMP280TASK_h
#define _BMP280TASK_h
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Task.h>
//#include "Settings.h"

#define SEALEVELPRESSURE_HPA (1013.25)
#define ADDRESS 0x67

class BMP280Task: public Task{
public:
BMP280Task(const char *name, uint32_t stack):Task(name, stack){};

protected:
void cleanup() override;
void setup() override;
void loop() override;
Adafruit_BMP280 * bmp; 
};
#endif