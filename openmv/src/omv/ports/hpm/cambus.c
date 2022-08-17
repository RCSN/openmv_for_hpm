/*
 * @Descripttion: 
 * @version: 
 * @Author: RCSN
 * @Date: 2022-08-16 17:01:27
 * @LastEditors: fy-tech
 * @LastEditTime: 2022-08-16 17:01:27
 */
#include <stdio.h>
#include <stdbool.h>
#include "py/mphal.h"

#include "omv_boardconfig.h"
#include "cambus.h"

#include "board.h"

#define CAM_I2C     BOARD_CAM_I2C_BASE

#define I2C_TIMEOUT             (100*1000)
#define I2C_SCAN_TIMEOUT        (1*1000)
#define I2C_GPIO_CTRL           HPM_GPIO0

int cambus_init(cambus_t *bus, uint32_t bus_id, uint32_t speed)
{
    board_init_i2c(CAM_I2C);
    bus->id = bus_id;
    bus->speed = speed;
    bus->i2c = NULL;
    bus->initialized = false;

    bus->i2c = CAM_I2C;
    bus->scl_pin = (omv_gpio_t) {11, GPIO_DO_GPIOB,I2C_GPIO_CTRL};
    bus->scl_pin = (omv_gpio_t) {10, GPIO_DI_GPIOB,I2C_GPIO_CTRL};
    bus->initialized = true;
    return 0;
}

int cambus_deinit(cambus_t *bus)
{
    bus->i2c = NULL;
    bus->scl_pin = (omv_gpio_t) {0, 0, 0};
    bus->sda_pin = (omv_gpio_t) {0, 0, 0};
    bus->initialized = false;
    return 0;
}

int cambus_scan(cambus_t *bus)
{
   int idx = 0;
    for (uint8_t addr=0x20, rxdata; addr<=0x77; addr++) {
        if (i2c_master_read(bus->i2c, addr, &rxdata, 1) == status_success) {
            return addr;
        }
    }
    return idx;
}

int cambus_enable(cambus_t *bus, bool enable)
{
    return 0;
}

int cambus_gencall(cambus_t *bus, uint8_t cmd)
{
    uint8_t reg_data = 0;
    if (i2c_master_address_write(bus->i2c, 0x00, (uint8_t*)&cmd, 1, (uint8_t*)&reg_data, 1) != status_success) {
        return -1;
    }
    return 0;
}

int cambus_readb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data)
{
    int ret = 0;
    if(i2c_master_address_read(bus->i2c, (uint16_t)slv_addr, (uint8_t*)&reg_addr, 1, reg_data, 1) != status_success)
      ret = -1;
    return ret;
}

int cambus_writeb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int ret = 0;
    if(i2c_master_address_write(bus->i2c,(uint16_t)slv_addr,  (uint8_t*)&reg_addr, 1, (uint8_t*)&reg_data, 1) != status_success)
      ret = -1;
    return ret;
}

int cambus_readb2(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t *reg_data)
{
    int ret = 0;
    if(i2c_master_address_read(bus->i2c, (uint16_t)slv_addr, (uint8_t*)&reg_addr, 2, reg_data, 1) != status_success)
      ret = -1;
    return ret;

}

int cambus_writeb2(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint8_t reg_data)
{
    int ret = 0;
    if(i2c_master_address_write(bus->i2c,(uint16_t)slv_addr,  (uint8_t*)&reg_addr, 2, (uint8_t*)&reg_data, 1) != status_success)
      ret = -1;
    return ret;
}

int cambus_readw(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint16_t *reg_data)
{
    int ret = 0;
    if(i2c_master_address_read(bus->i2c, (uint16_t)slv_addr, (uint8_t*)&reg_addr, 1, reg_data, 2) != status_success)
      ret = -1;
    *reg_data = (*reg_data >> 8) | (*reg_data << 8);
    return ret;

}

int cambus_writew(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint16_t reg_data)
{
    int ret = 0;
    reg_data = (reg_data >> 8) | (reg_data << 8);
    if(i2c_master_address_write(bus->i2c,(uint16_t)slv_addr,  (uint8_t*)&reg_addr, 1, (uint8_t*)&reg_data, 2) != status_success)
      ret = -1;
    return ret;

}

int cambus_readw2(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint16_t *reg_data)
{
    int ret = 0;
    if(i2c_master_address_read(bus->i2c, (uint16_t)slv_addr, (uint8_t*)&reg_addr, 2, reg_data, 2) != status_success)
      ret = -1;
    *reg_data = (*reg_data >> 8) | (*reg_data << 8);
    return ret;
}

int cambus_writew2(cambus_t *bus, uint8_t slv_addr, uint16_t reg_addr, uint16_t reg_data)
{
    int ret = 0;
    reg_data = (reg_data >> 8) | (reg_data << 8);
    if(i2c_master_address_write(bus->i2c,(uint16_t)slv_addr,  (uint8_t*)&reg_addr, 2, (uint8_t*)&reg_data, 2) != status_success)
      ret = -1;
    return ret;
}

int cambus_read_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int bytes = 0;

    return (bytes == len) ? 0 : -1;
}

int cambus_write_bytes(cambus_t *bus, uint8_t slv_addr, uint8_t *buf, int len, uint32_t flags)
{
    int bytes = 0;

    return (bytes == len) ? 0 : -1;
}

int cambus_pulse_scl(cambus_t *bus)
{
    return 0;
}
