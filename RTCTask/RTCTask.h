#ifndef _RTCTASK_h
#define _RTCASK_h
#include <Arduino.h>
#include <Task.h>
//#include "Settings.h"
//#include <WiFiUdp.h>
//#include <NtpClient.h>
//#include <SPI.h>
#include "YssRTC.h"
#include <GlobalSettings.h>


//#define LED_PIN GPIO_NUM_32
//const ledc_channel_t channels[]={LEDC_CHANNEL_0,LEDC_CHANNEL_1,LEDC_CHANNEL_2,LEDC_CHANNEL_3};

//static const char  dayofweek[] ="\xD0\x92\xD1\x81\x2E\xD0\x9F\xD0\xBD\x2E\xD0\x92\xD1\x82\x2E\xD0\xA1\xD1\x80\x2E\xD0\xA7\xD1\x82\x2E\xD0\x9F\xD1\x82\x2E\xD0\xA1\xD0\xB1\x2E";
static const char  dayofweek[] ="Mon.Tue.Wed.Thu.Fri.Sat.Mon.";
// static const char * NTPServer = "pool.ntp.org";
#define TIME_SHIFT 8 //Временная зона

class RTCTask: public Task{
public:   
    RTCTask(const char *name, uint32_t stack,QueueHandle_t q):Task(name, stack)
    {que=q;init_complete=false;need_watch=false;}
    void needWatch(){need_watch=true;};
    void resetAlarms();
    void init(){init_complete=true;};
protected:
    void initAlarms();
    void saveAlarm(uint8_t idx);
    uint8_t findAndSetNext(DateTime dt, Ds3231Alarm1Mode mode);
    uint8_t findAndSetNext(DateTime dt, Ds3231Alarm2Mode mode);
    bool setupAlarm(uint8_t idx, uint8_t act, uint8_t h, uint8_t m,  period_t p,bool active=true,bool save=true);
    void setupTimer(uint16_t minutes,uint8_t idx, uint8_t act);
    uint8_t refreshAlarms();
    void resetAlarm(uint8_t n);
    void alarm(alarm_t &a);
    int minutesLeft(uint8_t timerNo);

    void alarmFired(uint8_t aNo);
    //bool update_time_from_inet();
    void cleanup() override;
    void setup() override;
    void loop() override;
    

    String printTime();
    int32_t last_sync;
    QueueHandle_t que;
    RTC_DS3231 * rtc;
    alarm_t alarms[ALARMS_COUNT];
    bool init_complete;
    bool set_clock;
    bool need_watch;// create everyminutes event at 10 alarm
    
    //bool set_watch;
};
#endif 