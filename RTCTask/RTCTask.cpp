#include "RTCTask.h"
//#include "Events.h"

void RTCTask::setup()
{
rtc= new RTC_DS3231();
rtc->begin();
fast_time_interval=true;
last_sync=0;
vTaskDelay(pdMS_TO_TICKS(5000));
initAlarms();
init_complete=false;
set_watch=false;
}

void RTCTask::initAlarms(){
event_t e;
e.state=MEM_EVENT;
e.button=200;
xQueueSend(que,&e,portMAX_DELAY);
}

void RTCTask::resetAlarms(){
  event_t e;
  e.state=MEM_EVENT;
  e.button=201;
  init_complete=false;
  xQueueSend(que,&e,portMAX_DELAY);
}



void RTCTask::cleanup()
{
delete rtc;
}

int RTCTask::minutesLeft(uint8_t timerNo){
 if (timerNo>2) return -1;
  DateTime dt=timerNo==1?rtc->getAlarm1():rtc->getAlarm2();
  DateTime  dt0=rtc->now();
  if (dt.isValid() && dt0.isValid()) return (dt.unixtime()-dt0.unixtime())/60+1;
  return -1;
}

//установка конкретного будильника
void RTCTask::alarm(alarm_t &a){
  a.active=true;
  DateTime dt;
  
  switch (a.period)
  {
    case  EVERYMINUTE_ALARM:
    dt=dt+TimeSpan(0,0,0,5);
    rtc->setAlarm1(dt,DS3231_A1_Second);  
    //Serial.print("alarm--");
    break;
    case  ONCE_ALARM:
    case  EVERYHOUR_ALARM:
    case  EVERYDAY_ALARM:
      rtc->setAlarm2(dt+TimeSpan(0,a.hour,a.minute,0),DS3231_A2_Hour);  
      break;
    case  WDAY_ALARM:
    case  HDAY_ALARM:
    case  WD1_ALARM:
    case  WD2_ALARM:
    case  WD3_ALARM:
    case  WD4_ALARM:
    case  WD5_ALARM:
    case  WD6_ALARM:
    case  WD7_ALARM:
      rtc->setAlarm2(dt+TimeSpan(a.wday+1>6?0:a.wday+1,a.hour,a.minute,0),DS3231_A2_Day);
    break;
  }
}





void RTCTask::alarmFired(uint8_t an){

DateTime dt;
event_t ev;
uint8_t idx;

if (an==1){
//Serial.println("Alarm 1 fired");
rtc->clearAlarm(1);
dt=rtc->getAlarm1();
idx=findAndSetNext(dt,rtc->getAlarm1Mode());
//Serial.println("Alarm fired");
if (idx==ALARMS_COUNT-1){
    ev.button=1;
    dt=rtc->now();
    ev.alarm.hour=dt.hour();
    ev.alarm.minute=dt.minute();
    ev.alarm.wday=dt.dayOfTheWeek();
    ev.alarm.period=(period_t)dt.month();
    ev.count=dt.day();
    ev.state=(buttonstate_t)(dt.year()-2000);
    xMessageBufferSend(disp_mess, &ev, sizeof(event_t), portMAX_DELAY);
    idx=refreshAlarms();
  }
}else if(an==2){
dt=rtc->getAlarm2();
rtc->clearAlarm(2);
idx=findAndSetNext(dt,rtc->getAlarm2Mode());
if (idx<ALARMS_COUNT){
ev.state=RTC_EVENT;
ev.button=alarms[idx].action;
ev.alarm=alarms[idx];
xQueueSend(que,&ev,portMAX_DELAY);
idx=refreshAlarms();
}
}
}

void RTCTask::resetAlarm(uint8_t n){
  if (n<ALARMS_COUNT){
    alarms[n].active=false;
    saveAlarm(n);
    refreshAlarms();
  }
}

uint8_t RTCTask::findAndSetNext(DateTime dt, Ds3231Alarm1Mode mode){
  uint8_t result=ALARMS_COUNT;
for (int i=0;i<ALARMS_COUNT;i++){
  if (!alarms[i].active) continue;
  
  if (mode==DS3231_A1_Second){
      result=i;
      break;
    }
}
if (result<ALARMS_COUNT) 
{
getNext(alarms[result]);
//saveAlarm(result);
}

return result;
}


//find fired alarm and set for this alarm next period
uint8_t RTCTask::findAndSetNext(DateTime dt, Ds3231Alarm2Mode mode){
  uint8_t result=ALARMS_COUNT;
for (int i=0;i<ALARMS_COUNT;i++){
  if (!alarms[i].active) continue;
  
  if (alarms[i].hour==dt.hour() && alarms[i].minute==dt.minute()){
    
    if (mode==DS3231_A2_Hour){
      if (alarms[i].period==EVERYHOUR_ALARM || alarms[i].period==EVERYDAY_ALARM || alarms[i].period==ONCE_ALARM) {result=i;break;}
    }
    else{
      if (dt.dayOfTheWeek()==alarms[i].wday){result=i;break;}
    }

  }
}
if (result<ALARMS_COUNT) 
{
getNext(alarms[result]);
saveAlarm(result);
}

return result;
}

