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
#include "bsp_lcd.h"
#include "hpm_debug_console.h"
#include "hpm_spi_drv.h"
#include "hpm_dma_drv.h"
#include "hpm_dmamux_drv.h"
#include "stdio.h"
#include "hpm_l1c_drv.h"
#define LCD_TIMEOUT_VALUE 1000

#define DELAY 0x80



//static void LCD_ExecuteCommandList(const uint8_t *addr);
static void LCD_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);

static uint16_t width = 0;
static uint16_t height = 0;
volatile static uint8_t  tx_dma_complete_flag = 0;
spi_control_config_t control_config = {0};

#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BLACK   0x0000
#define PURPLE  0xF81F

static void spi_transfer_mode_print(spi_control_config_t *config)
{
    uint8_t trans_mode = config->common_config.trans_mode;

    char trans_mode_table[][50] = { "write-read-together",
                                    "write-only",
                                    "read-only",
                                    "write-read",
                                    "read-write",
                                    "write-dummy-read",
                                    "read-dummy-write",
                                    "no-data",
                                    "dummy-write",
                                    "dummy-read"
                                };

   printf("SPI-Master transfer mode:%s\n", trans_mode_table[trans_mode]);
}


const uint16_t LcdColor[] =
{
    0xF800, 0x07E0, 0x001F, 0xFFE0, 0xFFFF, 0x0000, 0xF81F,
};

#ifdef USE_DMA
ATTR_RAMFUNC void dma_isr(void)
{
    volatile hpm_stat_t stat_channel0, stat_channel1;

    stat_channel0 = dma_check_transfer_status(LCD_SPI_MASTER_DMA, DMAMUX_MUXCFG_HDMA_MUX1);
    if(stat_channel0 & DMA_CHANNEL_STATUS_ERROR)
    {
        printf("\r\n Channel 0 transfers error! \r\n");
    }
    if (stat_channel0 & DMA_CHANNEL_STATUS_TC) {
        tx_dma_complete_flag = 1;
        //printf("\r\n Channel 0 transfers done! \r\n");
    }
}
SDK_DECLARE_EXT_ISR_M(LCD_SPI_MASTER_Tx_DMA_IRQn, dma_isr)
#endif
/*!
    \brief      configure the GPIO peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void spi_gpio_config(void)
{
      printf("SPI-Master clk:%d .\n", board_init_spi_clock(BOARD_APP_SPI_BASE));
     
      board_init_spi_pins(BOARD_APP_SPI_BASE);

      gpio_set_pin_output(LCD_GPIO_CTRL, LCD_CS_GPIO_Port, LCD_CS_Pin);
      RST_PIN_WRITE(TRUE);

      gpio_set_pin_output(LCD_GPIO_CTRL, LCD_RST_GPIO_Port, LCD_RS_Pin);
      RST_PIN_WRITE(TRUE);

      gpio_set_pin_output(LCD_GPIO_CTRL, LCD_RS_GPIO_Port, LCD_RST_Pin);
      RS_PIN_WRITE(TRUE);
      
#ifdef ILI9341	
      gpio_set_pin_output(LCD_GPIO_CTRL, LCD_LED_GPIO_Port, LCD_LED_Pin);
      LED_PIN_WRITE(TRUE);
#elif ST7789V
      gpio_set_pin_output(LCD_GPIO_CTRL, LCD_LED_GPIO_Port, LCD_LED_Pin);
      LED_PIN_WRITE(TRUE);
#endif
}

/*!
    \brief      configure the SPI peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void spi_config(void)
{
    spi_timing_config_t timing_config = {0};
    spi_format_config_t format_config = {0};
    spi_master_get_default_timing_config(&timing_config);
    timing_config.master_config.clk_src_freq_in_hz = BOARD_APP_SPI_SCLK_FREQ;
    timing_config.master_config.sclk_freq_in_hz = BOARD_APP_SPI_SCLK_FREQ;
    spi_master_timing_init(BOARD_APP_SPI_BASE, &timing_config);
    printf("SPI-Master transfer timing is configured.\n");
    printf("SPI-Master transfer source clock frequency: %dHz\n", timing_config.master_config.clk_src_freq_in_hz);
    printf("SPI-Master tannsfer sclk frequecny: %dHz\n", timing_config.master_config.sclk_freq_in_hz);

        /* set SPI format config for master */
    spi_master_get_default_format_config(&format_config);
    //format_config.master_config.addr_len_in_bytes = BOARD_APP_SPI_ADDR_LEN_IN_BYTES;
    format_config.common_config.data_len_in_bits = BOARD_APP_SPI_DATA_LEN_IN_BITS;
    format_config.common_config.data_merge = false;
    format_config.common_config.mosi_bidir = false;
    format_config.common_config.lsb = false;
    format_config.common_config.mode = spi_master_mode;
    format_config.common_config.cpol = spi_sclk_low_idle;
    format_config.common_config.cpha = spi_sclk_sampling_odd_clk_edges;
    spi_format_init(BOARD_APP_SPI_BASE, &format_config);
    printf("SPI-Master transfer format is configured.\n");

    /* set SPI control config for master */
    spi_master_get_default_control_config(&control_config);
    control_config.master_config.cmd_enable = false;
    control_config.master_config.addr_enable = false;
    control_config.master_config.addr_phase_fmt = spi_address_phase_format_single_io_mode;
    control_config.common_config.trans_mode = spi_trans_write_only;
    //control_config.common_config.data_phase_fmt = spi_dual_io_mode;
    //control_config.common_config.dummy_cnt = spi_dummy_count_1;
    spi_transfer_mode_print(&control_config);
