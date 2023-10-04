#ifndef __MEMTASK__
#define __MEMTASK__
#include <Task.h>
#include <Wire.h>

#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIMEOUT 10
#define WAIT_BEFORE_WRITE 20000

class MEMTask: public Task
{
public:
MEMTask(const char *name, uint32_t stack,QueueHandle_t q,MessageBufferHandle_t a,MessageBufferHandle_t w, uint8_t v,uint8_t adr,uint8_t ofs):Task(name, stack){que=q;alarm_mess=a;web_mess=w;VER=v;ADDRESS=adr,OFFSET=ofs;}
void pause() override;
void resume() override;

protected:

void cleanup() override;
void setup() override;
void loop() override;

void read(uint16_t index, uint8_t* buf, uint16_t len);
void write(uint16_t index, const uint8_t* buf, uint16_t len);
void read_state();
void write_state();
void reset_memory();
//void sendBuffer();
    // template<typename T> T & get(uint16_t index, T& t) {
	// 	read(index, (uint8_t*)&t, sizeof(T));
	// 	return t;
	// }
	// template<typename T> const T& put(uint16_t index, const T& t) {
	// 	write(index, (const uint8_t*)&t, sizeof(T));
	// 	return t;
	// }

static void MemTimerCb(TimerHandle_t tm);
void timerCallback();
void * saved_object;
QueueHandle_t que;
MessageBufferHandle_t alarm_mess;
MessageBufferHandle_t web_mess;
SystemState_t sstate;
TimerHandle_t _timer;
uint8_t VER;
uint8_t ADDRESS;
uint8_t OFFSET;
};
#endif