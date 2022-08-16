/*
 * @Descripttion: 
 * @version: 
 * @Author: RCSN
 * @Date: 2022-08-16 17:01:38
 * @LastEditors: fy-tech
 * @LastEditTime: 2022-08-16 17:01:38
 */
/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * OpenMV RP2 port abstraction layer.
 */
#ifndef __OMV_PORTCONFIG_H__
#define __OMV_PORTCONFIG_H__

#include "hpm_i2c_drv.h"

// omv_gpio_t definition
typedef uint32_t omv_gpio_t;

// cambus/i2c definition
typedef i2c_config_t *omv_cambus_t;

#endif // __OMV_PORTCONFIG_H__
