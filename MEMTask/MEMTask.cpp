#include "MEMTask.h"


// template<class T> void MEMTask<T>::setup()
// {
// 	Wire.begin();
// 	read_state();
// 	//_timer=xTimerCreate("MemTimer",pdMS_TO_TICKS( WAIT_BEFORE_WRITE),pdFALSE , static_cast<void*>(this), MemTimerCb);
// 	_timer=NULL;
// }

// template<class T> void MEMTask<T>::cleanup()
// {
	
// }


// template<class T> void MEMTask<T>::pause()
// {
//      if (xTimerDelete(_timer,0)==pdFAIL)
//      {
//          Serial.println("Timer stop error");
//      }
//     _timer=NULL;
//      Task::pause();    
// }

// template<class T> void MEMTask<T>::resume()
// {
// 	Task::resume();
//     if (!_timer) _timer=xTimerCreate("Memory",pdMS_TO_TICKS( WAIT_BEFORE_WRITE ),pdFALSE , static_cast<void*>(this), MemTimerCb);
//     if (xTimerStart(_timer,0)==pdFAIL)
//     {
//         Serial.println("Timer start error");
//     }
//  }


// template<class T1> void MEMTask<T1>::reset_memory(){
// uint8_t i;
		// for(i=0;i<RELAYS_COUNT;i++) sstate.rel[i] = false;
		// for(i=0;i<LEDS_COUNT;i++) {sstate.br[i].value = 0;sstate.br[i].stste=BLINK_OFF;}
		// for (i = 0; i < ALARMS_COUNT; i++)
		// {
		// 	sstate.alr[i].action = 0;
		// 	sstate.alr[i].period = ONCE_ALARM;
		// 	sstate.alr[i].active = false;
		// 	sstate.alr[i].hour = 0;
		// 	sstate.alr[i].minute = 0;
		// 	sstate.alr[i].wday = 0;
		// }
		// sstate.version = VER;
		// xTimerStart(_timer,0);
//}

// template<class T1> void MEMTask<T1>::read_state()
// {
// 	read(0, (uint8_t *)&sstate, sizeof(T1));
// 	uint8_t crc=crc8((uint8_t *)&sstate,sizeof(T1));

// 	if (crc!=0)//second attempt to read
// 	{
// 		read(0, (uint8_t *)&sstate, sizeof(T1));
// 		crc=crc8((uint8_t *)&sstate,sizeof(T1));
// 	}

// 	#ifdef DEBUGG
// 	if (crc!=0)	Serial.println("BAD CRC!!!");
// 	// Serial.print("SState length=");
// 	// Serial.println(sizeof(sstate));
// 	#endif
			

// 	if (VER!=sstate.version || crc!=0)
// 	{
// 		reset_memory();
// 	}
// }

// template<class T1> void MEMTask<T1>::write_state()
// {
// 	sstate.crc=crc8((uint8_t *)&sstate, sizeof(sstate)-1);
// 	write(0, (uint8_t *)&sstate, sizeof(sstate));
// }

