#ifndef _DISPTASK_h
#define _DISPTASK_h
#include <Arduino.h>
#include <Task.h>
#include <GlobalSettings.h>
//#include <Wire.h>
//#define U8X8_HAVE_HW_I2C
//#include <U8g2lib.h>
#include "display.h"
// #include "font8x8_basic.h"

//#define LED_PIN GPIO_NUM_32
//const ledc_channel_t channels[]={LEDC_CHANNEL_0,LEDC_CHANNEL_1,LEDC_CHANNEL_2,LEDC_CHANNEL_3};

const uint16_t ACTIVE_TIME = 1000*10;//10 sec


class DISPTask: public Task{
public:   
    DISPTask(const char *name, uint32_t stack,MessageBufferHandle_t m):Task(name, stack){mess=m;};
    
   

protected:

    void cleanup() override;
    void setup() override;
    void loop() override;
    void drawText();
    static void onTick(TimerHandle_t tm);
    void onMyTick();
    void showString(String s,String s1,String s2);
   //U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2=U8G2_SH1106_128X64_NONAME_F_SW_I2C();
    //U8G2_SH1106_128X64_VCOMH0_1_HW_I2C u8g2=U8G2_SH1106_128X64_VCOMH0_1_HW_I2C(U8G2_R2,U8X8_PIN_NONE);
    SSD1306_t display;
    boolean display_on;
    unsigned long display_timer;
    String toShow,toShow1,toShow2;
    int32_t last_time;
    MessageBufferHandle_t mess;
    char buf[DISP_MESSAGE_LENGTH+4];
    TimerHandle_t _timer;
};


#endif 