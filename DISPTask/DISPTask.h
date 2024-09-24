#ifndef _DISPLAY_h
#define _DISPLAY_h
#include <Arduino.h>
//#include <Task.h>
#include <GlobalSettings.h>
//#include <Wire.h>
//#define U8X8_HAVE_HW_I2C
//#include <U8g2lib.h>
//#include "display.h"
#include "font8x8_basic.h"
// #include "font8x8_basic.h"
/* Control byte for i2c
Co : bit 8 : Continuation Bit 
 * 1 = no-continuation (only one byte to follow) 
 * 0 = the controller should expect a stream of bytes. 
D/C# : bit 7 : Data/Command Select bit 
 * 1 = the next byte or byte stream will be Data. 
 * 0 = a Command byte or byte stream will be coming up next. 
 Bits 6-0 will be all zeros. 
Usage: 
0x80 : Single Command byte 
0x00 : Command Stream 
0xC0 : Single Data byte 
0x40 : Data Stream
*/
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE   0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_HORI_ADDR_MODE     0x00    // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE     0x01    // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0    0xA0    
#define OLED_CMD_SET_SEGMENT_REMAP_1    0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_NORMAL    0xC0
#define OLED_CMD_SET_COM_SCAN_REMAP     0xC8     
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT       0x26
#define OLED_CMD_HORIZONTAL_LEFT        0x27
#define OLED_CMD_VERTICAL_UP            0x29
#define OLED_CMD_VERTICAL_DOWN          0x2A
#define OLED_CMD_DEACTIVE_SCROLL        0x2E
#define OLED_CMD_ACTIVE_SCROLL          0x2F
#define OLED_CMD_VERTICAL               0xA3

#define I2C_ADDRESS 0x3C
#define SPI_ADDRESS 0xFF

//#define LED_PIN GPIO_NUM_32
//const ledc_channel_t channels[]={LEDC_CHANNEL_0,LEDC_CHANNEL_1,LEDC_CHANNEL_2,LEDC_CHANNEL_3};

const uint16_t ACTIVE_TIME = 1000*10;//10 sec
#define I2C_TICKS_TO_WAIT 100

//#define I2C_NUM I2C_NUM_0
#define TOTAL_PAGES 8
#define TOTAL_SEGMENTS 128
#define FONT_HEIGHT 8
#define FONT_WIDTH 8
static uint8_t buffer[TOTAL_PAGES*TOTAL_SEGMENTS+1];
struct Point{
    int x;
    int y;
};
class Display{
public:   
    Display(){};
    //void yss_draw_text(int x,int y,char * text,int text_len,bool invert=false);
    void setup(const char * greeting = NULL);
    void contrast(int contrast);
    void clear(bool ison=false);
    //void scroll(bool up, uint8_t count);
    void rotate(bool rot);
    void set_pixel(uint8_t x, uint8_t y,bool ison);
    Point draw_text(uint8_t x,uint8_t y,const char * text);
    Point center_text(const char * text);
    Point center_text_hor(uint8_t y,const char * text);
    void draw_text_ontime(uint8_t x,uint8_t y,const char * text,int time=3000);
    void center_text_ontime(const char * text,int time=3000);
    void line(int x1, int y1, int x2, int y2, bool invert=true);
    void rectangle(int x1, int y1, int x2, int y2, bool invert=true);
    void triangle(int x1, int y1, int x2, int y2, int x3, int y3, bool invert=true);
    void invert_box(int x1, int y1, int x2, int y2);
    void draw();
protected:

    
    
    static void onTick(TimerHandle_t tm);
    void onMyTick();
   //oid showString(String s,String s1,String s2);
   //U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2=U8G2_SH1106_128X64_NONAME_F_SW_I2C();
    //U8G2_SH1106_128X64_VCOMH0_1_HW_I2C u8g2=U8G2_SH1106_128X64_VCOMH0_1_HW_I2C(U8G2_R2,U8X8_PIN_NONE);
    //SSD1306_t display;
    //boolean display_on;
    unsigned long display_timer;
    //String toShow,toShow1,toShow2;
    //int32_t last_time;
   // MessageBufferHandle_t mess;
    //char buf[DISP_MESSAGE_LENGTH+4];
    TimerHandle_t _timer;
    //uint8_t * _ysspage;
    // void yss_display_text(SSD1306_t * dev, int page, char * text, int text_len, bool ,bool active);
    // void yss_display_image(SSD1306_t * dev, int page, int seg, uint8_t * images, int width);
    // void yss_redraw(SSD1306_t * dev, int row);
    
    //void preparePage(uint8_t pg, uint16_t seg, uint8_t * out_buff);
    //void drawPage(uint8_t * out_buf,uint8_t length);
    void init(int width, int height);

    
};


#endif 