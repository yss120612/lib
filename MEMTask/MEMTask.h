#ifndef __MEMTASK__
#define __MEMTASK__
#include <Task.h>
#include <Wire.h>

#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIMEOUT 10
#define WAIT_BEFORE_WRITE 20000

template< class T1 >
class MEMTask: public Task
{
public:
MEMTask(const char *name, uint32_t stack,QueueHandle_t q,void (*func)(T1 *),uint8_t(* fnotify)(T1*,event_t *,notify_t), MessageBufferHandle_t a,MessageBufferHandle_t w, uint8_t v,uint8_t adr,uint8_t ofs):Task(name, stack){
    que=q;alarm_mess=a;web_mess=w;VER=v;ADDRESS=adr,OFFSET=ofs;
    func_reset=func;
    process_notify=fnotify;
	_timer=NULL;
    }

void pause() override
{
     if (xTimerDelete(_timer,0)==pdFAIL)
     {
         Serial.println("Timer stop error");
     }
    _timer=NULL;
     Task::pause();    
};

void resume() override
{
	if (!_timer) _timer=xTimerCreate("Memory",pdMS_TO_TICKS( WAIT_BEFORE_WRITE ),pdFALSE , static_cast<void*>(this), MemTimerCb);
	Task::resume();
    //if (!_timer) _timer=xTimerCreate("Memory",pdMS_TO_TICKS( WAIT_BEFORE_WRITE ),pdFALSE , static_cast<void*>(this), MemTimerCb);
    // if (xTimerStart(_timer,0)==pdFAIL)
    // {
    //     Serial.println("Timer start error");
    // }
 };

// void set_reset_func(void (*func)(T1 *)) {
//     func_reset=func;
// };

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
MessageBufferHandle_t alarm_mess;
MessageBufferHandle_t web_mess;
T1 sstate;
TimerHandle_t _timer;
uint8_t VER;
uint8_t ADDRESS;
uint8_t OFFSET;

void setup() override
{
	 Wire.begin();
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
uint8_t resuls;
if (xTaskNotifyWait(0, 0, &command, portMAX_DELAY))	{
	memcpy(&nt,&command,sizeof(command));
	//process_notify=1 mem_read
	//process_notify=2 mem_ask
	//process_notify=3 mem_write
	result=process_notify(&sstate,&ev,nt);
	if (result==2) {
        xQueueSend(que,&ev,0);
    }else if (result==3){
        xTimerStart(_timer,0);
    }
}

};

void reset_memory(){
 func_reset(&sstate);
 xTimerStart(_timer,0);
};

void read_state()
{
	read(0, (uint8_t *)&sstate, sizeof(T1));
	uint8_t crc=crc8((uint8_t *)&sstate,sizeof(T1));
	
	if (crc!=0)//second attempt to read
	{
		read(0, (uint8_t *)&sstate, sizeof(T1));
		crc=crc8((uint8_t *)&sstate,sizeof(T1));
		
	}

	#ifdef DEBUGG
	if (crc!=0)	Serial.println("BAD CRC!!!");
	// Serial.print("SState length=");
	// Serial.println(sizeof(sstate));
	#endif
			

	if (VER!=sstate.version || crc!=0)
	{
		reset_memory();
	}
};

void write_state()
{
	sstate.crc=crc8((uint8_t *)&sstate, sizeof(sstate)-1);
	write(0, (uint8_t *)&sstate, sizeof(sstate));
};

void read(uint16_t index, uint8_t *buf, uint16_t len)
{
	index += OFFSET;
	index &= 0x0FFF;
	uint16_t idx=0;
	uint8_t count=0;
	//Serial.println("Read ");
	for (idx=0;idx<len;idx++){
		if (((index+idx)%EEPROM_PAGE_SIZE==0) || idx==0){
			 Wire.beginTransmission(ADDRESS);
			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
    		 Wire.write(((index+idx) >> 8) & 0x0F);
			 Wire.write((index+idx) & 0xFF);
    		 Wire.endTransmission();
			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
			 if (idx==0){
				count=(index % EEPROM_PAGE_SIZE)==0?len>=EEPROM_PAGE_SIZE?EEPROM_PAGE_SIZE:len:EEPROM_PAGE_SIZE-(index % EEPROM_PAGE_SIZE);
			 }
			 else{
				count=len-idx>=EEPROM_PAGE_SIZE?EEPROM_PAGE_SIZE:len-idx;
			 }
			 Wire.requestFrom(ADDRESS, count);	
			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
			}
			
			*buf = Wire.read();
			*buf ++;
			

	}
};


void write(uint16_t index, const uint8_t *buf, uint16_t len)
{
	index += OFFSET;
	index &= 0x0FFF;
	uint16_t idx=0;
	//Serial.print("Write ");
	for (idx=0;idx<len;idx++){
		if (((index+idx)%EEPROM_PAGE_SIZE==0) || idx==0){
			if (idx>0) {
				Wire.endTransmission();
				vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
				}
    		Wire.beginTransmission(ADDRESS);
			vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
    		Wire.write(((index+idx) >> 8) & 0x0F);
			Wire.write((index+idx) & 0xFF);
			vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
			}
			Wire.write(*buf);
			 //Serial.printf("%d=%d; ",idx,*buf);
			*buf ++;
	}
	//Serial.println(".");
	Wire.endTransmission();
	vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
};



};



#endif