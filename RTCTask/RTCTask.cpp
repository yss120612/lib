#include "RTCTask.h"
//#include "Events.h"

void RTCTask::setup()
{
rtc= new RTC_DS3231();
last_sync=0;
vTaskDelay(pdMS_TO_TICKS(5000));
initAlarms();
set_clock=false;
init_complete=false;
}

void RTCTask::initAlarms(){
event_t e;
  e.state=MEM_EVENT;
 for (uint8_t i=0;i<ALARMS_COUNT;i++)
 {
  e.button=MEM_ASK_00+i;
  xQueueSend(que,&e,portMAX_DELAY);
 }
}

void RTCTask::resetAlarms(){
 
  for (uint8_t i;i<ALARMS_COUNT;i++)
 {
  alarms[i].active=false;
 }
}



void RTCTask::cleanup()
{
delete rtc;
}

//возвращает время до срабатывания таймера № в минутах
//если что то не так -1,-2...
int RTCTask::minutesLeft(uint8_t timerNo){
  //ESP_LOGE("RTC","timer %d %d:%d period=%d active=%d",timerNo,alarms[timerNo].hour,alarms[timerNo].minute,alarms[timerNo].period,alarms[timerNo].active);
 if (timerNo>=ALARMS_COUNT) return -1;
 if (alarms[timerNo].period!=ONCE_DATE_ALARM) return -3;
  if (!alarms[timerNo].active) return -2;
  
  DateTime  dt=rtc->getAlarm2();
  if (!dt.isValid()) return -4;
  
   DateTime  dt0=rtc->now();
   
   
   if (!dt0.isValid()) return -5;
   DateTime dt2(dt0.year(),dt0.month(),dt.day(),dt.hour(),dt.minute(),dt.second());
   if (!dt2.isValid()) return -6;
   
   if (dt2.unixtime()>=dt0.unixtime()) return (dt2.unixtime()-dt0.unixtime())/60+1;
   return -7;
  // uint8_t d,m;
  // uint16_t y;
  // d=dt0.day();
  // m=dt0.month();
  // y=dt0.year();
  // if (dt.hour()<dt0.hour()||dt.hour()==dt0.hour()&&dt.minute()<dt0.minute()){
  //   switch (dt0.month()){
  //     case 1:
  //     case 3:
  //     case 5:
  //     case 7:
  //     case 8:
  //     case 10:
  //       if (d==31){d=1;m++;} else d++;
  //     break;
  //     case 2:
  //       if (d==(y%4?28:29)){d=1;m++;} else d++;
  //     break;
  //     case 4:
  //     case 6:
  //     case 9:
  //     case 11:
  //       if (d==30){d=1;m+=1;} else d++;
  //     break;
  //     case 12:
  //       if (d==31){d=1;m=1;y++;} else d++;
  //     break;
  //   }
  // }
  // DateTime  dt1(y,m,d,dt.hour(),dt.minute());
  
}

//установка конкретного будильника
//(конкретное возведение)
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

//заряжаем таймер при помощи аларма idx
//на minutes минут от текущего времени
//возводится конкретный будильник, как в alarm
void RTCTask::setupTimer(uint16_t minutes,uint8_t idx, uint8_t act){
  
  for (uint8_t i;i<ALARMS_COUNT;i++) if(alarms[i].period!=EVERYMINUTE_ALARM){
    alarms[i].active=false;
    saveAlarm(i);
  }
  DateTime dt1(rtc->now().unixtime()+minutes*60);
  setupAlarm(idx,act,dt1.hour(),dt1.minute(),ONCE_DATE_ALARM,true,false);
      rtc->setAlarm2(dt1,DS3231_A2_Date);
  
}

