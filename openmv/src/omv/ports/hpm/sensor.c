/*
 * @Descripttion: 
 * @version: 
 * @Author: RCSN
 * @Date: 2022-08-17 11:42:33
 * @LastEditors: fy-tech
 * @LastEditTime: 2022-08-19 14:04:43
 */
/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Sensor abstraction layer.
 */
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mphal.h"
#include "cambus.h"
#include "sensor.h"
#include "framebuffer.h"
#include "omv_boardconfig.h"
#include "unaligned_memcpy.h"

#include "board.h"
#include "hpm_cam_drv.h"

#define USE_CAM_IRQ   (0)
#define TIME_SENSOR   (0)
#if (TIME_SENSOR == 1)
#include "py/mphal.h"
#endif

#define SENSOR_TIMEOUT_MS       (3000)
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))

#if (USE_CAM_IRQ == 0)
static uint8_t __attribute__((section (".framebuffer"))) sensor_buffer[3072 * 1024] __attribute__ ((aligned(__SCB_DCACHE_LINE_SIZE)));
#endif

sensor_t sensor = {};
static cam_config_t cam_config;

static bool first_line = false;
static bool drop_frame = false;

extern uint8_t _line_buf;

#if (USE_CAM_IRQ == 1)

ATTR_RAMFUNC void isr_cam0(void)
{
    if((HPM_CAM0->STA & CAM_STATUS_END_OF_FRAME) == CAM_STATUS_END_OF_FRAME)
  //if((HPM_CAM0->STA & CAM_STATUS_FB1_DMA_TRANSFER_DONE) == CAM_STATUS_FB1_DMA_TRANSFER_DONE)
    {
      //printf("sensor_snapshot : 0x%08x \r\n",HPM_CAM0->STA);
      HPM_CAM0->STA |= CAM_STA_DMA_TSF_DONE_FB1_SET(1);
      framebuffer_get_tail(FB_NO_FLAGS);
      vbuffer_t *buffer = framebuffer_get_tail(FB_PEEK);
      if (buffer != NULL) 
      {
          cam_config.buffer1 = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)buffer->data);
          cam_init(HPM_CAM0, &cam_config);
          cam_start(HPM_CAM0);
      }
    }
}
SDK_DECLARE_EXT_ISR_M(IRQn_CAM1, isr_cam0)

#endif 


static int sensor_dma_config()
{
    // DMA Stream configuration
    return 0;
}
uint32_t sensor_get_xclk_frequency()
{
    return clock_get_frequency(clock_camera0);
}

int sensor_set_xclk_frequency(uint32_t frequency)
{
    if(frequency == (12000000))
    {
      clock_set_source_divider(clock_camera0, clk_src_osc24m, 2U);
    }
    else if(frequency == (24000000))
    {
      clock_set_source_divider(clock_camera0, clk_src_osc24m, 1U);
    }
    else
    {
      clock_set_source_divider(clock_camera0, clk_src_osc24m, 1U);
    }
    return 0;
}

void sensor_init0()
{
    sensor_abort();

    // Re-init cambus to reset the bus state after soft reset, which
    // could have interrupted the bus in the middle of a transfer.
    if (sensor.bus.initialized) {
        // Reinitialize the bus using the last used id and speed.
        cambus_init(&sensor.bus, sensor.bus.id, sensor.bus.speed);
    }

    // Disable VSYNC IRQ and callback
    sensor_set_vsync_callback(NULL);

    // Disable Frame callback.
    sensor_set_frame_callback(NULL);
}

