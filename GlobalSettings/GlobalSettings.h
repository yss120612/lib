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
    PROCESS_EVENT,
    TELEGRAM_EVENT,
    ALARM_EVENT
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
    EVERYMINUTE_ALARM,
    ONCE_DATE_ALARM
};

// enum flags_t : uint8_t
// {
#define FLAG_WIFI  BIT0
#define FLAG_MQTT  BIT1
#define FLAG_HTTP  BIT2
//};

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
  blinkmode_t state : 8;
};

static float uint_16_to_float(uint16_t value){
return (value>>8 & 0x00FF)*1.0 + (value & 0x00FF)/100.0;
}

static uint16_t float_to_uint_16(float value){
uint16_t i=(uint16_t)value;
return  ((i<<8) & 0xFF00)|((int)((value-i)*100.0) & 0x00FF);
}

static  uint32_t led_state2uint32(led_state_t ls){
uint32_t res=((ls.value & 0xFF)<<8 | (uint8_t)(ls.state) & 0xFF)  & 0x0000FFFF;
return res;
}

static  led_state_t  uint322led_state(uint32_t ui){
led_state_t ls;
ls.value = (ui >> 8 ) & 0xFF;
ls.state = (blinkmode_t)(ui & 0xFF);
return ls;
}

static  uint32_t rel_state2uint32(relState_t rs){
uint32_t res=((rs.ison & 0x1)<<7 | (rs.level & 1) << 6 | (rs.armed & 1) << 5 | (rs.type & 1) << 4) & 0x000000F0;
return res;
}

static  relState_t  uint322rel_state(uint32_t ui){
relState_t rs; 
rs.ison = (ui >> 7 ) & 0x1;
rs.level = (ui >> 6 ) & 0x1;
rs.armed = (ui >> 5 ) & 0x1;
rs.type = (rel_t)((ui >> 4 ) & 0x1);
rs.dumm = 0;
return rs;
}