void RTCTask::saveAlarm(uint8_t idx){
if (alarms[idx].period==EVERYMINUTE_ALARM) return;
event_t ev;
ev.state=MEM_EVENT;
ev.button=idx+100;
ev.alarm=alarms[idx];
xQueueSend(que,&ev,portMAX_DELAY);
}

void RTCTask::setupTimer(uint16_t minutes,uint8_t idx, uint8_t act){
if (idx>=ALARMS_COUNT) return;
alarms[idx].active=true;
alarms[idx].action=act;
alarms[idx].period=ONCE_ALARM;
DateTime dt=rtc->now();
TimeSpan ts(minutes*60);
dt=dt+ts;
alarms[idx].hour=dt.hour();
alarms[idx].minute=dt.minute();
saveAlarm(idx);
}


bool RTCTask::setupAlarm(uint8_t idx, uint8_t act, uint8_t h, uint8_t m,  period_t p){
if (idx>=ALARMS_COUNT) return false;
alarms[idx].active=true;
alarms[idx].action=act;
alarms[idx].minute=m;
alarms[idx].period=p;


DateTime dt=rtc->now();
uint16_t amin=h*60+m;
uint16_t nmin=dt.hour()*60+dt.minute();
uint8_t dw=dt.dayOfTheWeek();

if (p>=WD7_ALARM) {dw=(uint8_t)p-(uint8_t)WD7_ALARM;}
else if (p==WDAY_ALARM) {
if (dw>5||dw==0) {dw=1;} 
else if (nmin>=amin) {dw=dw<5?dw+1:1;}
}else if (p==HDAY_ALARM) {
if (dw>0 && dw<6) {dw=6;} 
else if (nmin>=amin) {dw=dw==6?0:6;}
}else if (p==ONCE_ALARM || p==EVERYDAY_ALARM){

}else if (p==EVERYHOUR_ALARM){
  h=dt.hour();
  if(m<=dt.minute()) h++;
  if (h>23) h=0;
}

alarms[idx].hour=h;
alarms[idx].wday=dw;
saveAlarm(idx);
return true;
}


//find and set next nearest alarm
uint8_t RTCTask::refreshAlarms(){
//rtc->clearAlarm(1);
rtc->clearAlarm(2);
uint8_t index=ALARMS_COUNT;
uint16_t amin,nmin,cdiff,min_diff=WEEK+1;//week and one minutes
DateTime d=rtc->now();
for (uint8_t i=0;i<ALARMS_COUNT;i++){
  if (!alarms[i].active) continue;
  amin=alarms[i].hour*60+alarms[i].minute;
  nmin=d.hour()*60+d.minute();
  switch (alarms[i].period)
  {
  case EVERYMINUTE_ALARM:
      amin=nmin+1;
  break;
  case EVERYHOUR_ALARM:
    while(amin<=nmin) amin+=60;
  break;
  case EVERYDAY_ALARM:
  case ONCE_ALARM:
  if (amin<=nmin) amin+=DAY;
    break;
  default:
  amin+=(alarms[i].wday*DAY+((d.dayOfTheWeek()>alarms[i].wday)?WEEK:0));
  nmin+=d.dayOfTheWeek()*DAY;
  if (amin<=nmin){ 
  amin+=WEEK;
  }
    break;
  }
cdiff=amin-nmin;  
#ifdef DEBUGG
Serial.printf("Diff=%d ", cdiff);
Serial.print(printAlarm(alarms[i]).c_str());
#endif
if (cdiff<min_diff){
  min_diff=cdiff;
  index=i;
}
}
if (index<ALARMS_COUNT) {
alarm(alarms[index]);
}
return index;
}