int sensor_init()
{
    int init_ret = 0;

    // List of cambus I2C buses to scan.
    uint32_t buses[][2] = {
        {ISC_I2C_ID, ISC_I2C_SPEED},
        #if defined(ISC_I2C_ALT)
        {ISC_I2C_ALT_ID, ISC_I2C_ALT_SPEED},
        #endif
    };

    // Reset the sesnor state
    memset(&sensor, 0, sizeof(sensor_t));
    cam_config.width = 0;
    cam_config.height = 0;

    // Set default snapshot function.
    // Some sensors need to call snapshot from init.
    sensor.snapshot = sensor_snapshot;

    // Configure the sensor external clock (XCLK).
    if (sensor_set_xclk_frequency(OMV_XCLK_FREQUENCY) != 0) {
        // Failed to initialize the sensor clock.
        return SENSOR_ERROR_TIM_INIT_FAILED;
    }
#if  (USE_CAM_IRQ == 1)
    HPM_CAM0->INT_EN = CAM_INT_EN_EOF_INT_EN_SET(1);
    printf("cam1 int_en:0x%08x",HPM_CAM0->INT_EN);
    intc_m_enable_irq_with_priority(IRQn_CAM1, 8);
    //intc_m_enable_irq(IRQn_CAM1);
#endif
    // Detect and initialize the image sensor.
    for (uint32_t i=0, n_buses = ARRAY_SIZE(buses); i < n_buses; i++) {
        uint32_t id = buses[i][0], speed = buses[i][1];
        if ((init_ret = sensor_probe_init(id, speed)) == 0) {
            // Sensor was detected on the current bus.
            break;
        }
        cambus_deinit(&sensor.bus);
        // Scan the next bus or fail if this is the last one.
        if ((i+1) == n_buses) {
            // Sensor probe/init failed.
            return init_ret;
        }
    }
    
    // Configure the DCMI DMA Stream
    if (sensor_dma_config() != 0) {
        // DMA problem
        return SENSOR_ERROR_DMA_INIT_FAILED;
    }

    // Configure the DCMI interface.
    if (sensor_pixformat(PIXFORMAT_INVALID) != 0){
        // DCMI config failed
        return SENSOR_ERROR_DCMI_INIT_FAILED;
    }

    if (sensor_framesize(cam_config.width,cam_config.height) != 0){
        // DCMI config failed
        return -1;
    }

    // Clear fb_enabled flag
    // This is executed only once to initialize the FB enabled flag.
    // JPEG_FB()->enabled = 0;

    // Set default color palette.
    sensor.color_palette = rainbow_table;

    sensor.detected = true;

    /* All good! */
    return 0;
}

int sensor_pixformat(uint32_t pixformat)
{
    if(pixformat == PIXFORMAT_INVALID)
      return 0;
 //   cam_stop(HPM_CAM0);
    cam_get_default_config(HPM_CAM0, &cam_config, display_pixel_format_rgb565);
    cam_config.hsync_active_low = (sensor.hw_flags.hsync ? 0 : 1);
    cam_config.pixclk_sampling_falling = (sensor.hw_flags.pixck ? 0 : 1);
    cam_config.vsync_active_low = (sensor.hw_flags.vsync ? 0 : 1);
    cam_config.sensor_bitwidth = CAM_SENSOR_BITWIDTH_10BITS;
    cam_config.data_pack_msb = false;
    cam_config.color_ext = false;    
    //MAIN_FB()->pixfmt = pixformat;
    if(pixformat == PIXFORMAT_RGB565)
    {
        cam_config.data_store_mode = CAM_DATA_STORE_MODE_NORMAL;
        cam_config.color_format = CAM_COLOR_FORMAT_RGB565;   
    }
    else if(pixformat == PIXFORMAT_GRAYSCALE)
    {
        cam_config.data_store_mode = CAM_DATA_STORE_MODE_Y_ONLY;
        cam_config.color_format = CAM_COLOR_FORMAT_YCBCR422;
    }
    else
    {
        cam_config.data_store_mode = CAM_DATA_STORE_MODE_NORMAL;
        cam_config.color_format = CAM_COLOR_FORMAT_RGB565;   
    }
    printf("sensor_pixformat:%d %d %d\r\n",pixformat, cam_config.data_store_mode,cam_config.color_format);
 //   cam_start(HPM_CAM0);
    return 0;
}

int sensor_framesize(int32_t w,int32_t h)
{
//    cam_stop(HPM_CAM0);
    if(w == 0 || h == 0)
      return 0;
    cam_config.width = w;
    cam_config.height = h;
#if (USE_CAM_IRQ == 0)
    cam_config.buffer1 = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)sensor_buffer);
    cam_init(HPM_CAM0, &cam_config);
    cam_start(HPM_CAM0);
#endif
    printf("sensor_framesize:%d %d\r\n", cam_config.data_store_mode,cam_config.color_format);
    return 0;
}

int sensor_dcmi_config(uint32_t pixformat)
{
    return 0;
}

int sensor_abort()
{
    return 0;
}


int sensor_shutdown(int enable)
{
    int ret = 0;
    return ret;
}

int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;

    return 0;
}


// This is the default snapshot function, which can be replaced in sensor_init functions. This function
// uses the DCMI and DMA to capture frames and each line is processed in the DCMI_DMAConvCpltUser function.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    framebuffer_update_jpeg_buffer();

#if (USE_CAM_IRQ == 0)
        // This driver supports a single buffer.
    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }
#endif

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }
     // Free the current FB head.
    framebuffer_free_current_buffer();

    // Set framebuffer pixel format.
    if (sensor->pixformat == PIXFORMAT_INVALID) {
        return SENSOR_ERROR_INVALID_PIXFORMAT;
    }

    MAIN_FB()->pixfmt = sensor->pixformat;

