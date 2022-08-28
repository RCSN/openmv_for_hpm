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

#define MDMA_BUFFER_SIZE        (64)
#define DMA_MAX_XFER_SIZE       (0xFFFF*4)
#define DMA_MAX_XFER_SIZE_DBL   ((DMA_MAX_XFER_SIZE)*2)
#define DMA_LENGTH_ALIGNMENT    (16)
#define SENSOR_TIMEOUT_MS       (3000)
#define ARRAY_SIZE(a)           (sizeof(a) / sizeof((a)[0]))
static uint8_t __attribute__((section (".framebuffer"))) sensor_buffer[1024 * 1024] __attribute__ ((aligned(16)));
sensor_t sensor = {};
static cam_config_t cam_config;

#if (OMV_ENABLE_SENSOR_MDMA == 1)
static MDMA_HandleTypeDef DCMI_MDMA_Handle0 = {.Instance = MDMA_Channel0};
static MDMA_HandleTypeDef DCMI_MDMA_Handle1 = {.Instance = MDMA_Channel1};
#endif
// SPI on image sensor connector.
#ifdef ISC_SPI
SPI_HandleTypeDef ISC_SPIHandle = {.Instance = ISC_SPI};
#endif // ISC_SPI

static bool first_line = false;
static bool drop_frame = false;

extern uint8_t _line_buf;

void DCMI_IRQHandler(void) {
    //HAL_DCMI_IRQHandler(&DCMIHandle);
}

void DMA2_Stream1_IRQHandler(void) {
    //HAL_DMA_IRQHandler(DCMIHandle.DMA_Handle);
}

#ifdef ISC_SPI
void ISC_SPI_IRQHandler(void)
{
    HAL_SPI_IRQHandler(&ISC_SPIHandle);
}

void ISC_SPI_DMA_IRQHandler(void)
{
    HAL_DMA_IRQHandler(ISC_SPIHandle.hdmarx);
}
#endif // ISC_SPI

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
    cam_config_t _config;
    if(pixformat == PIXFORMAT_INVALID)
      return 0;
 //   cam_stop(HPM_CAM0);
    cam_get_default_config(HPM_CAM0, &_config, display_pixel_format_rgb565);
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
    cam_config.buffer1 = core_local_mem_to_sys_address(HPM_CORE0, (uint32_t)sensor_buffer);
    cam_init(HPM_CAM0, &cam_config);
    cam_start(HPM_CAM0);
    printf("sensor_framesize:%d %d\r\n", cam_config.data_store_mode,cam_config.color_format);
    return 0;
}

int sensor_dcmi_config(uint32_t pixformat)
{
    return 0;
}

int sensor_abort()
{
    // This stops the DCMI hardware from generating DMA requests immediately and then stops the DMA
    // hardware. Note that HAL_DMA_Abort is a blocking operation. Do not use this in an interrupt.
    //if (DCMI->CR & DCMI_CR_ENABLE) {
    //    DCMI->CR &= ~DCMI_CR_ENABLE;
    //    HAL_DMA_Abort(&DMAHandle);
    //    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
    //    #if (OMV_ENABLE_SENSOR_MDMA == 1)
    //    HAL_MDMA_Abort(&DCMI_MDMA_Handle0);
    //    HAL_MDMA_Abort(&DCMI_MDMA_Handle1);
    //    HAL_MDMA_DeInit(&DCMI_MDMA_Handle0);
    //    HAL_MDMA_DeInit(&DCMI_MDMA_Handle1);
    //    #endif
    //    __HAL_DCMI_DISABLE_IT(&DCMIHandle, DCMI_IT_FRAME);
    //    __HAL_DCMI_CLEAR_FLAG(&DCMIHandle, DCMI_FLAG_FRAMERI);
    //    first_line = false;
    //    drop_frame = false;
    //    sensor.last_frame_ms = 0;
    //    sensor.last_frame_ms_valid = false;
    //}

    //framebuffer_reset_buffers();

    return 0;
}


