/*
 * @Descripttion: 
 * @version: 
 * @Author: RCSN
 * @Date: 2022-09-21 16:44:47
 * @LastEditors: fy-tech
 * @LastEditTime: 2022-09-21 16:44:48
 */
/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2018 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "rng.h"

#if MICROPY_HW_ENABLE_RNG

#if MICROPY_HW_MCU_VERSION
#include "hpm_rng_drv.h"
#include "board.h"

#else

#include "hpm_rtc_drv.h"
#include "riscv/riscv_core.h"
#include "hpm_soc.h"

#endif

static uint8_t flag = 1;
uint32_t __attribute__((section (".noncacheable"))) rand_val;
static void hpm_rng_init();

#define RNG_TIMEOUT_MS (10)

static void hpm_rng_init()
{
#if MICROPY_HW_MCU_VERSION
  if(flag)
  {
    flag = 0;
    rng_init(HPM_RNG);
  }
#else
  struct tm cfg_date_time;
  cfg_date_time.tm_year = 121; // Year: 2021 (started from 1900)
  cfg_date_time.tm_mon = 6;    // Month: July (Started from 0)
  cfg_date_time.tm_mday = 12;  // Day in Month: 12
  cfg_date_time.tm_hour = 16;  // Hour: 16
  cfg_date_time.tm_min = 45;   // Minute: 45
  cfg_date_time.tm_sec = 41;   // Second: 41
  time_t cfg_time = mktime(&cfg_date_time); // Convert date time to tick
  rtc_config_time(HPM_RTC, cfg_time);
#endif
}

#if !MICROPY_HW_MCU_VERSION
static uint32_t pyb_rng_yasmarang(void) {
    static bool seeded = false;
    static uint32_t pad = 0, n = 0, d = 0;
    static uint8_t dat = 0;

    if (!seeded) {
        seeded = true;
        hpm_rng_init();
        pad = read_csr(CSR_CYCLE);
        n = HPM_RTC->SECOND;
        d = HPM_RTC->SUBSEC;
    }

    pad += dat + d * n;
    pad = (pad << 3) + (pad >> 29);
    n = pad | 2;
    d ^= (pad << 31) + (pad >> 1);
    dat ^= (char)pad ^ (d >> 8) ^ 1;

    return pad ^ (d << 5) ^ (pad >> 18) ^ (dat << 1);
}
#endif

uint32_t rng_randint(uint32_t min, uint32_t max) {
    if (min==max) {
        return 0;
    }
#if MICROPY_HW_MCU_VERSION
    hpm_rng_init();
    rng_rand_wait(HPM_RNG, &rand_val, sizeof(rand_val));
    return (rand_val%(max-min))+min;
#else
    return (pyb_rng_yasmarang()%(max-min))+min;
#endif
}

STATIC mp_obj_t pyb_rng_randint(mp_obj_t min, mp_obj_t max) {
    return mp_obj_new_int(rng_randint(mp_obj_get_int(min), mp_obj_get_int(max)));
}
MP_DEFINE_CONST_FUN_OBJ_2(pyb_rng_randint_obj, pyb_rng_randint);

uint32_t rng_get(void) {
#if MICROPY_HW_MCU_VERSION
   hpm_rng_init();
    // Get and return the new random number
    rng_rand_wait(HPM_RNG, &rand_val, sizeof(rand_val));
    return rand_val;
#else
    return pyb_rng_yasmarang();
#endif 
}

// Return a 30-bit hardware generated random number.
STATIC mp_obj_t pyb_rng_get(void) {
    return mp_obj_new_int(rng_get() >> 2);
}
MP_DEFINE_CONST_FUN_OBJ_0(pyb_rng_get_obj, pyb_rng_get);

#else // MICROPY_HW_ENABLE_RNG

#include "hpm_rtc_drv.h"
#include "riscv/riscv_core.h"
#include "hpm_soc.h"

static void hpm_rtc_init(void)
{
    struct tm cfg_date_time;
    cfg_date_time.tm_year = 121; // Year: 2021 (started from 1900)
    cfg_date_time.tm_mon = 6;    // Month: July (Started from 0)
    cfg_date_time.tm_mday = 12;  // Day in Month: 12
    cfg_date_time.tm_hour = 16;  // Hour: 16
    cfg_date_time.tm_min = 45;   // Minute: 45
    cfg_date_time.tm_sec = 41;   // Second: 41
    time_t cfg_time = mktime(&cfg_date_time); // Convert date time to tick
    rtc_config_time(HPM_RTC, cfg_time);
}
// For MCUs that don't have an RNG we still need to provide a rng_get() function,
// eg for lwIP and random.seed().  A pseudo-RNG is not really ideal but we go with
// it for now, seeding with numbers which will be somewhat different each time.  We
// don't want to use urandom's pRNG because then the user won't see a reproducible
// random stream.

// Yasmarang random number generator by Ilya Levin
// http://www.literatecode.com/yasmarang
STATIC uint32_t pyb_rng_yasmarang(void) {
    static bool seeded = false;
    static uint32_t pad = 0, n = 0, d = 0;
    static uint8_t dat = 0;

    if (!seeded) {
        seeded = true;
        hpm_rtc_init();
        pad = read_csr(CSR_CYCLE);
        n = HPM_RTC->SECOND;
        d = HPM_RTC->SUBSEC;
    }

    pad += dat + d * n;
    pad = (pad << 3) + (pad >> 29);
    n = pad | 2;
    d ^= (pad << 31) + (pad >> 1);
    dat ^= (char)pad ^ (d >> 8) ^ 1;

    return pad ^ (d << 5) ^ (pad >> 18) ^ (dat << 1);
}

uint32_t rng_get(void) {
    return pyb_rng_yasmarang();
}

#endif // MICROPY_HW_ENABLE_RNG