static void getNext(alarm_t &at)
{
    switch (at.period)
    {
    case EVERYMINUTE_ALARM:
            at.minute=at.minute<59?at.minute+1:0;
            at.active=true;
            break;
    case ONCE_ALARM:
    case ONCE_DATE_ALARM:
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

static void printAlarm(String &per, alarm_t at)
{
    per = at.active ? "+" : "-";

    switch (at.period)
    {
    case EVERYMINUTE_ALARM:
        per += " Em.";
        break;
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
        per += " Ed.";
        break;
    case EVERYHOUR_ALARM:
        per += " Eh.";
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
    uint8_t res = snprintf(buf, sizeof(buf), "%d %02d:%02d %s (%d)", at.action, at.hour, at.minute, per.c_str(), at.wday);
    if (res >= 0 && res < sizeof(buf))
        per = buf;
    else per="error !";    
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
        float fdata;
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


static void crc8_byte(uint8_t &crc, uint8_t data) {
#if defined (__AVR__)
    // резкий алгоритм для AVR
    uint8_t counter;
    uint8_t buffer;
    asm volatile (
    "EOR %[crc_out], %[data_in] \n\t"
    "LDI %[counter], 8          \n\t"
    "LDI %[buffer], 0x8C        \n\t"
    "_loop_start_%=:            \n\t"
    "LSR %[crc_out]             \n\t"
    "BRCC _loop_end_%=          \n\t"
    "EOR %[crc_out], %[buffer]  \n\t"
    "_loop_end_%=:              \n\t"
    "DEC %[counter]             \n\t"
    "BRNE _loop_start_%="
    : [crc_out]"=r" (crc), [counter]"=d" (counter), [buffer]"=d" (buffer)
    : [crc_in]"0" (crc), [data_in]"r" (data)
    );
#else
    // обычный для всех остальных
    uint8_t i = 8;
    while (i--) {
        crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
        data >>= 1;
    }
#endif
}

static uint8_t crc8(uint8_t *buffer, uint8_t size) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < size; i++) crc8_byte(crc, buffer[i]);
    return crc;
}

// static uint8_t crc8(uint8_t *buffer, uint16_t size) {
//   uint8_t crc = 0;
//   for (uint16_t i = 0; i < size; i++) {
//     uint8_t data = buffer[i];
//     for (uint8_t j = 8; j > 0; j--) {
//       crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
//       data >>= 1;
//     }
//   }
//   return crc;
// }
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
#define RTCTIMELEFT_TAKE 47
#define ALARMSETUP 18
#define RTCALARMFIRED 19
#define ALARMSETFROMMEM 20
#define RTCGETALARM 21
#define WWW_GIVE_DATA 42
#define RTCTIMEADJUST 22
#define VOICECOMMAND 23

//#define GETDATETIME 10

#define LED_CONNECTING 111
#define LED_CONNECTED 112
#define LED_CONNECT_FAILED 113

#define SHOWTIME 1

#define ALARMS_COUNT 10
#define LEDS_COUNT 4
#define RELAYS_COUNT 4


#define MEM_ASK_00 100 //запрос на данные
#define MEM_ASK_01 101
#define MEM_ASK_02 102
#define MEM_ASK_03 103
#define MEM_ASK_04 104
#define MEM_ASK_05 105
#define MEM_ASK_06 106
#define MEM_ASK_07 107
#define MEM_ASK_08 108
#define MEM_ASK_09 109
#define MEM_ASK_10 110
#define MEM_ASK_11 111
#define MEM_ASK_12 112
#define MEM_ASK_13 113
#define MEM_ASK_14 114
#define MEM_ASK_15 115
#define MEM_ASK_16 116
#define MEM_ASK_17 117
#define MEM_ASK_18 118
#define MEM_ASK_19 119
#define MEM_ASK_20 120
#define MEM_ASK_21 121
#define MEM_ASK_22 122
#define MEM_ASK_23 123
#define MEM_ASK_24 124
#define MEM_ASK_25 125
#define MEM_ASK_26 126
#define MEM_ASK_27 127
#define MEM_ASK_28 128
#define MEM_ASK_29 129
#define MEM_ASK_30 130
#define MEM_ASK_31 131

#define ASK_ALL    199

#define MEM_READ_00 50 //отправка данных по запросу
#define MEM_READ_01 51
#define MEM_READ_02 52
#define MEM_READ_03 53
#define MEM_READ_04 54
#define MEM_READ_05 55
#define MEM_READ_06 56
#define MEM_READ_07 57
#define MEM_READ_08 58
#define MEM_READ_09 59
#define MEM_READ_10 60
#define MEM_READ_11 61
#define MEM_READ_12 62
#define MEM_READ_13 63
#define MEM_READ_14 64
#define MEM_READ_15 65
#define MEM_READ_16 66
#define MEM_READ_17 67
#define MEM_READ_18 68
#define MEM_READ_19 69
#define MEM_READ_20 70
#define MEM_READ_21 71
#define MEM_READ_22 72
#define MEM_READ_23 73
#define MEM_READ_24 74
#define MEM_READ_25 75
#define MEM_READ_26 76
#define MEM_READ_27 77
#define MEM_READ_28 78
#define MEM_READ_29 79
#define MEM_READ_30 80
#define MEM_READ_31 81


#define MEM_SAVE_00 200 //команда на запись
#define MEM_SAVE_01 201
#define MEM_SAVE_02 202
#define MEM_SAVE_03 203
#define MEM_SAVE_04 204
#define MEM_SAVE_05 205
#define MEM_SAVE_06 206
#define MEM_SAVE_07 207
#define MEM_SAVE_08 208
#define MEM_SAVE_09 209
#define MEM_SAVE_10 210
#define MEM_SAVE_11 211
#define MEM_SAVE_12 212
#define MEM_SAVE_13 213
#define MEM_SAVE_14 214
#define MEM_SAVE_15 215
#define MEM_SAVE_16 216
#define MEM_SAVE_17 217
#define MEM_SAVE_18 218
#define MEM_SAVE_19 219
#define MEM_SAVE_20 220
#define MEM_SAVE_21 221
#define MEM_SAVE_22 222
#define MEM_SAVE_23 223
#define MEM_SAVE_24 224
#define MEM_SAVE_25 225
#define MEM_SAVE_26 226
#define MEM_SAVE_27 227
#define MEM_SAVE_28 228
#define MEM_SAVE_29 229
#define MEM_SAVE_30 230
#define MEM_SAVE_31 231

static char* http_content_type(char *path) {
    char *ext = strrchr(path, '.');
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".htm") == 0)  return "text/html";
    if (strcmp(ext, ".css") == 0)  return "text/css";
    if (strcmp(ext, ".js") == 0)   return "text/javascript";
    if (strcmp(ext, ".png") == 0)  return "image/png";
    if (strcmp(ext, ".jpg") == 0)  return "image/jpeg";
    if (strcmp(ext, ".ico") == 0)  return "image/x-icon";
    if (strcmp(ext, ".json") == 0) return "application/json";
    return "text/plain";
}

static char * malloc_stringf(const char *format, ...) 
{
  char *ret = nullptr;
  if (format != nullptr) {
    // get the list of arguments
    va_list args;
    va_start(args, format);
    //va_copy(args2, args1);
    // calculate length of resulting string
    int len = vsnprintf(nullptr, 0, format, args);
    
    // allocate memory for string
    if (len > 0) {
      ret = (char*)malloc(len+1);
      if (ret != nullptr) {
        memset(ret, 0, len+1);     
        vsnprintf(ret, len+1, format, args);
      } else {
        //ESP_LOGE("GLOBAL", "Failed to format string: out of memory!");
      };
    };
    va_end(args);
  };
  return ret;
}

#endif
