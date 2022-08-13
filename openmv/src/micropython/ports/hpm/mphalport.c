#include <string.h>

#include "py/mpstate.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "hpm_common.h"
#include "hpm_clock_drv.h"

bool mp_hal_ticks_cpu_enabled = false;

// this table converts from HAL_StatusTypeDef to POSIX errno
const byte mp_hal_status_to_errno_table[4] = {
    [HAL_OK] = 0,
    [HAL_ERROR] = MP_EIO,
    [HAL_BUSY] = MP_EBUSY,
    [HAL_TIMEOUT] = MP_ETIMEDOUT,
};

NORETURN void mp_hal_raise(HAL_StatusTypeDef status) {
    mp_raise_OSError(mp_hal_status_to_errno_table[status]);
}
ATTR_WEAK void usbdbg_try_run_script(void) {}
extern void mp_handle_pending(bool raise_exc); 
extern uint32_t recv_cdc_data(uint8_t *data);
extern void send_cdc_data(uint8_t *data,uint32_t len);
extern bool usb_vcp_is_enabled(void);
void EventPollHook(void) {
    mp_handle_pending(true); 
//    usbdbg_try_run_script();
#if MICROPY_PY_THREAD
    if (pyb_thread_enabled) { 
        MP_THREAD_GIL_EXIT(); 
        pyb_thread_yield(); 
        MP_THREAD_GIL_ENTER(); 
    } 
#endif
//    HAL_WFI();
}

int mp_hal_stdin_rx_chr(void) {
	int c;
	for (;;) {
	#ifdef USE_HOST_MODE
		pyb_usb_host_process();
		int c = pyb_usb_host_get_keyboard();
		if (c != 0) {
			return c;
		}
	#endif
       if (recv_cdc_data((uint8_t*)&c) != 0) {
           return (uint8_t)(c & 0xFF);
       }
       //else if (MP_STATE_PORT(pyb_stdio_uart) != NULL && uart_rx_any(MP_STATE_PORT(pyb_stdio_uart))) {
       //    return uart_rx_char(MP_STATE_PORT(pyb_stdio_uart));
       //}
//       MICROPY_EVENT_POLL_HOOK
   }
 return 0;
}

void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}

void mp_hal_stdout_tx_strn(const char *str, size_t len) {
	//if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
	//	uart_tx_strn(MP_STATE_PORT(pyb_stdio_uart), str, len);
	//}
#if 0 && defined(USE_HOST_MODE) && MICROPY_HW_HAS_LCD
	lcd_print_strn(str, len);
#endif
    
	if (usb_vcp_is_enabled()) {
	//	// rocky: if send through VCP, on windows, MUST open the port,
	//	// otherwise, a buffer on windows will finally overrun, and host 
	//	// will no longer accept data from us!
      send_cdc_data(str,len);
	}
#if MICROPY_HW_WIFIDBG_EN
	wifidbg_send_strn(str, len);
#endif	

}

void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    // send stdout to UART and USB CDC VCP
    //if (MP_STATE_PORT(pyb_stdio_uart) != NULL) {
    //    uart_tx_strn_cooked(MP_STATE_PORT(pyb_stdio_uart), str, len);
    //}
    if (usb_vcp_is_enabled()) {
		//// rocky: if send through VCP, on windows, MUST open the port,
		//// otherwise, a buffer on windows will finally overrun, and host 
		//// will no longer accept data from us!		
        send_cdc_data(str,len);
    } else {

    }
	
	#if MICROPY_HW_WIFIDBG_EN
	wifidbg_send_strn(str, len);
	#endif	

}

uint32_t HAL_GetHalVersion(void)
{
 return 1;
}


void mp_hal_delay_ms(mp_uint_t ms)
{
  clock_cpu_delay_ms(ms);
}

void mp_hal_delay_us(mp_uint_t us)
{
  clock_cpu_delay_us(us);
}


mp_uint_t mp_hal_ticks_ms(void)
{
    return  0;
}

mp_uint_t mp_hal_ticks_us(void)
{
    return  0;
}

// Nanoseconds since the Epoch.
uint64_t mp_hal_time_ns(void)
{
    return  0;
}


void mp_hal_ticks_cpu_enable(void) {
    if (!mp_hal_ticks_cpu_enabled) {
        //CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        //#if defined(__CORTEX_M) && __CORTEX_M == 7
        //// on Cortex-M7 we must unlock the DWT before writing to its registers
        //DWT->LAR = 0xc5acce55;
        //#endif
        //DWT->CYCCNT = 0;
        //DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        //mp_hal_ticks_cpu_enabled = true;
    }
}

