#ifndef  __DEV_PORT_CONFIG_H_
#define  __DEV_PORT_CONFIG_H_

#include "hpm_usb_device.h"
#include "board.h"
#include "hpm_l1c_drv.h"
#include "hpm_ppor_drv.h"
//#define __DCACHE_PRESENT

#ifdef __DCACHE_PRESENT
#define __SCB_DCACHE_LINE_SIZE  64
#else
#define __SCB_DCACHE_LINE_SIZE  16
#endif
static inline void nvic_system_reset(void)
{
  ppor_sw_reset(HPM_PPOR, 10);
}


#endif