#ifdef USE_DMA	
 //   BOARD_APP_SPI_BASE->CTRL |= SPI_CTRL_TXDMAEN_MASK;
    intc_m_enable_irq_with_priority(LCD_SPI_MASTER_Tx_DMA_IRQn, 10); //设置中断

    control_config.common_config.tx_dma_enable = true;
    control_config.common_config.rx_dma_enable = false;
    //control_config.common_config.trans_mode = spi_trans_write_dummy_read;
#endif

}

/*********************************************************************************************************
*  : LcdWriteCommandByte
*: ST773R
*    DataByte
*  : 
*
*********************************************************************************************************/
static void LcdWriteCommandByte(uint8_t DataByte)
{
//    uint16_t i;
    uint16_t retry=0;  
    hpm_stat_t spi_stat;
    RS_PIN_WRITE(FALSE); 
    CS_PIN_WRITE(FALSE);
    control_config.common_config.tx_dma_enable = false;
    spi_stat = spi_transfer(BOARD_APP_SPI_BASE, &control_config,
                                    NULL,
                                    NULL,
                                    (uint8_t*) &DataByte, 1,
                                    NULL, 0);

    for (retry = 0; retry < 5; retry++)
    {
        retry = retry;
    }

    CS_PIN_WRITE(TRUE);
}

/*********************************************************************************************************
*  : LcdWriteDataByte
*: ST773R
*    DataByte
*  : 
*
*********************************************************************************************************/
static void LcdWriteDataByte(uint8_t DataByte)
{
//    uint16_t i;
    uint16_t retry=0;    
    hpm_stat_t spi_stat;
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE);  // data
    control_config.common_config.tx_dma_enable = false;
    spi_stat = spi_transfer(BOARD_APP_SPI_BASE, &control_config,
                                    NULL,
                                    NULL,
                                    (uint8_t*) &DataByte, 1,
                                    NULL, 0);

    for (retry = 0; retry < 5; retry++)
    {
        retry = retry;
    }      
    CS_PIN_WRITE(TRUE);
}