//сработал будильник №
void RTCTask::alarmFired(uint8_t an){

DateTime dt;
event_t ev;
uint8_t idx;

if (an==1){
  
rtc->clearAlarm(1);
dt=rtc->getAlarm1();

idx=findAndSetNext(dt,rtc->getAlarm1Mode());
//Serial.println("Alarm fired");
if (idx==ALARMS_COUNT-1){
  //if (idx==1){
    
    
    dt=rtc->now();
    ev.alarm.hour=dt.hour();
    ev.alarm.minute=dt.minute();
    ev.alarm.wday=dt.dayOfTheWeek();
    ev.alarm.period=(period_t)dt.month();
    ev.count=dt.day()<<8 & 0xFF00 | (dt.year()-2000) & 0x0FFF;
    ev.state=ALARM_EVENT;
    ev.button=idx;
    xQueueSend(que,&ev,portMAX_DELAY);
    idx=refreshAlarms();
    
  }
}else if(an==2){

dt=rtc->getAlarm2();
rtc->clearAlarm(2);

idx=findAndSetNext(dt,rtc->getAlarm2Mode());
if (idx<ALARMS_COUNT){
//ev.state=(buttonstate_t)(ALARM_EVENT<<16 & 0xFF00 | idx & 0x00FF);
ev.state=(buttonstate_t)ALARM_EVENT;
//ev.button=alarms[idx].action;
ev.button=idx;
ev.alarm=alarms[idx];
xQueueSend(que,&ev,portMAX_DELAY);
idx=refreshAlarms();
}
}
}

//сброс будильника №
void RTCTask::resetAlarm(uint8_t n){
  if (n<ALARMS_COUNT){
    
    alarms[n].active=false;
    saveAlarm(n);
    refreshAlarms();
  }
}

//поиск сработавшего аларма (будильник №1)
//и пересчет его нового значения если он переодический
uint8_t RTCTask::findAndSetNext(DateTime dt, Ds3231Alarm1Mode mode){
  uint8_t result=ALARMS_COUNT;
  
for (int i=0;i<ALARMS_COUNT;i++){
  if (!alarms[i].active) continue;
  if (alarms[i].period==EVERYMINUTE_ALARM){
      result=i;
      break;
    }
}
if (result<ALARMS_COUNT) 
{
  //ESP_LOGE("RTC","Alarm 1 no %d", result);
 getNext(alarms[result]);
}

return result;
}


//поиск сработавшего аларма (будильник №2)
//и пересчет его нового значения если он переодический
uint8_t RTCTask::findAndSetNext(DateTime dt, Ds3231Alarm2Mode mode){
  uint8_t result=ALARMS_COUNT;
for (int i=0;i<ALARMS_COUNT;i++){
  if (!alarms[i].active) continue;
  
  if (alarms[i].hour==dt.hour() && alarms[i].minute==dt.minute()){
    if (alarms[i].period==ONCE_DATE_ALARM || alarms[i].period==ONCE_ALARM){
      //alarms[i].active=false;
      result=i;
      break;
    }
    else if (alarms[i].period==EVERYHOUR_ALARM || alarms[i].period==EVERYDAY_ALARM){
      result=i;
      break;
    }
    else{
      if (dt.dayOfTheWeek()==alarms[i].wday){result=i;break;}
    }

  }
}
if (result<ALARMS_COUNT) 
{
getNext(alarms[result]);
//ESP_LOGE("RTC","Alarm 2 no %d", result);
saveAlarm(result);
}

return result;
}

void RTCTask::saveAlarm(uint8_t idx){
if (alarms[idx].period==EVERYMINUTE_ALARM || alarms[idx].period==ONCE_DATE_ALARM) return;
event_t ev;
ev.state=MEM_EVENT;
ev.button=MEM_SAVE_00+idx;
ev.alarm=alarms[idx];
ev.count=1;//copy to www
xQueueSend(que,&ev,portMAX_DELAY);
}



//заряжаем аларм idx
//на конкретные параметры (active & save параметры по умолчанию true)
bool RTCTask::setupAlarm(uint8_t idx, uint8_t act, uint8_t h, uint8_t m,  period_t p,bool active,bool save){
if (idx>=ALARMS_COUNT) return false;
alarms[idx].active=active;
alarms[idx].action=act;
alarms[idx].minute=m;
alarms[idx].period=p;


DateTime dt=rtc->now();
uint16_t amin=h*60+m;
uint16_t nmin=dt.hour()*60+dt.minute();
uint8_t dw=dt.dayOfTheWeek();

if (p>=WD7_ALARM && p<=WD6_ALARM) {dw=(uint8_t)p-(uint8_t)WD7_ALARM;}
else if (p==WDAY_ALARM) {
if (dw>5||dw==0) {dw=1;} 
else if (nmin>=amin) {dw=dw<5?dw+1:1;}
}else if (p==HDAY_ALARM) {
if (dw>0 && dw<6) {dw=6;} 
else if (nmin>=amin) {dw=dw==6?0:6;}
}else if (p==ONCE_ALARM || p==EVERYDAY_ALARM || ONCE_DATE_ALARM){

}else if (p==EVERYHOUR_ALARM){
  h=dt.hour();
  if(m<=dt.minute()) h++;
  if (h>23) h=0;
}

alarms[idx].hour=h;
alarms[idx].wday=dw;
if (save) saveAlarm(idx);
return true;
}


