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

#include "mpu6050.h"

static const unsigned char MPU650_I2C_ADDR = 0xD0;

void mpu6050_write(const unsigned char addr, const unsigned char* data,
    const size_t size)
{
    i2c_write(MPU650_I2C_ADDR, addr, data, size);
}

void mpu6050_read(const unsigned char addr, const size_t size)
{
    i2c_read(MPU650_I2C_ADDR, addr, size);
}

unsigned char mpu6050_events_get(void)
{
    const unsigned char i2c_events = i2c_events_get();
    unsigned char events = 0;
    if ((i2c_events & I2C_ERROR_MASK) != 0)
    {
        events |= MPU6050_COMM_ERR_MASK;
    }
    if ((i2c_events & I2C_COLLISION_MASK) != 0)
    {
        events |= MPU6050_COMM_ERR_MASK;
    }
    if ((i2c_events & I2C_NACK_MASK) != 0)
    {
        events |= MPU6050_COMM_ERR_MASK;
    }
    if ((i2c_events & I2C_DATA_READY_MASK) != 0)
    {
        events |= MPU6050_DATA_READY_MASK;
    }
    if ((i2c_events & I2C_STOPPED_MASK) != 0)
    {
        events |= MPU6050_DONE_MASK;
    }
    return events;
}

void mpu6050_events_clear(const unsigned char events)
{
    if ((events & MPU6050_COMM_ERR_MASK) != 0)
    {
        i2c_events_clear(I2C_COLLISION_MASK | I2C_NACK_MASK | I2C_ERROR_MASK);
    }
    if ((events & MPU6050_DATA_READY_MASK) != 0)
    {
        i2c_events_clear(I2C_DATA_READY_MASK);
    }
    if ((events & MPU6050_DONE_MASK) != 0)
    {
        i2c_events_clear(I2C_STOPPED_MASK);
    }
}

void mpu6050_int_enable(void)
{
    const unsigned char val = 0x01;
    mpu6050_write(MPU6050_INT_ENABLE, &val, 1);
}

void mpu6050_int_config(void)
{
    const unsigned char val = 0x30; // latch high and clear on read
    mpu6050_write(MPU6050_INT_PIN_CFG, &val, 1);
}

void mpu6050_config(unsigned char lpf)
{
    lpf &= 0x07;
    mpu6050_write(MPU6050_CONFIG, &lpf, 1);
}

void mpu6050_sample_rate(const unsigned char rate)
{
    mpu6050_write(MPU6050_SMPRT_DIV, &rate, 1);
}

void mpu6050_start(void)
{
    const unsigned char val = 0x03;
    mpu6050_write(MPU6050_PWR_MGMT_1, &val, 1);
}

void mpu6050_i2c_whoami(void)
{
    mpu6050_read(0x75, 1);
}

void mpu6050_init(const unsigned long int clock_frequency)
{
    i2c_init(clock_frequency, 320000);
}

void mpu6050_reset(void)
{
    unsigned char val = 0x80;
    mpu6050_write(MPU6050_PWR_MGMT_1, &val, 1);
}
