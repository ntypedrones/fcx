/* Copyright 2016 Julian Ingram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TARGET_H
#define TARGET_H

#include "i2c.h"
#include "led.h"
#include "pwm.h"
#include "input.h"
#include "p32mx250f128b.h"
#include "gyro.h"
#include "timer.h"

#include <mips/m32c0.h>

enum target_results
{
    TARGET_BUSY,
    TARGET_SUCCESS,
    TARGET_COMM_ERR,
    TARGET_NOT_READY
};

enum target_start_states
{
    TARGET_GYRO_START,
    TARGET_GYRO_CALIB_READ,
    TARGET_GYRO_CALIB_WAIT
};

enum target_reset_states
{
    TARGET_GYRO_WAIT1,
    TARGET_GYRO_RESET,
    TARGET_GYRO_WAIT2
};

struct target
{
    struct gyro gyro;
};

struct target_start
{
    struct gyro_start gyro;
    struct gyro_calib gyro_calib;
    enum target_start_states state;
};

struct target_reset
{
    struct gyro_reset gyro;
    counter_t wait_start;
    enum target_reset_states state;
};

static const unsigned long int SYS_CLK = 40000000UL;


unsigned char target_data_interrupt_get(void);
void target_gie(void);
void target_gid(void);

enum target_results target_idle(struct target* const t);

enum target_results target_start_idle(struct target* const t,
    struct target_start* const s);
void target_start(struct target* const t, struct target_start* const s);

enum target_results target_reset_idle(struct target* const t,
    struct target_reset* const r);
void target_reset(struct target* const t, struct target_reset* const r);

void target_init(struct target* const t);

#endif
