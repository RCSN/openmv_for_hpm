#include <stdint.h>
// Python internal features.
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_HELPER_REPL                     (1)
#define MICROPY_ERROR_REPORTING                 (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_FLOAT_IMPL                      (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_GCREGS_SETJMP                   (1)
// emitters
#define MICROPY_PERSISTENT_CODE_LOAD (1)
// Fine control over Python builtins, classes, modules, etc.
#define MICROPY_KBD_EXCEPTION                   (1)
#define MICROPY_USE_INTERNAL_ERRNO              (1)
#define MICROPY_PY_ASYNC_AWAIT                  (0)
#define MICROPY_PY_BUILTINS_SET                 (0)
#define MICROPY_PY_ATTRTUPLE                    (0)
#define MICROPY_PY_COLLECTIONS                  (0)
#define MICROPY_PY_MATH                         (0)
#define MICROPY_PY_IO                           (0)
#define MICROPY_PY_STRUCT                       (1)
#define MICROPY_PY_BUILTINS_EVAL_EXEC           (1)

// Type definitions for the specific machine.
typedef int ssize_t;
typedef intptr_t mp_int_t; // must be pointer size
typedef uintptr_t mp_uint_t; // must be pointer size
typedef long mp_off_t;

// We need to provide a declaration/definition of alloca().
//#include <alloca.h>

void* rollback_alloca(uint32_t size);
#define alloca rollback_alloca

// Define the port's name and hardware.
#define MICROPY_HW_BOARD_NAME "hpm6750evkmini"
#define MICROPY_HW_MCU_NAME   "hpm6750"

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8];