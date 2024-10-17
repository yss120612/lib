#ifndef __MEMTASK__
#define __MEMTASK__
#include <Task.h>
//#include <Wire.h>
#include <driver\i2c.h>
#include <esp_log.h>

#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIMEOUT 10
#define WAIT_BEFORE_WRITE 10000
static const char * MYTAG="Mem";
template< class T1 >
class MEMTask: public Task
{
public:
MEMTask(const char *name, uint32_t stack,QueueHandle_t q,void (*func)(T1 *),uint8_t(* fnotify)(T1*,event_t *,notify_t), T1 * state, uint8_t v,int adr,int ofs):Task(name, stack){
    que=q;
	//web_mess=w;
	VER=v;ADDRESS=adr,OFFSET=ofs;
    func_reset=func;
    process_notify=fnotify;
	_timer=NULL;
	sstate=state;
    }

void pause() override
{
	if (xTimerIsTimerActive){
		write_state();
	}
     if (xTimerDelete(_timer,0)==pdFAIL)
     {
         ESP_LOGE(MYTAG,"Timer stop error");
     }
    _timer=NULL;
     Task::pause();    
};

void resume() override
{
	if (!_timer) _timer=xTimerCreate("Memory",pdMS_TO_TICKS( WAIT_BEFORE_WRITE ),pdFALSE , static_cast<void*>(this), MemTimerCb);
	Task::resume();
}

protected:

static void MemTimerCb(TimerHandle_t tm){
  MEMTask<T1> *lt = static_cast<MEMTask<T1> * >(pvTimerGetTimerID(tm));
  if (lt) lt->timerCallback();
};
void timerCallback()
{
	write_state();
	
};

void (*func_reset)(T1 *);
uint8_t (*process_notify) (T1 *, event_t *, notify_t);

QueueHandle_t que;
//MessageBufferHandle_t web_mess;
T1 * sstate;
TimerHandle_t _timer;
uint8_t VER;
int ADDRESS;
int OFFSET;

void setup() override
{
	vTaskDelay(pdMS_TO_TICKS(2000));
	 read_state();
};

void cleanup() override
{
	
};



void loop() override
{
uint32_t command;
event_t ev;
notify_t nt;
uint8_t result;
ev.count=0;
if (xTaskNotifyWait(0, 0, &command, portMAX_DELAY))	{
	memcpy(&nt,&command,sizeof(command));
	//process_notify=1 mem_read
	//process_notify=2 mem_ask
	//process_notify=3 mem_write
    result=process_notify(sstate,&ev,nt);
	if (result==2) {
		ev.count=1;//also notify web
        xQueueSend(que,&ev,0);
    }else if (result==3){
        xTimerStart(_timer,0);
		//ESP_LOGE("MEM","Timer started!");
	}
    // }else if (result==4){
	// 	xMessageBufferSend(web_mess,(uint8_t *)sstate,sizeof(T1),0);
	// }
}

};

void reset_memory(){
 func_reset(sstate);
 xTimerStart(_timer,0);
};

void read_state()
{
	read(0, (uint8_t *)sstate, sizeof(T1));
	uint8_t crc=crc8((uint8_t *)sstate,sizeof(T1));
	
	if (crc!=0)//second attempt to read
	{
		read(0, (uint8_t *)sstate, sizeof(T1));
		crc=crc8((uint8_t *)sstate,sizeof(T1));
		
	}
	if (crc!=0)//second attempt to read
	{
		ESP_LOGE("MEM","Bad CRC!");
	}

	if (VER!=sstate->version || crc!=0)
	{
		reset_memory();
	}
};

void write_state()
{
	sstate->crc=crc8((uint8_t *)sstate, sizeof(T1)-1);
	write(0, (uint8_t *)sstate, sizeof(T1));
};

void read(uint16_t index, uint8_t *buf, uint16_t len)
{
	index += OFFSET;
	index &= 0x0FFF;
	// uint16_t idx=0;
	// uint8_t count=0;
	uint8_t mem[2];
	mem[0]=index>>8 & 0x0F;
	mem[1]=index & 0xFF;
	
		if (i2c_master_write_read_device(0,ADDRESS,mem,2,buf,len,pdMS_TO_TICKS(3000))!=ESP_OK){
			ESP_LOGE(MYTAG,"Error read in a24c32 %d bytes!",len);
		}else{
			//ESP_LOGE(MYTAG,"Reading is OK!");
		}
};


void write(uint16_t index, const uint8_t *buf, uint16_t len)
{
	if (len==0) return;
	index += OFFSET;
	index &= 0x0FFF;
	uint8_t towrite, mem[2+EEPROM_PAGE_SIZE];
	uint16_t counter=0;
	while (1){
		mem[0]=index>>8 & 0x0F;
		mem[1]=index & 0xFF;
		towrite=EEPROM_PAGE_SIZE-index%EEPROM_PAGE_SIZE;
		towrite=counter+towrite>len?len-counter:towrite;
		memcpy(mem+2,buf,towrite);
		if (i2c_master_write_to_device(0,ADDRESS,mem,towrite+2,pdMS_TO_TICKS(3000))!=ESP_OK){
			ESP_LOGE(MYTAG,"Error writing in a24c32 %d bytes len=%d!",towrite,len);
		}
	else{
			//ESP_LOGE(MYTAG,"Writing is OK %d bytes! len=%d",towrite,len);
		}
		counter+=towrite;
		if (counter>=len) break;
		vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
		index+=towrite;
		buf+=towrite;
	}

	
};



};



#endif