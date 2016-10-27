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

#include "i2c.h"

#include "cirq.h"
#include "p32mx250f128b.h"

#define I2C_CMDS_BUF_SIZE (64)
#define I2C_OUT_BUF_SIZE (64)
#define I2C_IN_BUF_SIZE (64)

enum i2c_cmds
{
    I2C_START = 0,
    I2C_REP_START,
    I2C_TRANSMIT,
    I2C_RECEIVE,
    I2C_SEND_NACK,
    I2C_SEND_ACK,
    I2C_STOP,
    I2C_ERR_STOP
};

struct i2c
{
    struct cirq cmds;
    struct cirq out;
    struct cirq in;
    volatile enum i2c_cmds cmds_buf[I2C_CMDS_BUF_SIZE];
    volatile unsigned char out_buf[I2C_OUT_BUF_SIZE];
    volatile unsigned char in_buf[I2C_IN_BUF_SIZE];
    volatile unsigned char events;
    volatile unsigned char nack_expected;
    volatile unsigned char stop;
};
static struct i2c i2c;

static inline void i2c_critical_begin(void)
{
    IEC1CLR = _IEC1_I2C1BIE_MASK | _IEC1_I2C1MIE_MASK;
}

static inline void i2c_critical_end(void)
{
    IEC1SET = _IEC1_I2C1BIE_MASK | _IEC1_I2C1MIE_MASK;
}

static inline void i2c_tx(const unsigned char data)
{
    const enum i2c_cmds cmd = I2C_TRANSMIT;
    cirq_push_back(&i2c.cmds, &cmd);
    cirq_push_back(&i2c.out, &data);
}

void i2c_write(const unsigned char slv_addr, const unsigned char reg_addr,
               const unsigned char* const data, const size_t size)
{
    if (size != 0)
    {
        i2c_critical_begin();
        if ((cirq_empty(&i2c.cmds) != 0) && (i2c.stop == 0))
        {   // start enable
            I2C1CONbits.SEN = 1;
        }
        else
        {   // queue start
            const enum i2c_cmds cmd = I2C_START;
            cirq_push_back(&i2c.cmds, &cmd);
        }
        i2c_tx(slv_addr & (unsigned char)(~1u));
        i2c_tx(reg_addr);
        size_t i = 0;
        while (i < size)
        {
            i2c_tx(data[i]);
            ++i;
        }
        const enum i2c_cmds cmd = I2C_STOP;
        cirq_push_back(&i2c.cmds, &cmd);
        i2c_critical_end();
    }
}

// this sends the stuff off to be read, does not actually return with the data
void i2c_read(const unsigned char slv_addr, const unsigned char reg_addr,
              const size_t size)
{
    if (size != 0)
    {
        i2c_critical_begin();
        if ((cirq_empty(&i2c.cmds) != 0) && (i2c.stop == 0))
        {   // start enable
            I2C1CONbits.SEN = 1;
        }
        else
        {   // queue start
            const enum i2c_cmds cmd = I2C_START;
            cirq_push_back(&i2c.cmds, &cmd);
        }
        i2c_tx(slv_addr & (unsigned char)(~1u)); // write
        i2c_tx(reg_addr);
        enum i2c_cmds cmd = I2C_REP_START;
        cirq_push_back(&i2c.cmds, &cmd);
        i2c_tx(slv_addr | 1); // read
        size_t i = 0;
        while (i < (size - 1))
        {
            cmd = I2C_RECEIVE;
            cirq_push_back(&i2c.cmds, &cmd);
            cmd = I2C_SEND_ACK;
            cirq_push_back(&i2c.cmds, &cmd);
            ++i;
        }
        cmd = I2C_RECEIVE;
        cirq_push_back(&i2c.cmds, &cmd);
        cmd = I2C_SEND_NACK;
        cirq_push_back(&i2c.cmds, &cmd);
        cmd = I2C_STOP;
        cirq_push_back(&i2c.cmds, &cmd);
        i2c_critical_end();
    }
}

unsigned char i2c_events_get(void)
{
    i2c_critical_begin();
    const unsigned char ret = i2c.events;
    i2c_critical_end();
    return ret;
}

void i2c_events_clear(const unsigned char events)
{
    i2c_critical_begin();
    i2c.events &= (unsigned char)~events;
    i2c_critical_begin();
}

size_t i2c_buf_population(void)
{
    i2c_critical_begin();
    const size_t ret = cirq_population(&i2c.in);
    i2c_critical_end();
    return ret;
}

void i2c_buf_read(unsigned char* data, const size_t size)
{
    unsigned char* const lim = data + size;
    i2c_critical_begin();
    while (data < lim)
    {
        cirq_pop_front(&i2c.in, data);
        ++data;
    }
    i2c_critical_end();
}

void i2c_buf_flush(void)
{
    i2c_critical_begin();
    cirq_flush(&i2c.in);
    i2c_critical_end();
}

static inline void i2c_set_baudrate(const unsigned long int clock_frequency,
				    const unsigned long int baudrate)
{
    unsigned int baudrate_div = (clock_frequency / (2 * baudrate)) -
        (clock_frequency / 10000000) - 2;
    I2C1BRG = baudrate_div & 0x0fff;
}