int sensor_shutdown(int enable)
{
    int ret = 0;
    //sensor_abort();

    //if (enable) {
    //    if (sensor.pwdn_pol == ACTIVE_HIGH) {
    //        DCMI_PWDN_HIGH();
    //    } else {
    //        DCMI_PWDN_LOW();
    //    }
    //    HAL_NVIC_DisableIRQ(DCMI_IRQn);
    //    HAL_DCMI_DeInit(&DCMIHandle);
    //} else {
    //    if (sensor.pwdn_pol == ACTIVE_HIGH) {
    //        DCMI_PWDN_LOW();
    //    } else {
    //        DCMI_PWDN_HIGH();
    //    }
    //    ret = sensor_dcmi_config(sensor.pixformat);
    //}

    //mp_hal_delay_ms(10);
    return ret;
}

int sensor_set_vsync_callback(vsync_cb_t vsync_cb)
{
    sensor.vsync_callback = vsync_cb;
    //if (sensor.vsync_callback == NULL) {
    //    // Disable VSYNC EXTI IRQ
    //    HAL_NVIC_DisableIRQ(DCMI_VSYNC_IRQN);
    //} else {
    //    // Enable VSYNC EXTI IRQ
    //    NVIC_SetPriority(DCMI_VSYNC_IRQN, IRQ_PRI_EXTINT);
    //    HAL_NVIC_EnableIRQ(DCMI_VSYNC_IRQN);
    //}
    return 0;
}

void DCMI_VsyncExtiCallback()
{
    //__HAL_GPIO_EXTI_CLEAR_FLAG(1 << DCMI_VSYNC_IRQ_LINE);
    //if (sensor.vsync_callback != NULL) {
    //    sensor.vsync_callback(HAL_GPIO_ReadPin(DCMI_VSYNC_PORT, DCMI_VSYNC_PIN));
    //}
}

// If we are cropping the image by more than 1 word in width we can align the line start to
// a word address to improve copy performance. Do not crop by more than 1 word as this will
// result in less time between DMA transfers complete interrupts on 16-byte boundaries.
static uint32_t get_dcmi_hw_crop(uint32_t bytes_per_pixel)
{
    uint32_t byte_x_offset = (MAIN_FB()->x * bytes_per_pixel) % sizeof(uint32_t);
    uint32_t width_remainder = (resolution[sensor.framesize][0] - (MAIN_FB()->x + MAIN_FB()->u)) * bytes_per_pixel;
    uint32_t x_crop = 0;

    if (byte_x_offset && (width_remainder >= (sizeof(uint32_t) - byte_x_offset))) {
        x_crop = byte_x_offset;
    }

    return x_crop;
}


// This is the default snapshot function, which can be replaced in sensor_init functions. This function
// uses the DCMI and DMA to capture frames and each line is processed in the DCMI_DMAConvCpltUser function.
int sensor_snapshot(sensor_t *sensor, image_t *image, uint32_t flags)
{
    framebuffer_update_jpeg_buffer();

        // This driver supports a single buffer.
    if (MAIN_FB()->n_buffers != 1) {
        framebuffer_set_buffers(1);
    }

    if (sensor_check_framebuffer_size() != 0) {
        return SENSOR_ERROR_FRAMEBUFFER_OVERFLOW;
    }
     // Free the current FB head.
    framebuffer_free_current_buffer();

    // Set framebuffer pixel format.
    if (sensor->pixformat == PIXFORMAT_INVALID) {
        return SENSOR_ERROR_INVALID_PIXFORMAT;
    }
    

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
    
    
//    memcpy(buffer->data,sensor_buffer,MAIN_FB()->u * MAIN_FB()->v * MAIN_FB()->bpp);
    for(int i = 0;i < (MAIN_FB()->w * MAIN_FB()->h * MAIN_FB()->bpp);i++)
    {
        buffer->data[i] = sensor_buffer[i];
    }
    if ((MAIN_FB()->pixfmt == PIXFORMAT_RGB565 && sensor->hw_flags.rgb_swap) ||
        (MAIN_FB()->pixfmt == PIXFORMAT_YUV422 && sensor->hw_flags.yuv_swap)) {
        unaligned_memcpy_rev16(buffer->data, buffer->data, MAIN_FB()->w * MAIN_FB()->h);
    }
   ;
    framebuffer_init_image(image);

    return 0;
}
