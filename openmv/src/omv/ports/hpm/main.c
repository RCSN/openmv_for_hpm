#include "board.h"
#include "py/compile.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "py/stackctrl.h"
#include "shared/runtime/pyexec.h" 
//#include "lib/utils/pyexec.h"
extern void send_cdc_data(uint8_t *data,uint32_t len);
extern uint32_t recv_cdc_data(uint8_t *data);
extern bool usb_vcp_is_enabled(void);

//HPM_PRO_BASE=D:/vmware/share-dir/openmv_for_hpmo/hpm_sdk
//HPM_PRO_BASE=C:/Users/RCSN/Desktop/hpm6750evkmini/opemv_for_hpm/hpm_sdk
static ATTR_PLACE_AT_NONCACHEABLE  uint8_t heap[4096];
int main(void)
{
  uint32_t recv_len = 0;
  
  board_init();
  l1c_dc_disable();
  board_init_led_pins();
  board_init_usb_pins();
  extern void cdc_acm_msc_init(void);  
  cdc_acm_msc_init();
  mp_stack_ctrl_init();
  gc_init(heap, heap + sizeof(heap));
  mp_init();
  mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
  mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);

  ////// Start a normal REPL; will exit when ctrl-D is entered on a blank line.
again_repl:
  if(usb_vcp_is_enabled())
  {
    board_delay_ms(2000);
    pyexec_friendly_repl();
  }
  else
  {
    goto again_repl;
  }
 
  ////// Deinitialise the runtime.
  gc_sweep_all();
  mp_deinit();
  while(1)
  {
    
    //recv_len = recv_cdc_data(heap);
    //if(recv_len)
    //{
    //    send_cdc_data(heap,recv_len);
    //}
  }
}
// Handle uncaught exceptions (should never be reached in a correct C implementation).
void nlr_jump_fail(void *val) {
    for (;;) {
    }
}

// Do a garbage collection cycle.
void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    gc_collect_end();
}

// There is no filesystem so stat'ing returns nothing.
mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

// There is no filesystem so opening a file raises an exception.
mp_lexer_t *mp_lexer_new_from_file(const char *filename) {
    mp_raise_OSError(MP_ENOENT);
}

#define ALLOCA_BUF_SIZE	2048
uint32_t s_allocaBuf[ALLOCA_BUF_SIZE / 4];
uint32_t s_allocaNdx;
void* rollback_alloca(uint32_t cb) 
{
	void *pRet;
	if (s_allocaNdx * 4 + cb >= ALLOCA_BUF_SIZE)
		s_allocaNdx = 0;
	pRet = (void*)(s_allocaBuf + s_allocaNdx);
	s_allocaNdx += (cb + 3) >> 2;
	return pRet;
}
