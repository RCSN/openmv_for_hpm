/******************************Copyright(C) CN*******************************
File name  : Bsplcd
Description: ST7735R
Platform   : MDK V5.23.0.0
Version    : V1.0
Author     : RCSN
Create Time: 2019.2.23
Modify     :
Modify Time:
Remarks:
//
******************************************************************************/
#ifndef _BSP_LCD_H_
#define _BSP_LCD_H_

#include "fonts.h"
#include <stdbool.h>
#include "board.h"
#include "hpm_gpio_drv.h"
//#define ST7735R		1
//#define ILI9341		2
#define ST7789V		    3

//#define USE_DMA

#define  LCD_GPIO_CTRL        HPM_GPIO0
#ifdef ILI9341
#define  LCD_LED_GPIO_Port  	GPIOD
#define  LCD_LED_Pin        	GPIO_PINS_9
#define  LED_PIN_WRITE(bit)     gpio_bits_write(LCD_LED_GPIO_Port, LCD_LED_Pin, bit);
#elif ST7789V
#define  LCD_LED_GPIO_Port  	GPIO_DO_GPIOF
#define  LCD_LED_Pin        	0
#define  LED_PIN_WRITE(bit)     gpio_write_pin(LCD_GPIO_CTRL,LCD_LED_GPIO_Port, LCD_LED_Pin, bit);
#endif

#define LCD_SPI                 SPI3
#define LCD_SPI_CLK             CRM_SPI3_PERIPH_CLOCK
#define LCD_SPI_GPIO            GPIOC
#define LCD_SPI_GPIO_CLK        CRM_GPIOC_PERIPH_CLOCK 
#define LCD_SPI_PIN_SCK         GPIO_PINS_10
#define LCD_SPI_PIN_MISO        GPIO_PINS_11
#define LCD_SPI_PIN_MOSI        GPIO_PINS_12 


#define  LCD_RST_GPIO_Port      GPIO_DO_GPIOF
#define  LCD_RST_Pin            1
#define  LCD_RS_GPIO_Port       GPIO_DO_GPIOF
#define  LCD_RS_Pin             2
#define  LCD_CS_GPIO_Port       GPIO_DO_GPIOB
#define  LCD_CS_Pin             24

#define RST_PIN_WRITE(bit)      gpio_write_pin(LCD_GPIO_CTRL, LCD_RST_GPIO_Port, LCD_RST_Pin, bit);
#define RS_PIN_WRITE(bit)       gpio_write_pin(LCD_GPIO_CTRL, LCD_RS_GPIO_Port, LCD_RS_Pin, bit);
#define CS_PIN_WRITE(bit)       gpio_write_pin(LCD_GPIO_CTRL,LCD_CS_GPIO_Port, LCD_CS_Pin, bit);


#ifdef USE_DMA

#define LCD_SPI_MASTER_DMA             HPM_HDMA
#define LCD_SPI_MASTER_Tx_DMA_IRQn     IRQn_HDMA
#define LCD_SPI_MASTER_DMAMUX          HPM_DMAMUX

//#define LCD_SPI_MASTER_DR_Base			    (uint32_t)(&(SPI3->dt)); 
#endif

#define TRUE      (1)
#define FALSE     (0)

#define LCD_MADCTL_MY  0x80
#define LCD_MADCTL_MX  0x40
#define LCD_MADCTL_MV  0x20
#define LCD_MADCTL_ML  0x10
#define LCD_MADCTL_RGB 0x00
#define LCD_MADCTL_BGR 0x08
#define LCD_MADCTL_MH  0x04

