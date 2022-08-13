#ifndef HPM_LED_PIN_H_
#define HPM_LED_PIN_H_

#include "board.h"
#include "hpm_gpio_drv.h"

#define MICROPY_HW_LED1             (19) // red
#define MICROPY_HW_LED2             (18) // green
#define MICROPY_HW_LED3             (20) // blue

#define MICROPY_HW_LED_ON(pin)      gpio_write_pin(BOARD_R_GPIO_CTRL,BOARD_R_GPIO_INDEX,pin,1)
#define MICROPY_HW_LED_OFF(pin)     gpio_write_pin(BOARD_R_GPIO_CTRL,BOARD_R_GPIO_INDEX,pin,0)
#define MICROPY_HW_LED_TOGGLE(pin)  gpio_toggle_pin(BOARD_R_GPIO_CTRL,BOARD_R_GPIO_INDEX,pin)
#define MICROPY_HW_LED_READ(pin)    gpio_read_pin(BOARD_R_GPIO_CTRL,BOARD_R_GPIO_INDEX,pin)
#endif