/*********************************************************************************************************
*  : LcdWriteCommand
*: ST773R
*    DataByte
                    len
                    dat:
*  : 
*
*********************************************************************************************************/
#if 0
static void LcdWriteCommand(uint8_t DataByte, uint32_t len, uint8_t *dat)
{
    uint32_t i;
    
    LcdWriteCommandByte(DataByte);
    for (i = 0; i < len; i++)
    {
        LcdWriteDataByte(dat[i]);
    }
}
#endif

void LcdWriteDataDmaSingle(uint16_t len, uint8_t *dat)
{   
#ifdef USE_DMA
    uint8_t cmd = 0x1a;
    uint32_t addr = 0x10;

    dma_handshake_config_t config;
    volatile hpm_stat_t stat_channel0;
    hpm_stat_t stat;
    tx_dma_complete_flag = 0;
    control_config.common_config.tx_dma_enable = true;
    stat = spi_setup_dma_transfer(BOARD_APP_SPI_BASE,
                        &control_config,
                        &cmd, &addr,
                        len, 0);
    if (stat != status_success) {
        //printf("spi setup dma transfer failed\n");
    }

    dmamux_config(LCD_SPI_MASTER_DMAMUX, DMAMUX_MUXCFG_HDMA_MUX1, HPM_DMA_SRC_SPI2_TX, true);//绑定mux
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE); // data	
    config.ch_index = DMAMUX_MUXCFG_HDMA_MUX1;
    config.dst = (uint32_t)&BOARD_APP_SPI_BASE->DATA;
    config.dst_fixed = true; //设备地址固定
    config.src = (uint32_t) core_local_mem_to_sys_address(BOARD_RUNNING_CORE, (uint32_t)dat);
    config.src_fixed = false; //内存地址递增
    config.size_in_byte = len;
    dma_setup_handshake(LCD_SPI_MASTER_DMA, &config);
    //while(!tx_dma_complete_flag);
    while(tx_dma_complete_flag == 0)
    {
      cmd = cmd;
    } 
    CS_PIN_WRITE(TRUE);
#else
    hpm_stat_t spi_stat;
    uint16_t retry;
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE); // data	
    spi_stat = spi_transfer(BOARD_APP_SPI_BASE, &control_config,
                                    NULL,
                                    NULL,
                                    dat, len,
                                    NULL, 0);

    for (retry = 0; retry < 5; retry++)
    {
        retry = retry;
    } 
    CS_PIN_WRITE(TRUE);
  
#endif
}

void LcdWriteDataDma(uint32_t len, uint8_t *dat)
{
#ifdef USE_DMA
//    hpm_stat_t stat;
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE); // data	  
    int32_t _len = len;
    int32_t send_len = 0;
    int32_t inc_len = 0;
    volatile uint16_t retry;
    while(_len > 0)
    {
            send_len = (_len > 512 ? 512:_len);
            LcdWriteDataDmaSingle(send_len,&dat[inc_len]);
            for (retry = 0; retry < 3; retry++)
            {
                retry = retry;
            } 
            inc_len += send_len;
            _len    -= send_len;
            
    }
    CS_PIN_WRITE(TRUE);
#endif

}

/*********************************************************************************************************
*  : LcdWriteData
*: ST773R
*    len
                    dat:
*  : 
*
*********************************************************************************************************/
void LcdWriteData(uint32_t len, uint8_t *dat)
{
#ifndef  USE_DMA
    uint32_t i;
    uint16_t retry=0;  
    hpm_stat_t spi_stat;  
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE); // data
    int32_t _len = len;
    int32_t send_len = 0;
    int32_t inc_len = 0;
    while(_len > 0)
    {
            send_len = (_len > 512 ? 512:_len);
            LcdWriteDataDmaSingle(send_len,&dat[inc_len]);
            //board_delay_ms(5);
            inc_len += send_len;
            _len    -= send_len;
            
    }
    CS_PIN_WRITE(TRUE);
