#include "IRTask.h"
#include <esp_log.h>
#define LOG_LOCAL_LEVEL ESP_LOG_INFO

void IRTask::setup(){
irrecv = new IRrecv(pin);
irrecv->enableIRIn();
}
#define DEBUGG
void IRTask::loop(){
    if (irrecv->decode(&dres))
{       uint32_t command=dres.command;
        if (command && command!=old_command){
        old_command=command;    
        event_t ev;
        ev.state=PULT_BUTTON;
        ev.button=(uint8_t)dres.command;
        ev.count=(uint8_t)dres.address;
        ev.data=dres.decode_type<0?999:(int32_t)dres.decode_type;
        ESP_LOGI("IR","IR Command=%d Address=%d Type=%d",ev.button,ev.count,ev.data);
        if (dres.address==device){
        xQueueSend(que,&ev,portMAX_DELAY);
        }
        
        }
        irrecv->resume();
    }
        
    vTaskDelay(pdMS_TO_TICKS(500));
    old_command=0;

}

void IRTask::cleanup(){
    irrecv->disableIRIn();
    delete irrecv;
}