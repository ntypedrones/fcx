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
 
#ifndef GYRO_H
#define GYRO_H

enum gyro_axis
{
    GYRO_X = 0,
    GYRO_Y = 1,
    GYRO_Z = 2
};

enum gyro_results
{
    GYRO_BUSY = 0,
    GYRO_SUCCESS = 1,
    GYRO_DATA_READY = 2,
    GYRO_CALIB_STEP = 3,
    GYRO_ERROR = -1,
    GYRO_COMM_ERROR = -2,
    GYRO_DATA_INVALID = -3
};

enum gyro_start_states
{
    GYRO_START_LPF_CONFIG,
    GYRO_START_SAMPLE_RATE_SET,
    GYRO_START_RANGE_SET,
    GYRO_START_INT_CONFIG,
    GYRO_START_INT_ENABLE,
    GYRO_START_DONE
};

enum gyro_reset_states
{
    GYRO_RESET_DONE,
};

struct gyro_vals
{
    long int arr[3];
};

struct gyro_calib
{
    unsigned int deviation_limit;
    unsigned int sample_time;
    unsigned int sample_count;
    unsigned char reset;
};

struct gyro_start
{
    enum gyro_start_states state;
};

struct gyro_reset
{
    enum gyro_reset_states state;
};

struct gyro
{
    struct gyro_vals calib;
};

struct gyro_vals gyro_vals_get(struct gyro* const g);
void gyro_flush(void);

enum gyro_results gyro_idle(struct gyro* const g);
void gyro_read(struct gyro* const g);

// To calibrate: Call gyro_calib once with uninitialised structs to start the
// process. Then call gyro_calib_read on every sample ready interrupt, while
// calling gyro_calib_idle in a loop until it stops returning GYRO_BUSY.

enum gyro_results gyro_calib_idle(struct gyro* const g,
    struct gyro_calib* const c);
void gyro_calib_read(struct gyro* const g, struct gyro_calib* const c);
void gyro_calib(struct gyro* const g, struct gyro_calib* const c,
    const unsigned int deviation_limit, const unsigned int sample_time);

// To start: Similar to reset below.

enum gyro_results gyro_start_idle(struct gyro* const g,
    struct gyro_start* const s);
void gyro_start(struct gyro* const g, struct gyro_start* const s);

// To reset: Call gyro_reset once with uninitialised structs to start the
// process. Then call gyro_reset_idle in a loop until it stops returning
// GYRO_BUSY.

enum gyro_results gyro_reset_idle(struct gyro* const g,
    struct gyro_reset* const r);
void gyro_reset(struct gyro* const g, struct gyro_reset* const r);

void gyro_init(struct gyro* const g, const unsigned long int clock_frequency);

#endif
