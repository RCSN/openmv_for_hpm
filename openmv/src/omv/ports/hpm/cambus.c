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

#define I2C_TIMEOUT             (100*1000)
#define I2C_SCAN_TIMEOUT        (1*1000)

int cambus_init(cambus_t *bus, uint32_t bus_id, uint32_t speed)
{
    return 0;
}

int cambus_deinit(cambus_t *bus)
{

    return 0;
}

int cambus_scan(cambus_t *bus)
{

    return 0;
}

int cambus_enable(cambus_t *bus, bool enable)
{
    return 0;
}

int cambus_readb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr,  uint8_t *reg_data)
{
    int bytes = 0;
    return (bytes == 2) ? 0 : -1;
}

int cambus_writeb(cambus_t *bus, uint8_t slv_addr, uint8_t reg_addr, uint8_t reg_data)
{
    int bytes = 0;
    return (bytes == 2) ? 0 : -1;
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
