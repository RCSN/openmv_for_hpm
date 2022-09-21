/*
 * @Descripttion: 
 * @version: 
 * @Author: RCSN
 * @Date: 2022-09-21 16:38:16
 * @LastEditors:
 * @LastEditTime: 2022-09-21 16:43:24
 */
#define MICROPY_HW_BOARD_NAME       "HPM6750EVKMINI"
#define MICROPY_HW_MCU_NAME         "HPM6750IVM1"

#define MICROPY_HW_ENABLE_RTC       (0)
#define MICROPY_HW_ENABLE_RNG       (1)
#define MICROPY_HW_ENABLE_DAC       (0)
#define MICROPY_HW_ENABLE_SERVO     (0) // SERVO requires TIM5 (not on L452).
#define MICROPY_HW_HAS_SWITCH       (0)