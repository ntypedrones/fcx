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

#ifndef MPU6050_H
#define	MPU6050_H

#include <stdlib.h>

#include "i2c.h"

enum MPU6050_regs
{
    MPU6050_SELF_TEST_X = 0x0d, // RW
    MPU6050_SELF_TEST_Y = 0x0e, // RW
    MPU6050_SELF_TEST_Z = 0x0f, // RW
    MPU6050_SELF_TEST_A = 0x10, // RW
    MPU6050_SMPRT_DIV = 0x19, // RW
    MPU6050_CONFIG = 0x1a, // RW
    MPU6050_GYRO_CONFIG = 0x1b, // RW
    MPU6050_ACCEL_CONFIG = 0x1c, // RW
    MPU6050_FIFO_EN = 0x23, // RW
    MPU6050_I2C_MST_CTRL = 0x24, // RW
    MPU6050_I2C_SLV0_ADDR = 0x25, // RW
    MPU6050_I2C_SLV0_REG = 0x26, // RW
    MPU6050_I2C_SLV0_CTRL = 0x27, // RW
    MPU6050_I2C_SLV1_ADDR = 0x28, // RW
    MPU6050_I2C_SLV1_REG = 0x29, // RW
    MPU6050_I2C_SLV1_CTRL = 0x2a, // RW
    MPU6050_I2C_SLV2_ADDR = 0x2b, // RW
    MPU6050_I2C_SLV2_REG = 0x2c, // RW
    MPU6050_I2C_SLV2_CTRL = 0x2d, // RW
    MPU6050_I2C_SLV3_ADDR = 0x2e, // RW
    MPU6050_I2C_SLV3_REG = 0x2f, // RW
    MPU6050_I2C_SLV3_CTRL = 0x30, // RW
    MPU6050_I2C_SLV4_ADDR = 0x31, // RW
    MPU6050_I2C_SLV4_REG = 0x32, // RW
    MPU6050_I2C_SLV4_DO = 0x33, // RW
    MPU6050_I2C_SLV4_CTRL = 0x34, // RW
    MPU6050_I2C_SLV4_DI = 0x35, // R
    MPU6050_I2C_MST_STATUS = 0x36, // R
    MPU6050_INT_PIN_CFG = 0x37, // RW
    MPU6050_INT_ENABLE = 0x38, // RW
    MPU6050_INT_STATUS = 0x3a, // R
    MPU6050_ACCEL_XOUT_H = 0x3b, // R
    MPU6050_ACCEL_XOUT_L = 0x3c, // R
    MPU6050_ACCEL_YOUT_H = 0x3d, // R
    MPU6050_ACCEL_YOUT_L = 0x3e, // R
    MPU6050_ACCEL_ZOUT_H = 0x3f, // R
    MPU6050_ACCEL_ZOUT_L = 0x40, // R
    MPU6050_TEMP_OUT_H = 0x41, // R
    MPU6050_TEMP_OUT_L = 0x42, // R
    MPU6050_GYRO_XOUT_H = 0x43, // R
    MPU6050_GYRO_XOUT_L = 0x44, // R
    MPU6050_GYRO_YOUT_H = 0x45, // R
    MPU6050_GYRO_YOUT_L = 0x46, // R
    MPU6050_GYRO_ZOUT_H = 0x47, // R
    MPU6050_GYRO_ZOUT_L = 0x48, // R
    MPU6050_EXT_SENS_DATA_00 = 0x49, // R
    MPU6050_EXT_SENS_DATA_01 = 0x4a, // R
    MPU6050_EXT_SENS_DATA_02 = 0x4b, // R
    MPU6050_EXT_SENS_DATA_03 = 0x4c, // R
    MPU6050_EXT_SENS_DATA_04 = 0x4d, // R
    MPU6050_EXT_SENS_DATA_05 = 0x4e, // R
    MPU6050_EXT_SENS_DATA_06 = 0x4f, // R
    MPU6050_EXT_SENS_DATA_07 = 0x50, // R
    MPU6050_EXT_SENS_DATA_08 = 0x51, // R
    MPU6050_EXT_SENS_DATA_09 = 0x52, // R
    MPU6050_EXT_SENS_DATA_10 = 0x53, // R
    MPU6050_EXT_SENS_DATA_11 = 0x54, // R
    MPU6050_EXT_SENS_DATA_12 = 0x55, // R
    MPU6050_EXT_SENS_DATA_13 = 0x56, // R
    MPU6050_EXT_SENS_DATA_14 = 0x57, // R
    MPU6050_EXT_SENS_DATA_15 = 0x58, // R
    MPU6050_EXT_SENS_DATA_16 = 0x59, // R
    MPU6050_EXT_SENS_DATA_17 = 0x5a, // R
    MPU6050_EXT_SENS_DATA_18 = 0x5b, // R
    MPU6050_EXT_SENS_DATA_19 = 0x5c, // R
    MPU6050_EXT_SENS_DATA_20 = 0x5d, // R
    MPU6050_EXT_SENS_DATA_21 = 0x5e, // R
    MPU6050_EXT_SENS_DATA_22 = 0x5f, // R
    MPU6050_EXT_SENS_DATA_23 = 0x60, // R
    MPU6050_I2C_SLV0_DO = 0x63, // RW
    MPU6050_I2C_SLV1_DO = 0x64, // RW
    MPU6050_I2C_SLV2_DO = 0x65, // RW
    MPU6050_I2C_SLV3_DO = 0x66, // RW
    MPU6050_I2C_MST_DELAY_CT_RL = 0x67, // RW
    MPU6050_SIGNAL_PATH_RES_ET = 0x68, // RW
    MPU6050_USER_CTRL = 0x6a, // RW
    MPU6050_PWR_MGMT_1 = 0x6B, // RW
    MPU6050_PWR_MGMT_2 = 0x6C, // RW
    MPU6050_FIFO_COUNT_H = 0x72, // RW
    MPU6050_FIFO_COUNT_L = 0x73, // RW
    MPU6050_FIFO_R_W = 0x74, // RW
    MPU6050_WHO_AM_I = 0x75 // RW
};

static const unsigned char MPU6050_DONE_MASK = (1 << 0);
static const unsigned char MPU6050_DATA_READY_MASK = (1 << 1);
static const unsigned char MPU6050_COMM_ERR_MASK = (1 << 2);

// writes \size registers starting at \addr
void mpu6050_write(const unsigned char addr, const unsigned char* data,
    const size_t size);

// reads \size registers starting at \addr
void mpu6050_read(const unsigned char addr, const size_t size);
// returns event flags
unsigned char mpu6050_events_get(void);
// clears the specified event flags
void mpu6050_events_clear(const unsigned char events);
// enables the interrupt
void mpu6050_int_enable(void);
// configures the interrupt to trigger when new readings are ready
void mpu6050_int_config(void);
// configures the LPF value, lpf should be 0-6
void mpu6050_config(unsigned char lpf);
// sets the sample rate, controls the interrupt rate
void mpu6050_sample_rate(const unsigned char rate);
// enables the device
void mpu6050_start(void);
// reads the device ID
void mpu6050_i2c_whoami(void);
// inits i2c
void mpu6050_init(const unsigned long int clock_frequency);
// soft resets the deveice
void mpu6050_reset(void);

#endif