#else
      hpm_stat_t spi_stat;
      int32_t _len = len;
      int32_t send_len = 0;
      int32_t inc_len = 0;
      while(_len > 0)
      {
          send_len = (_len > 512 ? 512:_len);               
          uint16_t retry;
          CS_PIN_WRITE(FALSE);
          RS_PIN_WRITE(TRUE); // data	
          control_config.common_config.tx_dma_enable = false;
          spi_stat = spi_transfer(BOARD_APP_SPI_BASE, &control_config,
                                          NULL,
                                          NULL,
                                          &dat[inc_len],send_len,
                                          NULL, 0);

          for (retry = 0; retry < 5; retry++)
          {
              retry = retry;
          } 
          CS_PIN_WRITE(TRUE);
          inc_len += send_len;
          _len    -= send_len;        
      }
#endif
}
/*********************************************************************************************************
*  : LcdWriteData
*: ST773R
*    len
                    dat:
*  : 
*
*********************************************************************************************************/
void LcdWriteData16(uint32_t len, uint16_t *dat)
{
#if 0
    uint32_t i;
    u16 retry=0;   
	spi_init_type spi_init_struct;	
    CS_PIN_WRITE(FALSE);
    RS_PIN_WRITE(TRUE); // data
	spi_enable(LCD_SPI, FALSE);  
	spi_init_struct.frame_bit_num = SPI_FRAME_16BIT;
	spi_init(LCD_SPI, &spi_init_struct);	
	spi_enable(LCD_SPI,TRUE);  
    for (i = 0; i < len; i++)
    {
		while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_TDBE_FLAG) == RESET);
		spi_i2s_data_transmit(LCD_SPI, dat[i]);
		while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_BF_FLAG) == SET) //SPI:
		{
			retry++;
			if(retry>0xFFFE)
			break;
		} 
    }
	spi_enable(LCD_SPI, FALSE); 
	spi_init_struct.frame_bit_num = SPI_FRAME_8BIT;
	spi_init(LCD_SPI, &spi_init_struct);	
	spi_enable(LCD_SPI, TRUE);   
    CS_PIN_WRITE(TRUE);
