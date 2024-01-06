#include "RELTask.h"
/////////////#include "Events.h"

void RELTask::setup()
{

  bool need_timer=false;
  event_t e;
  for (uint8_t i = 0; i < RELAYS_COUNT; i++)
    if (rpins[i] > 0)
    {
      e.button=MEM_ASK_10+i;
      xQueueSend(que,&e,portMAX_DELAY);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
    else
    {
      //relay[i] = NULL;
    }
   if (need_timer){
    _timer=xTimerCreate("RelTimer",pdMS_TO_TICKS( 300 ),pdFALSE , static_cast<void*>(this), timerCb);
   }

}

void RELTask::arm(uint8_t i) {
 if (i>=4 || !relay[i].isButton() || !_timer)  return;
relay[i].arm();
xTimerStart(_timer,0);

}

void RELTask::save(uint8_t idx){
        if (idx>=RELAYS_COUNT) return;
        event_t ev;
        ev.state=MEM_EVENT;
        ev.button=MEM_SAVE_10+idx;
        ev.count=1;//copy to www
        relState_t rs;
        rs.ison=relay[idx].isOn();
        rs.type =relay[idx].isButton()?RELTYPE_BUTTON:RELTYPE_SWICH;
        rs.level=_level;
        ev.data=rel_state2uint32(rs);
        xQueueSend(que,&ev,portMAX_DELAY);    
}

void RELTask::loop()
{
  uint32_t command;
  notify_t nt;
  event_t ev;
  uint8_t i;
  relState_t rs;
  if (xTaskNotifyWait(0, 0, &command, portMAX_DELAY))
  {
    memcpy(&nt,&command,sizeof(nt));
    switch (nt.title)
    {
    case MEM_READ_10:
    case MEM_READ_11:
    case MEM_READ_12:
    case MEM_READ_13:

    // 
    rs=uint322rel_state(nt.packet.value);
    if (relay[nt.title-MEM_READ_10].isButton())
    {
      arm(nt.title-MEM_READ_10);
    }
    else{
      relay[nt.title-MEM_READ_10].setState(rs.ison);
      //if (!nt.packet.var) save(nt.title-RELAYSET1);
    }
    
    break;
    case RELAYSWITCH1:
    case RELAYSWITCH2:
    case RELAYSWITCH3:
    case RELAYSWITCH4:
      if (relay[nt.title-RELAYSWITCH1].isButton()) {
        arm(nt.title-RELAYSWITCH1);
      }else{
        relay[nt.title-RELAYSWITCH1].swc();
        save(nt.title-RELAYSWITCH1);
      }
      Serial.println("Tutt");
    break;
    case RELAYALLOFF:
    #ifdef DEBUGG
    Serial.println("All off");
    #endif
    for (i=0;i<RELAYS_COUNT;i++){
    if (!relay[i].isButton())
    {
      relay[i].setOff();
      save(i);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
    }
  break;
  
  case INITRELAYS:
     
    for (i=0;i<RELAYS_COUNT;i++){
    if (!relay[i].isButton())
    {
      relay[i].setState(nt.packet.value>>i & 1);
      #ifdef DEBUGG
      Serial.printf("Relay%d %d \n",i,relay[i].isOn());
      #endif
    }
    }
    
    
  break;
  }
  }
}

void RELTask::cleanup()
{
  if (_timer)
  {
    xTimerDelete(_timer,0);
  }
    // esp_timer_stop(_timer);
    // esp_timer_delete(_timer);
    for (uint8_t i = 0; i < RELAYS_COUNT; i++)
    {
      if (rpins[i] <= 0)
        break;
      gpio_reset_pin((gpio_num_t)rpins[i]);
      //  delete (relay[i]);
    }
  
}

void RELTask::timerCb(TimerHandle_t tm){
  RELTask *lt = static_cast<RELTask * >(pvTimerGetTimerID(tm));
  if (lt) lt->timerCallback();
}


void RELTask::timerCallback()
{
for (uint8_t i = 0; i < RELAYS_COUNT; i++)
    if (rpins[i]>0)
    {
      relay[i].disarm();
    }
//esp_timer_stop(_timer);    
 
}
