#include "DISPTask.h"
#include <esp_log.h>
#include <driver/i2c.h>
#define TAG "YssDisp"

void Display::setup(const char * greeting)
{
	init(TOTAL_SEGMENTS, TOTAL_PAGES / 8);
	_timer = xTimerCreate("DispTimer", pdMS_TO_TICKS(1000), pdFALSE, static_cast<void *>(this), onTick);
	clear();
	if(greeting)
	{ 
	center_text_ontime(greeting);
	}
	else{
	draw();
	}
}

void Display::contrast(int contrast)
{
	uint8_t _contrast = contrast;
	if (contrast < 0x0)
		_contrast = 0;
	if (contrast > 0xFF)
		_contrast = 0xFF;

	uint8_t out_buf[3];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM; // 00
	out_buf[out_index++] = OLED_CMD_SET_CONTRAST;		 // 81
	out_buf[out_index++] = _contrast;

	esp_err_t res = i2c_master_write_to_device(0, I2C_ADDRESS, out_buf, 3, I2C_TICKS_TO_WAIT);
	// i2c_master_transmit(dev_handle, out_buf, 3, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK)
		ESP_LOGE(TAG, "Could not write to device [%d]: %d (%s)", I2C_NUM_0, res, esp_err_to_name(res));
}

void Display::onMyTick()
{
	clear();
	draw();
	xTimerStop(_timer, 0);
}

void Display::onTick(TimerHandle_t tm)
{
	Display *disp = static_cast<Display *>(pvTimerGetTimerID(tm));
	if (disp)
		disp->onMyTick();
}

void Display::clear(bool ison)
{
	memset(buffer, ison ? 0xFF : 0, TOTAL_PAGES * TOTAL_SEGMENTS + 1);
}