void i2c_init(const unsigned long int clock_frequency,
    const unsigned long int baudrate)
{
    I2C1CONCLR = 0xffffffff; // disable and reset
    __asm__ __volatile__("nop");
    i2c_set_baudrate(clock_frequency, baudrate); // set baudrate
    // init buffers
    i2c.cmds = cirq_init(i2c.cmds_buf, sizeof(i2c.cmds_buf) /
        sizeof(i2c.cmds_buf[0]), sizeof(i2c.cmds_buf[0]));
    i2c.out = cirq_init(i2c.out_buf, sizeof(i2c.out_buf) /
        sizeof(i2c.out_buf[0]), sizeof(i2c.out_buf[0]));
    i2c.in = cirq_init(i2c.in_buf, sizeof(i2c.in_buf) / sizeof(i2c.in_buf[0]),
        sizeof(i2c.in_buf[0]));
    i2c.events = 0;
    i2c.nack_expected = 0;
    i2c.stop = 0;
    // clear interrupt flags
    IFS1CLR = _IFS1_I2C1BIF_MASK | _IFS1_I2C1MIF_MASK;
    // enable interrupts
    IEC1SET = _IEC1_I2C1BIE_MASK | _IEC1_I2C1MIE_MASK;
    // set interrupt priority
    IPC8CLR = _IPC8_I2C1IP_MASK;

    IPC8SET = (1 << _IPC8_I2C1IP_POSITION); // priority 1
    // set interrupt subpriority
    IPC8CLR =  _IPC8_I2C1IS_MASK;
    IPC8SET = (0 << _IPC8_I2C1IS_POSITION); // subpriority 0

    I2C1CONSET = _I2C1CON_DISSLW_MASK;
    // enable
    I2C1CONSET =  _I2C1CON_ON_MASK;
}

static void i2c_next(void)
{
    while (cirq_empty(&i2c.cmds) == 0)
    {
        enum i2c_cmds cmd;
        cirq_peek_front(&i2c.cmds, 0, &cmd);
        if (cmd == I2C_START)
        {
            break;
        }
        else if ((cmd == I2C_TRANSMIT) && (cirq_empty(&i2c.out) == 0))
        {
            unsigned char g;
            cirq_pop_front(&i2c.out, &g); // g ignored
        }
        cirq_pop_front(&i2c.cmds, &cmd); // cmd ignored
    }
}

static void i2c_error(unsigned char error)
{
    i2c.events |= error;
    i2c.nack_expected = 0;
    i2c.stop = 0;
    cirq_flush(&i2c.cmds);
    cirq_flush(&i2c.out);
}

static void i2c_handle_cmds(void)
{
    if (cirq_empty(&i2c.cmds) == 0)
    {
        enum i2c_cmds cmd;
        cirq_pop_front(&i2c.cmds, &cmd);
        switch (cmd)
        {
        case I2C_START:
            if ((I2C1STATbits.P != 0) || (I2C1STATbits.S == 0))
            {
                I2C1CONbits.SEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        case I2C_REP_START:
	        if ((I2C1CON & ((1 << 5) - 1)) == 0)
            {
                I2C1CONbits.RSEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        case I2C_TRANSMIT:
        {
            unsigned char c;
            cirq_pop_front(&i2c.out, &c);
            I2C1TRN = c;
            break;
        }
        case I2C_RECEIVE:
            if ((I2C1CON & ((1 << 5) - 1)) == 0)
            {
                I2C1CONbits.RCEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        case I2C_SEND_ACK:
        {
            // read
            unsigned char c = (unsigned char)I2C1RCV;
            cirq_push_back(&i2c.in, &c);
            // send ack
            if ((I2C1CON & ((1 << 5) - 1)) == 0)
            {
                I2C1CONbits.ACKDT = 0;
                I2C1CONbits.ACKEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        }
        case I2C_SEND_NACK:
        {
            // read
            unsigned char c = (unsigned char)I2C1RCV;
            cirq_push_back(&i2c.in, &c);
            // send nack
            if ((I2C1CON & ((1 << 5) - 1)) == 0)
            {
                i2c.nack_expected = 1;
                I2C1CONbits.ACKDT = 1;
                I2C1CONbits.ACKEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        }
        case I2C_STOP:
            i2c.stop = 1;
            // FALLS THROUGH
        case I2C_ERR_STOP:
            if ((I2C1CON & ((1 << 5) - 1)) == 0)
            {
                I2C1CONbits.PEN = 1;
            }
            else
            {
                i2c_error(I2C_ERROR_MASK);
            }
            break;
        }
    }
}

void i2c_col_isr(void)
{
    i2c_error(I2C_COLLISION_MASK);
}

void i2c_mstr_isr(void)
{
    if (I2C1STATbits.ACKSTAT != 0)
    { // received nack
        if (i2c.nack_expected == 0)
        {
            i2c.events |= I2C_NACK_MASK;
            i2c_next();
            const enum i2c_cmds cmd = I2C_ERR_STOP;
            cirq_push_front(&i2c.cmds, &cmd);
        }
        else
        {
            i2c.events |= I2C_DATA_READY_MASK;
            i2c.nack_expected = 0;
        }
    }
    if (i2c.stop != 0)
    {
        i2c.events |= I2C_STOPPED_MASK;
        i2c.stop = 0;
    }
    i2c_handle_cmds();
}
