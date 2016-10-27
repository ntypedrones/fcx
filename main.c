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

#include "pid.h"
#include "target.h"

#include <mips/hal.h>
#include <mips/cpu.h>

#include <limits.h>

static const int THROTTLE_ON = 100;
static const int CHANNEL_MIDPOINT = 16384;
static const unsigned int THROTTLE_LOW_TIME = 3;

static void motor_set_speed(const unsigned char motor, int speed)
{
    if (speed > 30000)
    {
        speed = 30000;
    }
    else if (speed < 0)
    {
        speed = 0;
    }
    speed /= 6; // make speed out of 5000
    speed += 5000; // add the "zero throttle" pulse width
    pwm_width_set(motor, (long unsigned int)speed);
}

/* Shuts off motors as a last resort, This function never returns. Hope you
    don't end up here */

void failsafe(void)
{
    motor_set_speed(0, 0);
    motor_set_speed(1, 0);
    motor_set_speed(2, 0);
    motor_set_speed(3, 0);
    led_set(0);
    led_set(1);
    led_set(2);
    led_set(3);
    while (1)
    {
        timer_delay_ms(SYS_CLK, 200);
        led_toggle(0);
        led_toggle(1);
        led_toggle(2);
        led_toggle(3);
    }
}

static inline int err_restrict(const int value, const int limit)
{
    return (value > limit) ? limit : ((value < -limit) ? -limit : value);
}

static void set_motors(struct gyro_vals vals, struct pid pids[3],
    int axis_errs[3], int chs[4])
{
    axis_errs[0] = pid(&pids[0], -chs[1], vals.arr[GYRO_X]);
    axis_errs[1] = pid(&pids[1], -chs[2], vals.arr[GYRO_Y]);
    axis_errs[2] = pid(&pids[2], -chs[3], vals.arr[GYRO_Z]);
    if (chs[0] <= THROTTLE_ON)
    {
        led_set(2);
        motor_set_speed(0, 0);
        motor_set_speed(1, 0);
        motor_set_speed(2, 0);
        motor_set_speed(3, 0);
    }
    else
    {
        led_clear(2);
        motor_set_speed(0, ((chs[0] - axis_errs[0]) + axis_errs[1]) +
            axis_errs[2]);
        motor_set_speed(1, ((chs[0] + axis_errs[0]) + axis_errs[1]) -
            axis_errs[2]);
        motor_set_speed(2, ((chs[0] + axis_errs[0]) - axis_errs[1]) +
            axis_errs[2]);
        motor_set_speed(3, ((chs[0] - axis_errs[0]) - axis_errs[1]) -
            axis_errs[2]);
    }
}

int main()
{
    struct target t;
    target_init(&t);
    led_set(0);
    {
        struct target_reset r;
        target_reset(&t, &r);
        enum target_results res = TARGET_BUSY;
        // errors at this point are handled here by the user actively
        // resetting the board
        while (res != TARGET_SUCCESS)
        {
            res = target_reset_idle(&t, &r);
        }
    }
    led_clear(0);
    led_set(1);
    {
        struct target_start s;
        target_start(&t, &s);
        enum target_results res = TARGET_BUSY;
        // errors at this point are handled here by the user actively
        // resetting the board
        while (res != TARGET_SUCCESS)
        {
            res = target_start_idle(&t, &s);
        }
    }
    led_clear(1);

    struct pid pids[3] = {
        {
            .ierr = 0,
            .last = 0,
            .p = PID_P_P,
            .i = PID_P_I,
            .d = PID_P_D,
            .div = PID_P_DIV,
            .lim = PID_P_LIM,
            .ierr_lim = PID_P_ILIM
        },
        {
            .ierr = 0,
            .last = 0,
            .p = PID_R_P,
            .i = PID_R_I,
            .d = PID_R_D,
            .div = PID_R_DIV,
            .lim = PID_R_LIM,
            .ierr_lim = PID_R_ILIM
        },
        {
            .ierr = 0,
            .last = 0,
            .p = PID_Y_P,
            .i = PID_Y_I,
            .d = PID_Y_D,
            .div = PID_Y_DIV,
            .lim = PID_Y_LIM,
            .ierr_lim = PID_Y_ILIM
        }
    };

    unsigned char gyro_read_pending = 0;

    int throttle_low = INT_MAX;
    unsigned char throttle_low_count = 0;

    int chs[4] = {0};

    int axis_errs[3] = {0};

    while (1)
    {
        // process gyros and set motor speeds
        if (target_data_interrupt_get() && (gyro_read_pending == 0))
        { // 1ms tick
            gyro_read_pending = 1;
            gyro_read(&t.gyro);
        }
        switch (gyro_idle(&t.gyro))
        {
            case GYRO_DATA_READY:
                set_motors(gyro_vals_get(&t.gyro), pids, axis_errs, chs);
                gyro_read_pending = 0;
                break;
            case GYRO_BUSY:
                break;
            default:
                led_set(3);
                gyro_read_pending = 0;
                gyro_flush();
                break;
        }

        // read uart, sbus
        unsigned char input_errs = input_errs_get();
        if (input_errs != 0)
        {
            led_set(3);
            input_flush();
            input_errs_clear(input_errs);
        }
    	if (input_get(chs) != 0)
        {
            // modify throttle_low if required
        	if ((chs[0] < throttle_low) && (chs[0] > 0))
        	{
                if (throttle_low_count > 3)
                {
                    throttle_low = chs[0];
                    throttle_low_count = 0;
                }
                else
                {
                    ++throttle_low_count;
                }
                chs[0] = 0;
            }
            else
            {
                throttle_low_count = 0;
                chs[0] -= throttle_low;
            }
        	// offset other channels
        	chs[1] -= CHANNEL_MIDPOINT;
        	chs[2] -= CHANNEL_MIDPOINT;
        	chs[3] -= CHANNEL_MIDPOINT;
        }
    }
    return -1;
}