#endif
}
/*********************************************************************************************************
*  :bsp_lcdInit
*:LCD
*    
*  : 
*
*********************************************************************************************************/
void bsp_lcd_init(void)
{
    width = LCD_WIDTH;
    height = LCD_HEIGHT;   
    /* GPIO configure */
    spi_gpio_config();   
    /* SPI configure */
    spi_config();
    board_delay_ms(500); 
    RST_PIN_WRITE(FALSE);
    board_delay_ms(500);  
    RST_PIN_WRITE(TRUE);
    board_delay_ms(500);
#ifdef ST7735R
    LcdWriteCommandByte(0x11); // Sleep Exit
    board_delay_ms(120);
//  // Memory Data Access Control
    LcdWriteCommandByte(0x36);
#ifdef LCD_DR_SCR180
    LcdWriteDataByte(0xC0);
#elif  LCD_DR_CW
    LcdWriteDataByte(0xA0);
#elif  LCD_DR_CCW
    LcdWriteDataByte(0x60);
#elif  LCD_DR_SCR360
    LcdWriteDataByte(0x00);
#endif
//  // Interface Pixel Format
    LcdWriteCommandByte(0x3A);
    LcdWriteDataByte(0x05);
//  // Display on
    LcdWriteCommandByte(0x29);
#elif ILI9341
//    // SOFTWARE RESET
    LcdWriteCommandByte(0x01);
 //   board_delay_ms(1000);
        
    // POWER CONTROL A
    LcdWriteCommandByte(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        LcdWriteData(sizeof(data),data);
    }

    // POWER CONTROL B
    LcdWriteCommandByte(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        LcdWriteData(sizeof(data),data);
    }

    // DRIVER TIMING CONTROL A
    LcdWriteCommandByte(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        LcdWriteData(sizeof(data),data);
    }

    // DRIVER TIMING CONTROL B
    LcdWriteCommandByte(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        LcdWriteData(sizeof(data),data);
    }

    // POWER ON SEQUENCE CONTROL
    LcdWriteCommandByte(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        LcdWriteData(sizeof(data),data);
    }

    // PUMP RATIO CONTROL
    LcdWriteCommandByte(0xF7);
    {
        uint8_t data[] = { 0x20 };
        LcdWriteData(sizeof(data),data);
    }

    // POWER CONTROL,VRH[5:0]
    LcdWriteCommandByte(0xC0);
    {
        uint8_t data[] = { 0x23 };
        LcdWriteData(sizeof(data),data);
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    LcdWriteCommandByte(0xC1);
    {
        uint8_t data[] = { 0x10 };
        LcdWriteData(sizeof(data),data);
    }

    // VCM CONTROL
    LcdWriteCommandByte(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        LcdWriteData(sizeof(data),data);
    }

    // VCM CONTROL 2
    LcdWriteCommandByte(0xC7);
    {
        uint8_t data[] = { 0x86 };
        LcdWriteData(sizeof(data),data);
    }

    // MEMORY ACCESS CONTROL
    LcdWriteCommandByte(0x36);
    {
        uint8_t data[] = { 0x48 };
        LcdWriteData(sizeof(data),data);
    }

    // PIXEL FORMAT
    LcdWriteCommandByte(0x3A);
    {
        uint8_t data[] = { 0x55 };
        LcdWriteData(sizeof(data),data);
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    LcdWriteCommandByte(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        LcdWriteData(sizeof(data),data);
    }

    // DISPLAY FUNCTION CONTROL
    LcdWriteCommandByte(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        LcdWriteData(sizeof(data),data);
    }

    // 3GAMMA FUNCTION DISABLE
    LcdWriteCommandByte(0xF2);
    {
        uint8_t data[] = { 0x00 };
        LcdWriteData(sizeof(data),data);
    }

    // GAMMA CURVE SELECTED
    LcdWriteCommandByte(0x26);
    {
        uint8_t data[] = { 0x01 };
        LcdWriteData(sizeof(data),data);;
    }

    // POSITIVE GAMMA CORRECTION
    LcdWriteCommandByte(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        LcdWriteData(sizeof(data),data);
    }

    // NEGATIVE GAMMA CORRECTION
    LcdWriteCommandByte(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        LcdWriteData(sizeof(data),data);
    }

    // EXIT SLEEP
    LcdWriteCommandByte(0x11);
    board_delay_ms(120);

    // TURN ON DISPLAY
    LcdWriteCommandByte(0x29);

    // MADCTL
    LcdWriteCommandByte(0x36);
    {
        uint8_t data[] = { LCD_ROTATION };
        LcdWriteData(sizeof(data),data);
    }
	LCD_FillScreen(LCD_WHITE);
#elif ST7789V
//  // Memory Data Access Control
    LcdWriteCommandByte(0x36);
#ifdef LCD_DR_SCR180
    LcdWriteDataByte(0xC0);
#elif  LCD_DR_CW
    LcdWriteDataByte(0xA0);
#elif  LCD_DR_CCW
    LcdWriteDataByte(0x60);
#elif  LCD_DR_SCR360
    LcdWriteDataByte(0x00);
#endif
	LcdWriteCommandByte(0x3A); 
	LcdWriteDataByte(0x05);

	LcdWriteCommandByte(0xB2);
	LcdWriteDataByte(0x0C);
	LcdWriteDataByte(0x0C);
	LcdWriteDataByte(0x00);
	LcdWriteDataByte(0x33);
	LcdWriteDataByte(0x33);

	LcdWriteCommandByte(0xB7); 
	LcdWriteDataByte(0x35);  

	LcdWriteCommandByte(0xBB);
	LcdWriteDataByte(0x19);

	LcdWriteCommandByte(0xC0);
	LcdWriteDataByte(0x2C);

	LcdWriteCommandByte(0xC2);
	LcdWriteDataByte(0x01);

	LcdWriteCommandByte(0xC3);
	LcdWriteDataByte(0x12);   

	LcdWriteCommandByte(0xC4);
	LcdWriteDataByte(0x20);  

	LcdWriteCommandByte(0xC6); 
	LcdWriteDataByte(0x0F);    

	LcdWriteCommandByte(0xD0); 
	LcdWriteDataByte(0xA4);
	LcdWriteDataByte(0xA1);

	LcdWriteCommandByte(0xE0);
	LcdWriteDataByte(0xD0);
	LcdWriteDataByte(0x04);
	LcdWriteDataByte(0x0D);
	LcdWriteDataByte(0x11);
	LcdWriteDataByte(0x13);
	LcdWriteDataByte(0x2B);
	LcdWriteDataByte(0x3F);
	LcdWriteDataByte(0x54);
	LcdWriteDataByte(0x4C);
	LcdWriteDataByte(0x18);
	LcdWriteDataByte(0x0D);
	LcdWriteDataByte(0x0B);
	LcdWriteDataByte(0x1F);
	LcdWriteDataByte(0x23);

	LcdWriteCommandByte(0xE1);
	LcdWriteDataByte(0xD0);
	LcdWriteDataByte(0x04);
	LcdWriteDataByte(0x0C);
	LcdWriteDataByte(0x11);
	LcdWriteDataByte(0x13);
	LcdWriteDataByte(0x2C);
	LcdWriteDataByte(0x3F);
	LcdWriteDataByte(0x44);
	LcdWriteDataByte(0x51);
	LcdWriteDataByte(0x2F);
	LcdWriteDataByte(0x1F);
	LcdWriteDataByte(0x1F);
	LcdWriteDataByte(0x20);
	LcdWriteDataByte(0x23);

	LcdWriteCommandByte(0x21); 
	LcdWriteCommandByte(0x11); 
	board_delay_ms (120); 
	LcdWriteCommandByte(0x29); 
	LCD_FillScreen(LCD_BLUE);
#endif
}

/*********************************************************************************************************
*  : LcdClear
*: LCD
*    color16RGB565
*  : 
*
*********************************************************************************************************/
void LcdClear(uint16_t color)
{
    uint32_t i;
    uint8_t data[2];
    uint32_t lenth = width * height;
    data[0] = color >> 8;
    data[1] = (uint8_t)color;
    LcdWriteCommandByte(0x2C);
    for (i = 0; i < lenth; i++)
    {
//      LcdWriteData(2,data);
        LcdWriteDataByte(data[0]);
        LcdWriteDataByte(data[1]);
    }
}

/*********************************************************************************************************
*  :LcdClearTest
*:LCD
*    
*  : 
*
*********************************************************************************************************/
void LcdClearTest(void)
{
    uint8_t j;
    
    for (j = 0; j < sizeof(LcdColor); j++)
    {
        LcdClear(LcdColor[j]);
        board_delay_ms(500);
    }
}

#if 0
static void LCD_ExecuteCommandList(const uint8_t *addr)
{
    uint8_t numCommands, numArgs;
    uint16_t ms;

    numCommands = *addr++;
    while (numCommands--)
    {
        uint8_t cmd = *addr++;
        LcdWriteCommandByte(cmd);

        numArgs = *addr++;
        // If high bit set, delay follows args
        ms = numArgs & DELAY;
        numArgs &= ~DELAY;
        if (numArgs)
        {
            LcdWriteData(numArgs, (uint8_t *)addr);
            addr += numArgs;
        }

        if (ms)
        {
            ms = *addr++;
            if (ms == 255) ms = 500;
            board_delay_ms(ms);
        }
    }
}
#endif

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
#ifdef ST7735R
    // column address set
    uint8_t data[4] = { 0x00, x0 + LCD_XSTART, 0x00, x1 + LCD_XSTART };
    LcdWriteCommandByte(LCD_CASET);
    LcdWriteData(sizeof(data), data);

    // row address set
    LcdWriteCommandByte(LCD_RASET);
    data[1] = y0 + LCD_YSTART;
    data[3] = y1 + LCD_YSTART;
    LcdWriteData(sizeof(data), data);

    // write to RAM
    LcdWriteCommandByte(LCD_RAMWR);
#elif ILI9341
    // column address set
    LcdWriteCommandByte(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        LcdWriteData(sizeof(data),data);
    }
    // row address set
    LcdWriteCommandByte(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        LcdWriteData(sizeof(data),data);
    }
    // write to RAM
    LcdWriteCommandByte(0x2C); // RAMWR
#elif ST7789V
	// column address set
    x0 += LCD_XSTART;
    x1 += LCD_XSTART; 
    uint8_t data[] = { (x0 >> 8) & 0xFF, (x0 & 0xFF), (x1 >> 8) & 0xFF, (x1 & 0xFF) };
    LcdWriteCommandByte(LCD_CASET);
    LcdWriteData(sizeof(data), data);

    // row address set
    LcdWriteCommandByte(LCD_RASET);
    y0 += LCD_YSTART;
    y1 += LCD_YSTART; 
    uint8_t data1[] = { (y0 >> 8) & 0xFF, (y0 & 0xFF), (y1 >> 8) & 0xFF, (y1 & 0xFF) };
    LcdWriteData(sizeof(data1), data1);

    // write to RAM
    LcdWriteCommandByte(LCD_RAMWR);
#endif
}

void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT))
        return;   
    CS_PIN_WRITE(FALSE);   
    LCD_SetAddressWindow(x, y, x + 1, y + 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    LcdWriteData(sizeof(data), data);
    CS_PIN_WRITE(TRUE);
}

