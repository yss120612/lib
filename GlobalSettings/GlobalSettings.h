#ifndef __GLOBALSETTINGS__
#define __GLOBALSETTINGS__
#include <Arduino.h>

#define TIME_OFFSET 8             // смещение временной зоны
#define LONG_TIME 60000 * 60 * 24 // one per day
#define SHORT_TIME 60000 * 5      // one per 5 min
#define BOUNCE 20
#define LONGCLICK 1000
#define DOUBLECLICK 700

#define DISP_MESSAGE_LENGTH 120

enum blinkmode_t : uint8_t
{
    BLINK_OFF,
    BLINK_ON,
    BLINK_TOGGLE,
    BLINK_05HZ,
    BLINK_1HZ,
    BLINK_2HZ,
    BLINK_4HZ,
    BLINK_FADEIN,
    BLINK_FADEOUT,
    BLINK_FADEINOUT,
    BLINK_SUNRAISE,
    BLINK_SUNSET
};


enum buttonstate_t : uint8_t
{
    NONE_EVENT,
    BTN_CLICK,
    BTN_LONGCLICK,
    PULT_BUTTON,
    WEB_EVENT,
    MEM_EVENT,
    DISP_EVENT,
    LED_EVENT,
    ENCODER_EVENT,
    RTC_EVENT,
    RX433_EVENT,
    HEATER_EVENT,
    TERMOMETR_EVENT,
    WS_EVENT,
    MENU_EVENT,
    PROCESS_EVENT
};

enum period_t : uint8_t
{
    NONE_ALARM,
    ONCE_ALARM,
    EVERYHOUR_ALARM,
    EVERYDAY_ALARM,
    WDAY_ALARM,
    HDAY_ALARM,
    WD7_ALARM,
    WD1_ALARM,
    WD2_ALARM,
    WD3_ALARM,
    WD4_ALARM,
    WD5_ALARM,
    WD6_ALARM,
    EVERYMINUTE_ALARM
};


enum flags_t : uint8_t
{
    FLAG_WIFI = 1,
    FLAG_MQTT = 2,
    FLAG_MEMREADY = 4
};


enum rel_t
{
    RELTYPE_SWICH,
    RELTYPE_BUTTON
};

struct __attribute__((__packed__)) relState_t
{
    bool ison : 1;
    bool level : 1;
    bool armed : 1;
    rel_t type : 1;
    uint8_t dumm : 4;
};


#define ALARMS_COUNT 10
#define LEDS_COUNT 4
#define RELAYS_COUNT 4

const uint16_t WEEK = 10080; // minutes in week
const uint16_t DAY = 1440;   // minutes in day

struct __attribute__((__packed__)) alarm_t
{
    bool active : 1;
    uint8_t hour : 5;
    uint8_t minute : 6;
    uint8_t action : 4;
    uint8_t wday : 3;
    period_t period : 5;
};

struct __attribute__((__packed__)) bool1_t
{
  bool ison:1;
   bool anyth :1;
};

struct __attribute__((__packed__)) led_state_t
{
  uint8_t value;
  blinkmode_t stste : 8;
};

struct __attribute__((__packed__)) SystemState_t
{
    uint8_t version : 8;
    bool rel[RELAYS_COUNT];
    led_state_t br[LEDS_COUNT];
    alarm_t alr[ALARMS_COUNT];
    uint8_t crc;
};

const uint16_t SSTATE_LENGTH = sizeof(SystemState_t);

static void getNext(alarm_t &at)
{
    switch (at.period)
    {
    case EVERYMINUTE_ALARM:
            at.minute=at.minute<59?at.minute+1:0;
            at.active=true;
            break;
    case ONCE_ALARM:
        at.active = false;
        break;
    case WDAY_ALARM:
        if (at.wday >= 5)
            at.wday = 1;
        else
            at.wday++;
        break;
    case HDAY_ALARM:
        if (at.wday == 6)
            at.wday = 0;
        else
            at.wday = 6;
        break;
    case EVERYDAY_ALARM:
        if (at.wday == 6)
            at.wday = 0;
        else
            at.wday++;
        break;
    case EVERYHOUR_ALARM:
        if (at.hour >= 23)
            at.hour = 0;
        else
            at.hour++;
        break;
    case WD1_ALARM:
    case WD2_ALARM:
    case WD3_ALARM:
    case WD4_ALARM:
    case WD5_ALARM:
    case WD6_ALARM:
    case WD7_ALARM:
        break;
    }
}