//при каких то изменениях alarm`сов (сработал добавили удалили) 
//пересчитываем и заряжаем ближайший
uint8_t RTCTask::refreshAlarms(){
rtc->clearAlarm(1);
rtc->clearAlarm(2);
uint8_t index=ALARMS_COUNT;
uint16_t amin,nmin,cdiff,min_diff=WEEK+1;//week and one minutes
DateTime d=rtc->now();
DateTime d2=rtc->getAlarm2();
DateTime d0(d.year(),d.month(),d2.day(),d2.hour(),d2.minute(),0);
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
  case ONCE_DATE_ALARM:
     cdiff=(d0.unixtime()-d.unixtime())*60;
  break;  
  default:
  amin+=(alarms[i].wday*DAY+((d.dayOfTheWeek()>alarms[i].wday)?WEEK:0));
  nmin+=d.dayOfTheWeek()*DAY;
  if (amin<=nmin){ 
  amin+=WEEK;
  }
    break;
  }
if (alarms[i].period!=ONCE_DATE_ALARM) cdiff=amin-nmin;  
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
      //SystemState_t sst;
      //if (xMessageBufferReceive(alarm_mess,&sst,SSTATE_LENGTH,!init_complete?pdMS_TO_TICKS(1000):0)==SSTATE_LENGTH){
      //memcpy(alarms,sst.alr,sizeof(alarm_t)*ALARMS_COUNT);
    //   refreshAlarms();  
    //   init_complete=true;
      
    //   ev.state=MEM_EVENT;//init other devices
    //   ev.button=INITRELAYS;
    //   ev.data=sst.rel[0] & 1 | sst.rel[1]<<1 & 2 | sst.rel[2]<<2 & 4 | sst.rel[3]<<3 & 8; 
    //   xQueueSend(que,&ev,portMAX_DELAY);
    //   ev.button=INITLEDS;
    //   //ev.data=sst.br[2].value<<24 & 0xFF000000 | sst.br[1].value<<16 & 0x00FF0000 | sst.br[0].value << 8 & 0x0000FF00 | sst.br[2].stste << 4 & 0x000000F0 | sst.br[1].stste  & 0x0000000F;
    //   //ev.count=sst.br[0].stste;
    //   ev.data= sst.br[3].value<<24 & 0xFF000000 | sst.br[2].value<<16 & 0x00FF0000 | sst.br[1].value<<8 & 0x0000FF00 | sst.br[0].value & 0x000000FF;
    //   ev.count=sst.br[3].stste<<12 & 0xF000     | sst.br[2].stste<<8 &  0x0F00     | sst.br[1].stste<<4 & 0x00F0     | sst.br[0].stste & 0x000F;
    //   xQueueSend(que,&ev,portMAX_DELAY);
    // }

    if (need_watch && !set_clock && init_complete)  {
     set_clock=true;
     setupAlarm(ALARMS_COUNT-1,ALARMS_COUNT-1,0,0,EVERYMINUTE_ALARM);
     refreshAlarms();  
    }
    
    uint32_t command;
    notify_t nt;   
    if (xTaskNotifyWait(0, 0, &command, pdMS_TO_TICKS(5000))==pdTRUE)
    {
         
        memcpy(&nt,&command,sizeof(command));
        //Serial.println(nt.title);
        
        switch (nt.title)
        {
        case ALARMSETUP:

          setupAlarm(nt.alarm.action,nt.alarm.action,nt.alarm.hour,nt.alarm.minute,nt.alarm.period);
          refreshAlarms();
          break;
        case ALARMSETFROMMEM:
          setupAlarm(nt.alarm.action,nt.alarm.action,nt.alarm.hour,nt.alarm.minute,nt.alarm.period,nt.alarm.active,false);
          if(nt.alarm.active) refreshAlarms();
          if (nt.alarm.action==ALARMS_COUNT-1) init_complete=true;
          break;

        case RTCSETUPTIMER:
          setupTimer(nt.packet.value,nt.packet.var,nt.packet.var);
          //refreshAlarms();
        break;
        case RTCTIMELEFT_ASK:
        {
            ev.state=RTC_EVENT;
            ev.button=RTCTIMELEFT_TAKE;
            ev.count=nt.packet.var;
            int tl=minutesLeft(nt.packet.var);
            ev.data=tl>=0?tl:9999+tl;
            xQueueSend(que,&ev,portMAX_DELAY);
        }
        break;
        case RTCALARMRESET:
          resetAlarm(nt.packet.var);
        break;
        case RTCGETTIME:{
            ev.state=DISP_EVENT;
            ev.button=SHOWTIME;
            // if (fast_time_interval){
            //   ev.alarm.hour=25;
            //   //res = snprintf(buf, sizeof(buf), "%s","Time is not*syncronized**");
            // }else{
              DateTime dt=rtc->now();
              ev.alarm.hour=dt.hour();
              ev.alarm.minute=dt.minute();
              ev.alarm.wday=dt.dayOfTheWeek();
              ev.alarm.action=dt.month();
              ev.alarm.period=(period_t)dt.day();
              
            // }
            
            //si=xMessageBufferSend(disp_mess,buf,res,portMAX_DELAY);
            xQueueSend(que,&ev,portMAX_DELAY);
            break;
            }
        case RTCTIMEADJUST:
          if (nt.packet.var==0){
              last_sync=nt.packet.value<<16 & 0xFFFF0000;
          }else{
              last_sync=nt.packet.value & 0xFFFF | last_sync & 0xFFFF0000;
              DateTime d(last_sync);
              //TimeSpan *ts=new TimeSpan(0,TIME_SHIFT,0);
              d=d+TimeSpan(0,TIME_SHIFT,0,0);
              //delete(ts);
              rtc->adjust(d);
              last_sync=0;
              DateTime dt = rtc->now();
              ESP_LOGE("RTC","Success update time from inet. Time is : %02d:%02d",dt.hour(),dt.minute());
          }
        break;    
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
        } 
        case ALARMSRESET:
          resetAlarms();
          refreshAlarms();
        break;  
        case RTCGETALARM:
          ev.state=WEB_EVENT;
          ev.button=WWW_GIVE_DATA;
          ev.alarm=alarms[nt.packet.var];
          xQueueSend(que,&ev,portMAX_DELAY);
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
// if (xEventGroupWaitBits(flg, FLAG_WIFI, pdFALSE, pdTRUE,0) & FLAG_WIFI) {    
// unsigned long t= millis();    
// if (t < last_sync) last_sync=t;
//   if (last_sync==0 || t - last_sync > (fast_time_interval ? SHORT_TIME : LONG_TIME))
//   {
//     last_sync = t;
//     fast_time_interval = !update_time_from_inet();
//   }
// }

}


String RTCTask::printTime()
{
    DateTime d=rtc->now();
    return d.timestamp();
}


// bool RTCTask::update_time_from_inet()
// {
//   WiFiUDP *ntpUDP;
//   NTPClient *timeClient;
//   ntpUDP = new WiFiUDP();

//   timeClient = new NTPClient(*ntpUDP, NTPServer, 3600 * TIME_OFFSET, 60000 * 60 * 24);
//   timeClient->begin();
//   bool result=timeClient->forceUpdate();
  
//   if (result)
//   {
//     DateTime d(timeClient->getEpochTime());
//     rtc->adjust(d);
//     //#ifdef DEBUGG
//     Serial.println("Success update time from inet. Time is :" + rtc->now().timestamp());
//     //#endif
//   }else{
//     Serial.println("Failed update time from inet.");
//   }
    

//   timeClient->end();
//   delete timeClient;
//   delete ntpUDP;
//   return result;
// }