static void LCD_WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor)
{
    uint32_t i, b, j;

    LCD_SetAddressWindow(x, y, x + font.width - 1, y + font.height - 1);

    for (i = 0; i < font.height; i++)
    {
        b = font.data[(ch - 32) * font.height + i];
        for (j = 0; j < font.width; j++)
        {
            if ((b << j) & 0x8000)
            {
                uint8_t data[] = { color >> 8, color & 0xFF };
                LcdWriteData(sizeof(data), data);
            }
            else
            {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                LcdWriteData(sizeof(data), data);
            }
        }
    }
}

void LCD_WriteString(uint16_t x, uint16_t y, const char *str, FontDef font, uint16_t color, uint16_t bgcolor)
{
    CS_PIN_WRITE(FALSE);
    
    while (*str)
    {
        if (x + font.width >= LCD_WIDTH)
        {
            x = 0;
            y += font.height;
            if (y + font.height >= LCD_HEIGHT)
            {
                break;
            }
            if (*str == ' ')
            {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }
        LCD_WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }
    
    CS_PIN_WRITE(TRUE);
}

void LCD_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	uint32_t i;
    // clipping
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH) w = LCD_WIDTH - x;
    if ((y + h - 1) >= LCD_HEIGHT) h = LCD_HEIGHT - y;

