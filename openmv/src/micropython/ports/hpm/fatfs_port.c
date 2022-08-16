#include "py/runtime.h"
#include "lib/oofatfs/ff.h"

DWORD get_fattime(void) {
    // TODO: Implement this function. For now, fake it.
    return ((2016 - 1980) << 25) | ((12) << 21) | ((4) << 16) | ((00) << 11) | ((18) << 5) | (23 / 2);
}
