#include <stdint.h>

extern const struct _mp_obj_module_t pyb_module;
extern const struct _mp_obj_module_t mp_module_utime;
extern const struct _mp_obj_module_t sensor_module;
extern const struct _mp_obj_module_t image_module;
//#ifndef MICROPY_CONFIG_ROM_LEVEL
//#define MICROPY_CONFIG_ROM_LEVEL            (30)
//#endif
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
#define MICROPY_PY_SYS                          (1)
#define MICROPY_PY_BUILTINS_HELP                (1)
#define MICROPY_PY_BUILTINS_NOTIMPLEMENTED      (0)
#define MICROPY_ENABLE_SCHEDULER                (0)

#define MICROPY_PY_UTIME_MP_HAL                 (1)
 
#define MICROPY_MODULE_WEAK_LINKS               (1)

#define MICROPY_PY_PYB                          (1)

#define MICROPY_EVENT_POLL_HOOK \
    do { \
        extern void mp_handle_pending(bool); \
        mp_handle_pending(true); \
    } while (0);

#if MICROPY_PY_PYB
#define PYB_BUILTIN_MODULE                  { MP_ROM_QSTR(MP_QSTR_pyb), MP_ROM_PTR(&pyb_module) },
#else
#define PYB_BUILTIN_MODULE
#endif

#define MICROPY_PORT_BUILTIN_MODULES \
            {MP_OBJ_NEW_QSTR(MP_QSTR_utime), MP_ROM_PTR(&mp_module_utime)},\
            { MP_OBJ_NEW_QSTR(MP_QSTR_sensor),  (mp_obj_t)&sensor_module }, \
            { MP_OBJ_NEW_QSTR(MP_QSTR_image),  (mp_obj_t)&image_module }, \
            PYB_BUILTIN_MODULE


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