void Display::rotate(bool rot)
{
	uint8_t out_buf[3];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;

	if (!rot)
	{
		out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_0; // A0
		out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_NORMAL; // C0
	}
	else
	{
		out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1; // A1
		out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_REMAP;	 // C8
	}
	esp_err_t res;
	res = i2c_master_write_to_device(0, I2C_ADDRESS, out_buf, out_index, I2C_TICKS_TO_WAIT);
	if (res == ESP_OK)
	{
		ESP_LOGI(TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(TAG, "Could not write to device [%d]: %d (%s)", I2C_NUM_0, res, esp_err_to_name(res));
	}
}

void Display::set_pixel(uint8_t x, uint8_t y, bool ison)
{
	if (x<0 || y<0 || x>=TOTAL_SEGMENTS || y>=TOTAL_PAGES*8) return;
	
	if (ison)
	{
		buffer[1 + x + (y / TOTAL_PAGES) * TOTAL_SEGMENTS] |= (1 << (y & 7));
	}
	else
	{
		buffer[1 + x + (y / TOTAL_PAGES) * TOTAL_SEGMENTS] &= ~(1 << (y & 7));
	}
}

Point Display::draw_text(uint8_t x, uint8_t y, const char *text)
{
	uint8_t xpos = x;
	uint8_t lx, ly;
	uint8_t data;
	Point poi;
	int length=strlen(text);
	for (uint8_t i = 0; i < length; i++)
	{
		for (lx = 0; lx < FONT_WIDTH; lx++)
		{
			data = font8x8_basic_tr[(uint8_t)text[i]][lx];
			for (ly = 0; ly < FONT_HEIGHT; ly++)
			{
				set_pixel(xpos, y + ly, data >> ly & 0x1);
			}
			xpos++;
		}
	}
	poi.y = y + FONT_HEIGHT;
	poi.x = xpos - 1;
	return poi;
}

Point Display::center_text(const char *text)
{
	int length=strlen(text);
	int x=(TOTAL_SEGMENTS-(length*FONT_WIDTH))/2;
	int y=(TOTAL_PAGES*8-FONT_HEIGHT)/2;
    return draw_text(x,y,text);
}

Point Display::center_text_hor(uint8_t y, const char *text)
{
	int length=strlen(text);
	int x=(TOTAL_SEGMENTS-(length*FONT_WIDTH))/2;
    return draw_text(x,y,text);
}

void Display::draw_text_ontime(uint8_t x, uint8_t y, const char *text, int time)
{
	draw_text(x,y,text);
	draw_time(time);


}

void Display::center_text_ontime(const char *text, int time)
{
	center_text(text);
	draw_time(time);
}

void Display::draw_time(int ms){
	draw();
	xTimerChangePeriod(_timer,ms,0);
}	

void Display::draw()
{
	esp_err_t res;
	buffer[0] = OLED_CONTROL_BYTE_DATA_STREAM;
	res = i2c_master_write_to_device(I2C_NUM_0, I2C_ADDRESS, buffer, 1 + TOTAL_PAGES * TOTAL_SEGMENTS, I2C_TICKS_TO_WAIT);
	if (res != ESP_OK)
		ESP_LOGE(TAG, "Could not write to device [dev %d]: %d (%s)",
				 I2C_NUM_0, res, esp_err_to_name(res));
}

void Display::init(int width, int height)
{

	uint8_t out_buf[20];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
	// out_buf[out_index++] = OLED_CMD_DISPLAY_OFF;	            // AE
	//  out_buf[out_index++] = OLED_CMD_SET_MUX_RATIO;           // A8
	//  if (height == 64) out_buf[out_index++] = 0x3F;
	//  else if (height == 32) out_buf[out_index++] = 0x1F;

	// out_buf[out_index++] = OLED_CMD_SET_DISPLAY_OFFSET;      // D3
	// out_buf[out_index++] = 0x00;
	// out_buf[out_index++] = OLED_CONTROL_BYTE_DATA_STREAM;	// 40
	// out_buf[out_index++] = OLED_CMD_SET_DISPLAY_START_LINE;	// 40
	// out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP;		// A1
	// if (dev->_flip) {
	// 	out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_0; // A0
	// } else {

	// }
	out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1; // A1
	out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_REMAP;	 // C8
	// out_buf[out_index++] = OLED_CMD_SET_DISPLAY_CLK_DIV;		// D5
	// out_buf[out_index++] = 0x80;
	// out_buf[out_index++] = OLED_CMD_SET_COM_PIN_MAP;			// DA
	// if (height == 64) out_buf[out_index++] = 0x12;
	// else if (height == 32) out_buf[out_index++] = 0x02;

	out_buf[out_index++] = OLED_CMD_SET_CONTRAST; // 81
	out_buf[out_index++] = 0xFF;
	// out_buf[out_index++] = OLED_CMD_DISPLAY_RAM;				// A4
	//  out_buf[out_index++] = OLED_CMD_SET_VCOMH_DESELCT;		// DB
	//  out_buf[out_index++] = 0x40;
	//  out_buf[out_index++] = OLED_CMD_SET_MEMORY_ADDR_MODE;	// 20
	// out_buf[out_index++] = OLED_CMD_SET_HORI_ADDR_MODE;	// 00
	//  out_buf[out_index++] = OLED_CMD_SET_PAGE_ADDR_MODE;		// 02
	//  Set Lower Column Start Address for Page Addressing Mode
	//  out_buf[out_index++] = 0x00;
	//  Set Higher Column Start Address for Page Addressing Mode
	//  out_buf[out_index++] = 0x10;
	out_buf[out_index++] = OLED_CMD_SET_CHARGE_PUMP; // 8D
	out_buf[out_index++] = 0x14;
	out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL; // 2E
	// out_buf[out_index++] = OLED_CMD_DISPLAY_NORMAL;			// A6

	//

	out_buf[out_index++] = OLED_CMD_DISPLAY_ON; // AF
	out_buf[out_index++] = 0x20;				// set screen mode command
	out_buf[out_index++] = 0x00;				// horisontal mode
	out_buf[out_index++] = 0x21;				// hor. diapason
	out_buf[out_index++] = 0x00;				// first segment
	out_buf[out_index++] = TOTAL_SEGMENTS - 1;	// last segment
	out_buf[out_index++] = 0x22;				// vert. diapason
	out_buf[out_index++] = 0x00;				// first page
	out_buf[out_index++] = TOTAL_PAGES - 1;		// last page

	esp_err_t res;
	res = i2c_master_write_to_device(0, I2C_ADDRESS, out_buf, out_index, I2C_TICKS_TO_WAIT);
	// i2c_master_transmit(dev_handle, out_buf, out_index, I2C_TICKS_TO_WAIT);
	if (res == ESP_OK)
	{
		ESP_LOGI(TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(TAG, "Could not write to device [%d]: %d (%s)", I2C_NUM_0, res, esp_err_to_name(res));
	}
}

void Display::line(int x1, int y1, int x2, int y2, bool invert)
{
	int i, dx, dy, sx, sy, E;

	/* distance between two points */
	dx = (x2 > x1) ? x2 - x1 : x1 - x2;
	dy = (y2 > y1) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = (x2 > x1) ? 1 : -1;
	sy = (y2 > y1) ? 1 : -1;

	/* inclination < 1 */
	if (dx > dy)
	{
		E = -dx;
		for (i = 0; i <= dx; i++)
		{
			set_pixel(x1, y1, invert);
			x1 += sx;
			E += 2 * dy;
			if (E >= 0)
			{
				y1 += sy;
				E -= 2 * dx;
			}
		}

		/* inclination >= 1 */
	}
	else
	{
		E = -dy;
		for (i = 0; i <= dy; i++)
		{
			set_pixel(x1, y1, invert);
			y1 += sy;
			E += 2 * dx;
			if (E >= 0)
			{
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}

void Display::rectangle(int x1, int y1, int x2, int y2, bool invert)
{
	line(x1, y1, x2, y1, invert);
	line(x2, y1, x2, y2, invert);
	line(x2, y2, x1, y2, invert);
	line(x1, y2, x1, y1, invert);
}

void Display::triangle(int x1, int y1, int x2, int y2, int x3, int y3, bool invert)
{
	line(x1, y1, x2, y2, invert);
	line(x2, y2, x3, y3, invert);
	line(x3, y3, x1, y1, invert);
}

void Display::invert_box(int x1, int y1, int x2, int y2)
{
	int x01 = (x1 <= x2) ? x1 : x2;
	int x02 = (x1 > x2) ? x1 : x2;
	int y01 = (y1 <= y2) ? y1 : y2;
	int y02 = (y1 > y2) ? y1 : y2;

	for (int i = x01; i <= x02; i++)
		for (int j = y01; j <= y02; j++)
		{
			if (i<0 || j<0 || i>=TOTAL_SEGMENTS || j>=TOTAL_PAGES*8) continue;
			buffer[1 + i + (j / TOTAL_PAGES) * TOTAL_SEGMENTS] ^= (1 << (j & 7));
		}
}