// template<class T> void MEMTask<T>::loop()
// {
	// uint32_t command;
	// event_t ev;
	// notify_t nt;
	// if (xTaskNotifyWait(0, 0, &command, portMAX_DELAY))
	// //if (xTaskNotifyWait(0, 0, &command, pdMS_TO_TICKS(1000)))
	// {
		
	// 	memcpy(&nt,&command,sizeof(command));
		
	// 	switch (nt.title)
	// 	{
		
	
	// 	case RELWRITE1:	
	// 	case RELWRITE2:
	// 	case RELWRITE3:
	// 	case RELWRITE4:
	// 	sstate.rel[nt.title-RELWRITE1]=nt.packet.value;
	// 	#ifdef DEBUGG
	// 	Serial.print("Relay");
    //     Serial.print(nt.title-RELWRITE1);
    //     Serial.print(" set ");
    //     Serial.println(nt.packet.value);
	// 	#endif
	// 	//write_state();
	// 	xTimerStart(_timer,0);
	// 	//xTaskNotifyStateClear(NULL);
		
	// 	break;
	// 	case LEDWRITE1:	
	// 	case LEDWRITE2:
	// 	case LEDWRITE3:
	// 	case LEDWRITE4:
	// 	sstate.br[nt.title-LEDWRITE1].stste=(blinkmode_t)nt.packet.var;
	// 	sstate.br[nt.title-LEDWRITE1].value=nt.packet.value;
	// 	#ifdef DEBUGG
	// 	Serial.print("Leds band=");
    //     Serial.print(nt.title-20);
    //     Serial.print(" brightness is ");
    //     Serial.print(nt.packet.value);
	// 	Serial.print(" mode is ");
    //     Serial.println(nt.packet.var);
	// 	#endif
	// 	//write_state();
	// 	xTimerStart(_timer,0);
	// 	//xTaskNotifyStateClear(NULL);
		
	// 	break;
	// 	case 100:	
	// 	case 101:	
	// 	case 102:	
	// 	case 103:	
	// 	case 104:	
	// 	case 105:	
	// 	case 106:	
	// 	case 107:	
	// 	case 108:	
	// 	case 109:	
	// 	sstate.alr[nt.title-100]=nt.alarm;
	// 	//write_state();
	// 	xTimerStart(_timer,0);
	// 	break;
	// 	case 199:
	//    		//xMessageBufferSend(web_mess, &sstate, SSTATE_LENGTH, portMAX_DELAY);
	// 		break;
	// 	case 200:
	//    		xMessageBufferSend(alarm_mess, &sstate, sizeof(T), portMAX_DELAY);
	// 		break;
	// 	case 201://reset alarms
	// 	for (uint8_t i = 0; i < ALARMS_COUNT; i++)
	// 		{
	// 		sstate.alr[i].action = 0;
	// 		sstate.alr[i].period = ONCE_ALARM;
	// 		sstate.alr[i].active = false;
	// 		sstate.alr[i].hour = 0;
	// 		sstate.alr[i].minute = 0;
	// 		sstate.alr[i].wday = 0;
	// 		}
	// 		//write_state();
	// 		xTimerStart(_timer,0);
	//    		xMessageBufferSend(alarm_mess, &sstate, sizeof(T), portMAX_DELAY);
	// 		break;	
	// 		case 202://reset all
	//    		 reset_memory();
	// 		 xMessageBufferSend(alarm_mess, &sstate, sizeof(T), portMAX_DELAY);
	// 		break;
			

	// 	}
	// }
//}

// template<class T1> void MEMTask<T1>::read(uint16_t index, uint8_t *buf, uint16_t len)
// {
// 	index += OFFSET;
// 	index &= 0x0FFF;
// 	uint16_t idx=0;
// 	uint8_t count=0;
// 	//Serial.println("Read ");
// 	for (idx=0;idx<len;idx++){
// 		if (((index+idx)%EEPROM_PAGE_SIZE==0) || idx==0){
// 			 Wire.beginTransmission(ADDRESS);
// 			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
//     		 Wire.write(((index+idx) >> 8) & 0x0F);
// 			 Wire.write((index+idx) & 0xFF);
//     		 Wire.endTransmission();
// 			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
// 			 if (idx==0){
// 				count=(index % EEPROM_PAGE_SIZE)==0?len>=EEPROM_PAGE_SIZE?EEPROM_PAGE_SIZE:len:EEPROM_PAGE_SIZE-(index % EEPROM_PAGE_SIZE);
// 			 }
// 			 else{
// 				count=len-idx>=EEPROM_PAGE_SIZE?EEPROM_PAGE_SIZE:len-idx;
// 			 }
// 			 Wire.requestFrom(ADDRESS, count);	
// 			 vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
// 			}
			
// 			*buf = Wire.read();
// 			*buf ++;
			

// 	}
// }


// template<class T1> void MEMTask<T1>::write(uint16_t index, const uint8_t *buf, uint16_t len)
// {
// 	index += OFFSET;
// 	index &= 0x0FFF;
// 	uint16_t idx=0;
// 	//Serial.print("Write ");
// 	for (idx=0;idx<len;idx++){
// 		if (((index+idx)%EEPROM_PAGE_SIZE==0) || idx==0){
// 			if (idx>0) {
// 				Wire.endTransmission();
// 				vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
// 				}
//     		Wire.beginTransmission(ADDRESS);
// 			vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
//     		Wire.write(((index+idx) >> 8) & 0x0F);
// 			Wire.write((index+idx) & 0xFF);
// 			vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
// 			}
// 			Wire.write(*buf);
// 			 //Serial.printf("%d=%d; ",idx,*buf);
// 			*buf ++;
// 	}
// 	//Serial.println(".");
// 	Wire.endTransmission();
// 	vTaskDelay(pdMS_TO_TICKS(EEPROM_WRITE_TIMEOUT));
// }

// template<class T1> void MEMTask<T1>::MemTimerCb(TimerHandle_t tm){
//   MEMTask<T1> *lt = static_cast<MEMTask<T1> * >(pvTimerGetTimerID(tm));
//   if (lt) lt->timerCallback();
// }


// template<class T1> void MEMTask<T1>::timerCallback()
// {
// 	write_state();
// };