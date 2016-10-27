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

#ifndef I2C_H
#define	I2C_H

#include <stdlib.h>

static const unsigned char I2C_NACK_MASK = (1 << 0);
static const unsigned char I2C_COLLISION_MASK = (1 << 1);
static const unsigned char I2C_ERROR_MASK = (1 << 2);
static const unsigned char I2C_DATA_READY_MASK = (1 << 3);
static const unsigned char I2C_STOPPED_MASK = (1 << 4);

// executes a write operation
void i2c_write(const unsigned char slv_addr, const unsigned char reg_addr,
    const unsigned char* const data, const size_t size);
// executes a read operation, does not return with the data
void i2c_read(const unsigned char slv_addr, const unsigned char reg_addr,
    const size_t size);
// returns a set of flags indicating which events require handling
unsigned char i2c_events_get(void);
// clears the events after they have been handled
void i2c_events_clear(const unsigned char);
// returns the population of the read data buffer
size_t i2c_buf_population(void);
// reads up to out_buf_size from the read data buffer into out_buf
// does not check the population of the buffer so be sure to do this externally
void i2c_buf_read(unsigned char* data, const size_t size);
// clears out the read data buffer
void i2c_buf_flush(void);
// initialises and starts the i2c interface
void i2c_init(const unsigned long int clock_frequency,
    const unsigned long int baudrate);
void i2c_col_isr(void);
void i2c_mstr_isr(void);

#endif
