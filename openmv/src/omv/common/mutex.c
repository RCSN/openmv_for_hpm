/*
 * This file is part of the OpenMV project.
 *
 * Copyright (c) 2013-2021 Ibrahim Abdelkader <iabdalkader@openmv.io>
 * Copyright (c) 2013-2021 Kwabena W. Agyeman <kwagyeman@openmv.io>
 *
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * Mutex implementation.
 * This is a standard implementation of mutexs on ARM processors following the ARM guide.
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dai0321a/BIHEJCHB.html
 *
 * Note: The Cortex-M0/M0+ does Not have the Load/Store exclusive instructions, on these
 * CPUs the locking function is implemented with atomic access using disable/enable IRQs.
 */
#include "mutex.h"
//#include "cmsis_gcc.h"
#include "arm_compat.h"
#include "py/mphal.h"
#include "board.h"
void mutex_init0(omv_mutex_t *mutex)
{
    //__DMB();
    mutex->tid = 0;
    mutex->lock = 0;
    mutex->last_tid = 0;
}

static void _mutex_lock(omv_mutex_t *mutex, uint32_t tid, bool blocking)
{
    #if (__ARM_ARCH < 7)
    do {
        disable_global_irq(CSR_MSTATUS_MIE_MASK);
        if (mutex->lock == 0) {
            mutex->lock = 1;
            mutex->tid = tid;
        }
        enable_global_irq(CSR_MSTATUS_MIE_MASK);
    } while (mutex->tid != tid && blocking);
    #else
    do {
        // Attempt to lock the mutex
        if (__LDREXW(&mutex->lock) == 0) {
            if (__STREXW(1, &mutex->lock) == 0) {
                // Set TID if mutex is locked
                mutex->tid = tid;
            }
        }
    } while (mutex->tid != tid && blocking);
    #endif
    //__DMB();
}

void mutex_lock(omv_mutex_t *mutex, uint32_t tid)
{
    _mutex_lock(mutex, tid, true);
}

int mutex_try_lock(omv_mutex_t *mutex, uint32_t tid)
{
    // If the mutex is already locked by the current thread then
    // release it and return without locking, otherwise try to lock it.
    if (mutex->tid == tid) {
        mutex_unlock(mutex, tid);
    } else {
        _mutex_lock(mutex, tid, false);
    }

    return (mutex->tid == tid);
}
#if 1
int mutex_try_lock_alternate(omv_mutex_t *mutex, uint32_t tid)
{
    if (mutex->last_tid != tid) {
        if (mutex_try_lock(mutex, tid)) {
            mutex->last_tid = tid;
            //printf("mutex_try_lock_alternate:%d \r\n",tid);
            return 1;
        }
    }

    return 0;
}
#else
int mutex_try_lock_alternate(omv_mutex_t *mutex, uint32_t tid)
{
    volatile int locked = 1;
    if(mutex->tid == tid)
    {
        mutex_unlock(mutex,tid);
    }
    else if(mutex->lock == 0)
    {
        locked = mutex->lock;
        mutex->lock = 1;
        if(locked == 0)
        {
            mutex->tid = tid;
        }
    }
    return (locked == 0);
}
#endif
int mutex_lock_timeout(omv_mutex_t *mutex, uint32_t tid, uint32_t timeout)
{
    mp_uint_t tick_start = mp_hal_ticks_ms();
    while ((mp_hal_ticks_ms() - tick_start) >= timeout) {
        if (mutex_try_lock(mutex, tid)) {
            return 1;
        }
        //__WFI();
    }
    return 0;
}

void mutex_unlock(omv_mutex_t *mutex, uint32_t tid)
{
    if (mutex->tid == tid) {
        //__DMB();
        mutex->tid = 0;
        mutex->lock = 0;
    }
}