static std::string printAlarm(alarm_t at)
{
    std::string per = at.active ? "+" : "-";

    switch (at.period)
    {
    case ONCE_ALARM:
        per += " 1t.";
        break;
    case WDAY_ALARM:
        per += " Wd.";
        break;
    case HDAY_ALARM:
        per += " Hd.";
        break;
    case EVERYDAY_ALARM:
        per += " 1d.";
        break;
    case EVERYHOUR_ALARM:
        per += " 1h.";
        break;
    case WD7_ALARM:
        per += " Vs.";
        break;
    case WD1_ALARM:
        per += " Pn.";
        break;
    case WD2_ALARM:
        per += " Vt.";
        break;
    case WD3_ALARM:
        per += " Sr.";
        break;
    case WD4_ALARM:
        per += " Ct.";
        break;
    case WD5_ALARM:
        per += " Pt.";
        break;
    case WD6_ALARM:
        per += " Sb.";
        break;
    }
    char buf[30];
    uint8_t res = snprintf(buf, sizeof(buf), "%d %02d:%02d %s (%d)\n", at.action, at.hour, at.minute, per.c_str(), at.wday);
    std::string str = "error!";
    if (res >= 0 && res < sizeof(buf))
        str = buf;
    return str;
}

struct event_t
{
    buttonstate_t state;
    uint16_t button;
    uint16_t count;
    // int8_t type;
    union
    {
        uint32_t data;
        alarm_t alarm;
    };

    //  volatile long wait_time;
};

struct __attribute__((__packed__)) notify_packet_t
{
    uint8_t var : 8;
    uint16_t value : 16;
};

struct notify_t
{
    uint8_t title;
    union
    {
        notify_packet_t packet;
        alarm_t alarm;
    };
};



static uint8_t crc8(uint8_t *buffer, uint16_t size) {
  uint8_t crc = 0;
  for (uint16_t i = 0; i < size; i++) {
    uint8_t data = buffer[i];
    for (uint8_t j = 8; j > 0; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}
//LED COMMANDS 

#define LEDSETPARAM1 1
#define LEDSETPARAM2 2
#define LEDSETPARAM3 3
#define LEDSETPARAM4 4

#define LEDBRIGHTNESS1 21
#define LEDBRIGHTNESS2 22
#define LEDBRIGHTNESS3 23
#define LEDBRIGHTNESS4 24
#define LEDBRIGHTNESSALL3 25
    
#define LEDMODE1 31
#define LEDMODE2 32
#define LEDMODE3 33
#define LEDMODE4 34

#define LEDWRITE1 20
#define LEDWRITE2 21
#define LEDWRITE3 22
#define LEDWRITE4 23


#define LEDALLOFF 40


#define RELAYSET1 41
#define RELAYSET2 42
#define RELAYSET3 43
#define RELAYSET4 44

#define RELAYSWITCH1 11
#define RELAYSWITCH2 12
#define RELAYSWITCH3 13
#define RELAYSWITCH4 14

#define RELWRITE1   10
#define RELWRITE2   11
#define RELWRITE3   12
#define RELWRITE4   13

#define RELAYALLOFF 20

#define RELAYPRINTSTATE 23

#define INITRELAYS 203
#define INITLEDS 204

#define ALARMFIRED 0
#define RTCGETTIME 10
#define ALARMSPRINT 11
#define ALARMACTIVEPRINT 12
#define ALARMSRESET 13
#define RTCSETUPTIMER 14
#define RTCALARMRESET 15
#define RTCTIMELEFT_ASK 16
#define RTCTIMELEFT_TAKE 17
#define ALARMSETUP 18
#define RTCALARMFIRED 19



//#define GETDATETIME 10

#define LED_CONNECTING 111
#define LED_CONNECTED 112
#define LED_CONNECT_FAILED 113

#define SHOWTIME 1


#endif