//   CS_PIN_WRITE((BitState)FALSE);
    
    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    RS_PIN_WRITE(TRUE); // data
    for(i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++)
    {
          LcdWriteDataByte(data[0]);
          LcdWriteDataByte(data[1]);
    }
}

void LCD_FillRoi(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
		uint32_t i;
    // clipping
    if ((x2 > LCD_WIDTH) || (y2 > LCD_HEIGHT)) return;
    LCD_SetAddressWindow(x1, y1, x2, y2);
    uint8_t data[] = { color >> 8, color & 0xFF };
    RS_PIN_WRITE(TRUE); // data
    for(i = 0; i < ((y2 - y1) * (x2 - x1)); i++)
    {
    //for (uint32_t j = 0; j < 5; j++) {} // GD32SPI
        LcdWriteDataByte(data[0]);
        LcdWriteDataByte(data[1]);
    }
}

void LCD_FillScreen(uint16_t color)
{
    LCD_FillRectangle(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void LCD_DrawImage(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data)
{
    if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) return;
    if ((x + w - 1) >= LCD_WIDTH) return;
    if ((y + h - 1) >= LCD_HEIGHT) return;

    CS_PIN_WRITE(FALSE);
    LCD_SetAddressWindow(x, y, x + w - 1, y + h - 1);
#ifdef USE_DMA
    LcdWriteDataDma((w*h)*2,(uint8_t*)data);
#else
    LcdWriteData(sizeof(uint16_t)*w * h, (uint8_t *)data);
#endif
      
    CS_PIN_WRITE(TRUE);
}

void LCD_InvertColors(bool invert)
{
    CS_PIN_WRITE(FALSE);
#ifdef ST7735R    
    LcdWriteCommandByte(invert ? LCD_INVON : LCD_INVOFF);
#elif ILI9341
    LcdWriteCommandByte(invert ? 0x21 /* INVON */ : 0x20 /* INVOFF */);
#endif 
    CS_PIN_WRITE(TRUE);
}

void LcdTest(void)
{
    LCD_FillScreen(LCD_BLACK);

    for (int x = 0; x < LCD_WIDTH; x++)
    {
        LCD_DrawPixel(x, 0, LCD_RED);
        LCD_DrawPixel(x, LCD_HEIGHT - 1, LCD_RED);
    }

    for (int y = 0; y < LCD_HEIGHT; y++)
    {
        LCD_DrawPixel(0, y, LCD_RED);
        LCD_DrawPixel(LCD_WIDTH - 1, y, LCD_RED);
    }

    board_delay_ms(2000);
    // Check colors
    LCD_FillScreen(LCD_BLACK);
    LCD_WriteString(0, 0, "BLACK", Font_11x18, LCD_WHITE, LCD_BLACK);
    board_delay_ms(500);

    LCD_FillScreen(LCD_BLUE);
    LCD_WriteString(0, 0, "BLUE", Font_11x18, LCD_BLACK, LCD_BLUE);
    board_delay_ms(500);

    LCD_FillScreen(LCD_RED);
    LCD_WriteString(0, 0, "RED", Font_11x18, LCD_BLACK, LCD_RED);
    board_delay_ms(500);

    LCD_FillScreen(LCD_GREEN);
    LCD_WriteString(0, 0, "GREEN", Font_11x18, LCD_BLACK, LCD_GREEN);
    board_delay_ms(500);

    LCD_FillScreen(LCD_CYAN);
    LCD_WriteString(0, 0, "CYAN", Font_11x18, LCD_BLACK, LCD_CYAN);
    board_delay_ms(500);

    LCD_FillScreen(LCD_MAGENTA);
    LCD_WriteString(0, 0, "MAGENTA", Font_11x18, LCD_BLACK, LCD_MAGENTA);
    board_delay_ms(500);

    LCD_FillScreen(LCD_YELLOW);
    LCD_WriteString(0, 0, "YELLOW", Font_11x18, LCD_BLACK, LCD_YELLOW);
    board_delay_ms(500);

    LCD_FillScreen(LCD_WHITE);
    LCD_WriteString(0, 0, "WHITE", Font_11x18, LCD_BLACK, LCD_WHITE);
    board_delay_ms(2000);
    // Check fonts
    LCD_FillScreen(LCD_BLACK);
    LCD_WriteString(0, 0, "Font_7x10, red on black, lorem ipsum dolor sit amet", Font_7x10, LCD_RED, LCD_BLACK);
    LCD_WriteString(0, 3 * 10, "Font_11x18, green, lorem ipsum", Font_11x18, LCD_GREEN, LCD_BLACK);
    LCD_WriteString(0, 3 * 10 + 3 * 18, "Font_16x26", Font_16x26, LCD_BLUE, LCD_BLACK);
    board_delay_ms(2500);
}
/******************************Copyright(C) RCSN*******************************/


