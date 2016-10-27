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
 
#include "gyro.h"

#include "mpu6050.h"

static void gyro_config(void)
{
    const unsigned char val = 0x08; // range is 65.5 LSB/deg/s (+-500 deg/s)
    mpu6050_write(MPU6050_GYRO_CONFIG, &val, 1);
}

static int gyro_get_val(void)
{
    unsigned char a[2];
    i2c_buf_read(a, 2);
    int val = ((a[0] & 0x7F) << 8);
    val -= ((a[0] & 0x80) << 8);
    val |= a[1];
    return val; // (val * 15) + (val / 4); // change to millidegs/s
}

static struct gyro_vals gyro_calib_vals_get(struct gyro* const g)
{
    (void)g;
    struct gyro_vals vals;
    vals.arr[GYRO_X] = gyro_get_val();
    vals.arr[GYRO_Y] = gyro_get_val();
    vals.arr[GYRO_Z] = gyro_get_val();
    return vals;
}

struct gyro_vals gyro_vals_get(struct gyro* const g)
{
    struct gyro_vals vals;
    vals.arr[GYRO_X] = gyro_get_val() - g->calib.arr[GYRO_X];
    vals.arr[GYRO_Y] = gyro_get_val() - g->calib.arr[GYRO_Y];
    vals.arr[GYRO_Z] = gyro_get_val() - g->calib.arr[GYRO_Z];
    return vals;
}

void gyro_flush(void)
{
    i2c_buf_flush();
}

enum gyro_results gyro_idle(struct gyro* const g)
{
    (void)g;
    unsigned char events = mpu6050_events_get();
    if ((events & MPU6050_COMM_ERR_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_COMM_ERR_MASK);
        return GYRO_COMM_ERROR;
    }
    if ((events & MPU6050_DONE_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_DONE_MASK);
        return (i2c_buf_population() != 6) ? GYRO_DATA_INVALID :
            GYRO_DATA_READY;
    }
    return GYRO_BUSY;
}

void gyro_read(struct gyro* g)
{
    (void)g;
    mpu6050_read(MPU6050_GYRO_XOUT_H, 6);
}

static enum gyro_results gyro_calib_update(struct gyro* const g,
    struct gyro_calib* const c, const struct gyro_vals vals)
{
    if (c->reset != 0)
    {
        g->calib = vals;
        c->sample_count = 0;
        c->reset = 0;
    }
    else
    {
        size_t i = 0;
        while (i < (sizeof (vals.arr) / sizeof (vals.arr[0])))
        {
            long int diff = vals.arr[i] - g->calib.arr[i];
            if (diff < 0)
            {
                diff = -diff;
            }
            if (diff > ((long int)c->deviation_limit))
            {
                c->reset = 1;
                return GYRO_CALIB_STEP;
            }
            // rolling average
            g->calib.arr[i] += (vals.arr[i] - g->calib.arr[i]) /
                (long int)c->sample_time;
            ++i;
        }

        if (c->sample_count >= c->sample_time)
        { // calibrated
            return GYRO_SUCCESS;
        }
        else
        {
            ++c->sample_count;
        }
    }
    return GYRO_CALIB_STEP;
}

enum gyro_results gyro_calib_idle(struct gyro* const g,
    struct gyro_calib* const c)
{
    unsigned char events = mpu6050_events_get();
    if ((events & MPU6050_COMM_ERR_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_COMM_ERR_MASK);
        return GYRO_COMM_ERROR;
    }
    if ((events & MPU6050_DONE_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_DONE_MASK);
        return (i2c_buf_population() != 6) ? GYRO_DATA_INVALID :
        gyro_calib_update(g, c, gyro_calib_vals_get(g));
    }
    return GYRO_BUSY;
}

void gyro_calib_read(struct gyro* const g, struct gyro_calib* const c)
{
    (void)g;
    (void)c;
    mpu6050_read(MPU6050_GYRO_XOUT_H, 6);
}

void gyro_calib(struct gyro* const g, struct gyro_calib* const c,
    const unsigned int deviation_limit, const unsigned int sample_time)
{
    (void)g;
    c->deviation_limit = deviation_limit;
    c->sample_time = sample_time;
    c->reset = 1;
}

enum gyro_results gyro_start_idle(struct gyro* const g,
    struct gyro_start* const s)
{
    (void)g;
    (void)s;
    unsigned char events = mpu6050_events_get();
    if ((events & MPU6050_COMM_ERR_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_COMM_ERR_MASK);
        return GYRO_COMM_ERROR;
    }
    if ((events & MPU6050_DONE_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_DONE_MASK);
        switch (s->state)
        {
        case GYRO_START_LPF_CONFIG:
            s->state = GYRO_START_SAMPLE_RATE_SET;
            mpu6050_config(0);
            break;
        case GYRO_START_SAMPLE_RATE_SET:
            s->state = GYRO_START_RANGE_SET;
            mpu6050_sample_rate(7);
            break;
        case GYRO_START_RANGE_SET:
            s->state = GYRO_START_INT_CONFIG;
            gyro_config();
            break;
        case GYRO_START_INT_CONFIG:
            s->state = GYRO_START_INT_ENABLE;
            mpu6050_int_config(); // TODO: move these 2 to target
            break;
        case GYRO_START_INT_ENABLE:
            s->state = GYRO_START_DONE;
            mpu6050_int_enable();
            break;
        case GYRO_START_DONE:
            return GYRO_SUCCESS;
        }
    }
    return GYRO_BUSY;
}

void gyro_start(struct gyro* const g, struct gyro_start* const s)
{
    (void)g;
    (void)s;
    s->state = GYRO_START_LPF_CONFIG;
    mpu6050_start();
}

enum gyro_results gyro_reset_idle(struct gyro* const g,
    struct gyro_reset* const r)
{
    (void)g;
    (void)r;
    unsigned char events = mpu6050_events_get();
    if ((events & MPU6050_COMM_ERR_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_COMM_ERR_MASK);
        return GYRO_COMM_ERROR;
    }
    if ((events & MPU6050_DONE_MASK) != 0)
    {
        mpu6050_events_clear(MPU6050_DONE_MASK);
        return GYRO_SUCCESS;
    }
    return GYRO_BUSY;
}

void gyro_reset(struct gyro* const g, struct gyro_reset* const r)
{
    (void)g;
    (void)r;
    mpu6050_reset();
}

void gyro_init(struct gyro* const g, const unsigned long int clock_frequency)
{
    (void)g;
    mpu6050_init(clock_frequency);
}