//#define LCD_DR_CW          1  //
//#define LCD_DR_CCW         2  //
//#define LCD_DR_SCR180      3  //
#define LCD_DR_SCR360      4  //
#ifdef ST7735R


	#define LCD_IS_160X128 1
	#define LCD_IS_OFFSET 1  //,1

	#ifdef LCD_DR_SCR180
			#define LCD_WIDTH  128
			#define LCD_HEIGHT 160
	#elif  LCD_DR_CW
			#define LCD_WIDTH  160
			#define LCD_HEIGHT 128
	#elif  LCD_DR_CCW
			#define LCD_WIDTH  160
			#define LCD_HEIGHT 128
	#elif  LCD_DR_SCR360
			#define LCD_WIDTH  128
			#define LCD_HEIGHT 160
	#endif

	#if (LCD_IS_OFFSET > 0)
			#define LCD_XSTART 0
			#define LCD_YSTART 0
	#else
			#define LCD_XSTART 0
			#define LCD_YSTART 0
	#endif

	#define LCD_ROTATION (LCD_MADCTL_MX | LCD_MADCTL_MY | LCD_MADCTL_RGB)

	/****************************/

	#define LCD_NOP     0x00
	#define LCD_SWRESET 0x01
	#define LCD_RDDID   0x04
	#define LCD_RDDST   0x09

	#define LCD_SLPIN   0x10
	#define LCD_SLPOUT  0x11
	#define LCD_PTLON   0x12
	#define LCD_NORON   0x13

	#define LCD_INVOFF  0x20
	#define LCD_INVON   0x21
	#define LCD_DISPOFF 0x28
	#define LCD_DISPON  0x29
	#define LCD_CASET   0x2A
	#define LCD_RASET   0x2B
	#define LCD_RAMWR   0x2C
	#define LCD_RAMRD   0x2E

	#define LCD_PTLAR   0x30
	#define LCD_COLMOD  0x3A
	#define LCD_MADCTL  0x36

	#define LCD_FRMCTR1 0xB1
	#define LCD_FRMCTR2 0xB2
	#define LCD_FRMCTR3 0xB3
	#define LCD_INVCTR  0xB4
	#define LCD_DISSET5 0xB6

	#define LCD_PWCTR1  0xC0
	#define LCD_PWCTR2  0xC1
	#define LCD_PWCTR3  0xC2
	#define LCD_PWCTR4  0xC3
	#define LCD_PWCTR5  0xC4
	#define LCD_VMCTR1  0xC5

	#define LCD_RDID1   0xDA
	#define LCD_RDID2   0xDB
	#define LCD_RDID3   0xDC
	#define LCD_RDID4   0xDD

	#define LCD_PWCTR6  0xFC

	#define LCD_GMCTRP1 0xE0
	#define LCD_GMCTRN1 0xE1
#elif ILI9341

	
	#ifdef LCD_DR_SCR180
			#define LCD_WIDTH  240
			#define LCD_HEIGHT 320
			#define LCD_ROTATION 	(LCD_MADCTL_MX | LCD_MADCTL_BGR)
	#elif  LCD_DR_CW
			#define LCD_WIDTH  320
			#define LCD_HEIGHT 240
			#define LCD_ROTATION (LCD_MADCTL_MV | LCD_MADCTL_BGR)
	#elif  LCD_DR_CCW
			#define LCD_WIDTH  320
			#define LCD_HEIGHT 240
			#define LCD_ROTATION (LCD_MADCTL_MX | LCD_MADCTL_MY | LCD_MADCTL_MV | LCD_MADCTL_BGR)
	#elif  LCD_DR_SCR360
			#define LCD_WIDTH  240
			#define LCD_HEIGHT 320
			#define LCD_ROTATION (LCD_MADCTL_MY | LCD_MADCTL_BGR)
	#endif

#elif ST7789V
	#define LCD_CASET   0x2A
	#define LCD_RASET   0x2B
	#define LCD_RAMWR   0x2C

	#define LCD_XSTART 0
    #define LCD_YSTART 20

	#ifdef LCD_DR_SCR180
			#define LCD_WIDTH  240
			#define LCD_HEIGHT 280
			#define LCD_ROTATION 	(LCD_MADCTL_MX | LCD_MADCTL_BGR)
	#elif  LCD_DR_CW
			#define LCD_WIDTH  280
			#define LCD_HEIGHT 240
			#define LCD_ROTATION (LCD_MADCTL_MV | LCD_MADCTL_BGR)
	#elif  LCD_DR_CCW
			#define LCD_WIDTH  280
			#define LCD_HEIGHT 240
			#define LCD_ROTATION (LCD_MADCTL_MX | LCD_MADCTL_MY | LCD_MADCTL_MV | LCD_MADCTL_BGR)
	#elif  LCD_DR_SCR360
			#define LCD_WIDTH  240
			#define LCD_HEIGHT 280
			#define LCD_ROTATION (LCD_MADCTL_MY | LCD_MADCTL_BGR)
	#endif
	
#endif

// Color definitions
#define LCD_BLACK   0x0000
#define LCD_BLUE    0x001F
#define LCD_RED     0xF800
#define LCD_GREEN   0x07E0
#define LCD_CYAN    0x07FF
#define LCD_MAGENTA 0xF81F
#define LCD_YELLOW  0xFFE0
#define LCD_WHITE   0xFFFF
#define COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))


void bsp_lcd_init(void);
void LcdClear(uint16_t color);
void LcdClearTest(void);
void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LcdWriteData(uint32_t len, uint8_t *dat);
void LcdWriteDataDmaSingle(uint16_t len, uint8_t *dat);
void LcdWriteDataDma(uint32_t len, uint8_t *dat);
void LcdWriteData16(uint32_t len, uint16_t *dat);
void LCD_FillRoi(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void LCD_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor);
void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void LCD_FillScreen(uint16_t color);
void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data);
void LCD_InvertColors(bool invert);
void LcdTest(void);
#endif

/******************************Copyright(C) RCSN*******************************/


