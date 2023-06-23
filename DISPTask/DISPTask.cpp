#include "DISPTask.h"

void DISPTask::setup()
{
  u8g2.begin();
  _timer = xTimerCreate("DispTimer", pdMS_TO_TICKS(3000), pdFALSE, static_cast<void *>(this), onTick);
  u8g2.enableUTF8Print();
}

void DISPTask::onMyTick()
{
  u8g2.clear();
  u8g2.sleepOn();
  xTimerStop(_timer, 0);
}
void DISPTask::onTick(TimerHandle_t tm)
{
  DISPTask *disp = static_cast<DISPTask *>(pvTimerGetTimerID(tm));
  if (disp)
    disp->onMyTick();
}

void DISPTask::loop()
{
  uint32_t command;
  uint8_t s = xMessageBufferReceive(mess, buf, sizeof(buf), portMAX_DELAY);
  if (s)
  {
    String str(buf, s);
    if (str.length() < 1)
      return;
    int f1 = 0;
    int f2 = 0;
    try
    {
      f1 = str.indexOf('*');
      f2 = str.lastIndexOf('*');
    }
    catch (const char *error_message)
    {
#ifdef DEBUGG
      Serial.println(error_message);
#endif
    }
    showString(str.substring(1, f1), str.substring(f1 + 1, f2), str.substring(f2 + 1, str.length()));
  }
  drawText();

  vTaskDelay(1);
}

void DISPTask::drawText()
{

  u8g2.sleepOff();
  // u8g2.setFont(u8g2_font_8x13_tf);
  // u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  u8g2.setFont(u8g2_font_8x13_t_cyrillic);
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(0, 20);
    u8g2.print(toShow);
    if (toShow1 != "")
    {
      u8g2.setCursor(0, 40);
      u8g2.print(toShow1);
    }
    if (toShow2 != "")
    {
      // u8g2.setFont(u8g2_font_8x13_tf);
      u8g2.setCursor(0, 60);
      u8g2.print(toShow2);
      // u8g2.print("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82");
    }
  } while (u8g2.nextPage());
  xTimerStart(_timer, 0);
}

void DISPTask::showString(String s, String s1, String s2)
{
  toShow = s;
  toShow1 = s1;
  toShow2 = s2;
}

void DISPTask::cleanup()
{
  u8g2.setPowerSave(true);
  u8g2.sleepOn();
};