void RTCTask::loop()
{
      event_t ev;
      SystemState_t sst;
      if (xMessageBufferReceive(alarm_mess,&sst,SSTATE_LENGTH,!init_complete?pdMS_TO_TICKS(1000):0)==SSTATE_LENGTH){
      memcpy(alarms,sst.alr,sizeof(alarm_t)*ALARMS_COUNT);
      refreshAlarms();  
      init_complete=true;
      
      ev.state=MEM_EVENT;//init other devices
      ev.button=INITRELAYS;
      ev.data=sst.rel[0] & 1 | sst.rel[1]<<1 & 2 | sst.rel[2]<<2 & 4 | sst.rel[3]<<3 & 8; 
      xQueueSend(que,&ev,portMAX_DELAY);
      ev.button=INITLEDS;
      //ev.data=sst.br[2].value<<24 & 0xFF000000 | sst.br[1].value<<16 & 0x00FF0000 | sst.br[0].value << 8 & 0x0000FF00 | sst.br[2].stste << 4 & 0x000000F0 | sst.br[1].stste  & 0x0000000F;
      //ev.count=sst.br[0].stste;
      ev.data= sst.br[3].value<<24 & 0xFF000000 | sst.br[2].value<<16 & 0x00FF0000 | sst.br[1].value<<8 & 0x0000FF00 | sst.br[0].value & 0x000000FF;
      ev.count=sst.br[3].stste<<12 & 0xF000     | sst.br[2].stste<<8 &  0x0F00     | sst.br[1].stste<<4 & 0x00F0     | sst.br[0].stste & 0x000F;
      xQueueSend(que,&ev,portMAX_DELAY);
    }

    if (init_complete && need_watch && !set_clock)  {
     set_clock=true;
     setupAlarm(ALARMS_COUNT-1,ALARMS_COUNT-1,0,0,EVERYMINUTE_ALARM);
     refreshAlarms();  
    }

    uint32_t command;
    notify_t nt;   
    if (xTaskNotifyWait(0, 0, &command, init_complete?pdMS_TO_TICKS(1000):0))
    {
         
        memcpy(&nt,&command,sizeof(command));
        switch (nt.title)
        {
        case ALARMSETUP:
          setupAlarm(nt.alarm.action,nt.alarm.action,nt.alarm.hour,nt.alarm.minute,nt.alarm.period);
          #ifdef DEBUGG
          portENTER_CRITICAL(&_mutex);
          Serial.print(printAlarm(alarms[nt.alarm.action]).c_str());   
          portEXIT_CRITICAL(&_mutex);
        #endif
          refreshAlarms();
          break;
        case RTCSETUPTIMER:
          setupTimer(nt.packet.value,nt.packet.var,nt.packet.var);
          refreshAlarms();
        break;
        case RTCALARMTIMELEFT_ASK:
        {
            ev.state=RTC_EVENT;
            ev.button=RTCALARMTIMELEFT_TAKE;
            ev.count=nt.packet.var;
            int tl=minutesLeft(nt.packet.var);
            ev.data=tl>=0?tl:999;
            xQueueSend(que,&ev,portMAX_DELAY);
        }
        break;
        case RTCALARMRESET:
          resetAlarm(nt.packet.var);
        break;
        case RTCGETTIME:{
            ev.state=DISP_EVENT;
            ev.button=SHOWTIME;
            if (fast_time_interval){
              ev.alarm.hour=25;
              //res = snprintf(buf, sizeof(buf), "%s","Time is not*syncronized**");
            }else{
              DateTime dt=rtc->now();
              ev.alarm.hour=dt.hour();
              ev.alarm.minute=dt.minute();
              ev.alarm.wday=dt.dayOfTheWeek();
              ev.alarm.action=dt.month();
              ev.alarm.period=(period_t)dt.day();
              
            }
            //si=xMessageBufferSend(disp_mess,buf,res,portMAX_DELAY);
            xQueueSend(que,&ev,portMAX_DELAY);
            break;
            }
        case ALARMSPRINT:
        {
          #ifdef DEBUGG
            for (uint8_t ii=0;ii<ALARMS_COUNT;ii++){
              
                Serial.print(printAlarm(alarms[ii]).c_str());
            }
            #endif
        break;  
        }
        case ALARMACTIVEPRINT:{
        DateTime dtm=rtc->getAlarm2();
        #ifdef DEBUGG
              Serial.printf("Active alarm=%02d:%02d Wday=%d\n",dtm.hour(),dtm.minute(),dtm.dayOfTheWeek());
              #endif
        break;   

        } 
        case ALARMSRESET:
        #ifdef DEBUGG
          Serial.print("Reset alarms");
          #endif
          resetAlarms();
          refreshAlarms();
        break;  
       
     }
    }
if (rtc->alarmFired(1)){
alarmFired(1);
}
if (rtc->alarmFired(2)){
alarmFired(2);
}  


//if (xEventGroupWaitBits(flg, FLAG_WIFI, pdFALSE, pdTRUE, portMAX_DELAY) & FLAG_WIFI) {    
if (xEventGroupWaitBits(flg, FLAG_WIFI, pdFALSE, pdTRUE,pdMS_TO_TICKS(1000)) & FLAG_WIFI) {    
unsigned long t= millis();    
if (t < last_sync) last_sync=t;
  if (last_sync==0 || t - last_sync > (fast_time_interval ? SHORT_TIME : LONG_TIME))
  {
    last_sync = t;
    fast_time_interval = !update_time_from_inet();
  }
}

}


String RTCTask::printTime()
{
    DateTime d=rtc->now();
    return d.timestamp();
}
bool RTCTask::update_time_from_inet()
{
  WiFiUDP *ntpUDP;
  NTPClient *timeClient;
  ntpUDP = new WiFiUDP();

  timeClient = new NTPClient(*ntpUDP, NTPServer, 3600 * TIME_OFFSET, 60000 * 60 * 24);
  timeClient->begin();
  bool result=timeClient->forceUpdate();
  Serial.println("GetTime");
  if (result)
  {
    DateTime d(timeClient->getEpochTime());
    rtc->adjust(d);
    //#ifdef DEBUGG
    Serial.println("Success update time from inet. Time is :" + rtc->now().timestamp());
    //#endif
  }
    

  timeClient->end();
  delete timeClient;
  delete ntpUDP;
  return result;
}