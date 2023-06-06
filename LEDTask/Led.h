#ifndef _LED_h
#define _LED_h

#include <esp_timer.h>
#include <driver/ledc.h>
#include <GlobalSettings.h>

const ledc_timer_t TIMER_NUM = LEDC_TIMER_2;
const ledc_mode_t SPEED_MODE = LEDC_LOW_SPEED_MODE;

class Led {
public:
   Led(uint8_t pin, uint8_t level, ledc_channel_t channel);
  ~Led();

  
  

  int32_t getValue() const {
    return _value;
  }

  int32_t absValue() const {
    return _value<0?-_value:_value;
  }

  void setValue(int32_t value) {
    _value=value;
  }

  int32_t incValue() {return ++_value;}
  int32_t decValue() {return --_value;}

  blinkmode_t getMode() const {
    return _mode;
  }

  uint8_t getLevel() const{
    return _level;
  }

  uint8_t getPin() const{
    return _pin;
  }
  
  void setMode(blinkmode_t mode);

  ledc_channel_t getChannel(){
    return _channel;
  }

uint8_t getBrightness(){
    return _brightness;
  }

  void setBrightness(uint8_t b){
    _brightness=b;
  }

protected:
    uint8_t _pin;
    uint8_t _level;
    blinkmode_t _mode;
    ledc_channel_t _channel;
    volatile int32_t _value;
    uint8_t _brightness;
};
#endif