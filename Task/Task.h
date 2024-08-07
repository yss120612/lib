#ifndef __TASK__
#define __TASK__

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <freertos/portmacro.h>
#include <freertos/message_buffer.h>
#include <freertos/timers.h>
#include <GlobalSettings.h>

class Task {
public:
  enum core_t : uint8_t { CORE_0 = 0, CORE_1, CORE_ANY };

  Task(const char *name, uint32_t stack, uint8_t priority = 1, core_t core = CORE_ANY);
  virtual ~Task() {
    destroy();
  }

  operator bool() {
    return _task != NULL;
  }
  bool isRunning();
  //void pause();
  //void resume();
  void destroy();
  void notify(uint32_t value);
  void notify(notify_t nt);
  TaskHandle_t getHandle(){ return _task;}
  static void lock();
  static void unlock();
  virtual void pause();
  virtual void resume();
protected:
  enum flag_t : uint8_t { FLAG_DESTROYING = 1, FLAG_DESTROYED = 2, FLAG_USER = 4 };
  
  virtual void setup() {};
  virtual void loop() = 0;
  virtual void cleanup() {};

  void taskHandler();
  
  static portMUX_TYPE _mutex;
  EventGroupHandle_t _flags;
  TaskHandle_t _task;
};
#endif
