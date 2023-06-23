// #include <esp_log.h>
#include "Led.h"
#include <GlobalSettings.h>

static int8_t abs8(int8_t value)
{
  if (value < 0)
    return -value;
  return value;
}

Led::Led(uint8_t pin, uint8_t level, ledc_channel_t channel)
{
  _mode = BLINK_OFF;
  _level = level;
  _channel = channel;
  _value = 0;
  _pin = pin;
  _brightness = 255;

  // pinMode(pin,OUTPUT);
  gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);

  ledc_channel_config_t ledc_channel_cfg1;
  ledc_channel_cfg1.gpio_num = _pin;
  ledc_channel_cfg1.speed_mode = SPEED_MODE;
  ledc_channel_cfg1.intr_type = LEDC_INTR_DISABLE;
  ledc_channel_cfg1.channel = _channel;
  ledc_channel_cfg1.timer_sel = TIMER_NUM;
  ledc_channel_cfg1.duty = _level ? 0 : _brightness;
  ledc_channel_cfg1.hpoint = 255;
  ledc_channel_cfg1.flags.output_invert = !_level;
  if (ledc_channel_config(&ledc_channel_cfg1) != ESP_OK)
  {
    //  Serial.println("ERROR CONFIGURE CHANNEL");
  }
  else
  {
    // Serial.println("OK CONFIGURE CHANNEL");
  }
  // if (!level) ledc_d
}

Led::~Led()
{
}

void Led::setMode(blinkmode_t mode)
{
  _mode = mode;
  _value = 0;
}
