#ifndef  __DEV_PORT_CONFIG_H_
#define  __DEV_PORT_CONFIG_H_

#include "hpm_usb_device.h"
#include "board.h"
#include "hpm_l1c_drv.h"
#include "hpm_ppor_drv.h"

static inline void nvic_system_reset(void)
{
  ppor_sw_reset(HPM_PPOR, 10);
}


#endif