#if (USE_CAM_IRQ == 0)
    vbuffer_t *buffer = framebuffer_get_tail(FB_NO_FLAGS);

     if (!buffer) {
        return SENSOR_ERROR_FRAMEBUFFER_ERROR;
    }

    mp_uint_t ticks = mp_hal_ticks_ms();
    while(1)
    {
      if((mp_hal_ticks_ms() - ticks) > 3000)
      {
          return SENSOR_ERROR_CAPTURE_TIMEOUT;
      }
      if((HPM_CAM0->STA & CAM_STATUS_END_OF_FRAME) == CAM_STATUS_END_OF_FRAME)
      //if((HPM_CAM0->STA & CAM_STATUS_FB1_DMA_TRANSFER_DONE) == CAM_STATUS_FB1_DMA_TRANSFER_DONE)
      {
        //printf("sensor_snapshot : 0x%08x \r\n",HPM_CAM0->STA);
        HPM_CAM0->STA |= CAM_STA_DMA_TSF_DONE_FB1_SET(1);
        break;
      }
    }


    // Not useful for the NRF but must call to keep API the same.
    if (sensor->frame_callback) {
        sensor->frame_callback();
    }
    MAIN_FB()->pixfmt = sensor->pixformat;
    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;
    
#if (TIME_SENSOR==1)
    mp_uint_t start = mp_hal_ticks_ms();
#endif

#if 1
    if((MAIN_FB()->x == 0) && (MAIN_FB()->y == 0) && (MAIN_FB()->w == cam_config.width) && (MAIN_FB()->h == cam_config.height))
    {
      memcpy(buffer->data,sensor_buffer,MAIN_FB()->u * MAIN_FB()->v * MAIN_FB()->bpp);
    }
    else
    {
      int k = 0;
      for(int i = MAIN_FB()->x;i < (MAIN_FB()->x + MAIN_FB()->h);i++)
      {
        for(int j = MAIN_FB()->y;j < (MAIN_FB()->y + MAIN_FB()->w);j++)
        {
          *(uint16_t *)&buffer->data[k] = *(uint16_t *)&sensor_buffer[(cam_config.width * 2 * i - 2) + (j + 1) * 2];
          k += 2;
        }
      }  
    }
    
#else
    extern inline void usbdbg_set_irq_enabled(bool enabled);
//    l1c_dc_flush(buffer->data,MAIN_FB()->u * MAIN_FB()->v * MAIN_FB()->bpp);
    //usbdbg_set_irq_enabled(false);
    for(int i = 0;i < (MAIN_FB()->u * MAIN_FB()->v * MAIN_FB()->bpp);i++)
    {
        buffer->data[i] = sensor_buffer[i];
    }
    //usbdbg_set_irq_enabled(true);
#endif
 
#else
    vbuffer_t *buffer = framebuffer_get_head(FB_NO_FLAGS);

    // If there's no ready buffer in the fifo, and the DMA is Not currently
    // transferring a new buffer, reconfigure and restart the DMA transfer.
    if (buffer == NULL) {
        buffer = framebuffer_get_tail(FB_PEEK);
        if (buffer == NULL) {
            return SENSOR_ERROR_FRAMEBUFFER_ERROR;
        }
        cam_config.buffer1 = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)buffer->data);
        cam_init(HPM_CAM0, &cam_config);
        cam_start(HPM_CAM0);
    }

        // Wait for the DMA to finish the transfer.
    for (mp_uint_t ticks = mp_hal_ticks_ms(); buffer == NULL;) {
        buffer = framebuffer_get_head(FB_NO_FLAGS);
        if ((mp_hal_ticks_ms() - ticks) > 3000) {
            sensor_abort();
            return SENSOR_ERROR_CAPTURE_TIMEOUT;
        }
    }

#endif


    MAIN_FB()->w = MAIN_FB()->u;
    MAIN_FB()->h = MAIN_FB()->v;

#if (TIME_SENSOR==1)
    printf("time: %u ms\n", mp_hal_ticks_ms() - start);
#endif

    if ((MAIN_FB()->pixfmt == PIXFORMAT_RGB565 && sensor->hw_flags.rgb_swap) ||
        (MAIN_FB()->pixfmt == PIXFORMAT_YUV422 && sensor->hw_flags.yuv_swap)) {
        unaligned_memcpy_rev16(buffer->data, buffer->data, MAIN_FB()->w * MAIN_FB()->h);
    }
   ;
    framebuffer_init_image(image);

    return 0;
}
