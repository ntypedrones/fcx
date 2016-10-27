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

#include "input.h"

#include "sbus.h"
#include "cirq.h"

#include "p32mx250f128b.h"

#define INPUT_BUF_SIZE (64)

static const unsigned long int INPUT_BAUDRATE = 100000;

struct input_buffer
{
    struct cirq q;
    volatile unsigned char arr[INPUT_BUF_SIZE];
};

struct input
{
    struct input_buffer buffer;
    unsigned char errs;
};
static struct input input;

static inline void input_critical_begin(void)
{
    IEC1CLR = _IEC1_U1RXIE_MASK | _IEC1_U1EIE_MASK;
}

static inline void input_critical_end(void)
{
    IEC1SET = _IEC1_U1RXIE_MASK | _IEC1_U1EIE_MASK;
}

// returns 1 if there is new data.
int input_get(int* const channels)
{
    input_critical_begin();
    while (cirq_population(&input.buffer.q) > 24)
    { // remove all the chars from the buffer until a startbyte
        unsigned char d;
        cirq_peek_front(&input.buffer.q, 0, &d);
        if (d == SBUS_STARTBYTE)
        {
            cirq_peek_front(&input.buffer.q, 24, &d);
            if (d == SBUS_ENDBYTE)
            {
                cirq_pop_front(&input.buffer.q, &d); // startbyte
                unsigned char sbus_buf[22];
                unsigned char i = 0;
                while (i < 22)
                {
                    cirq_pop_front(&input.buffer.q, sbus_buf + i);
                    ++i;
                }
                channels[0] = (int)sbus_get_channel(sbus_buf, 0) << 4;
                channels[1] = (int)sbus_get_channel(sbus_buf, 1) << 4;
                channels[2] = (int)sbus_get_channel(sbus_buf, 2) << 4;
                channels[3] = (int)sbus_get_channel(sbus_buf, 3) << 4;
        	    cirq_pop_front(&input.buffer.q, &d); // endbyte
        	    input_critical_end();
        	    return 1;
            }
        }
        cirq_pop_front(&input.buffer.q, &d); // rubbish data
    }
    input_critical_end();
    return 0;
}

void input_flush(void)
{
    input_critical_begin();
    cirq_flush(&input.buffer.q);
    input_critical_end();
}

unsigned char input_errs_get(void)
{
    return input.errs;
}

void input_errs_clear(const unsigned char errs)
{
    input.errs &= (unsigned char)~errs;
}

void input_init(const unsigned long int clock_frequency)
{
    // init buffer
    input.buffer.q = cirq_init(input.buffer.arr, sizeof(input.buffer.arr) /
        sizeof(input.buffer.arr[0]), sizeof(input.buffer.arr[0]));

    U1MODE = 0; // disable and reset

    // set baudrate
    U1BRG = (clock_frequency / (16 * INPUT_BAUDRATE)) - 1;

    U1MODESET = 0x01 << _U1MODE_PDSEL_POSITION; // even parity
    U1MODESET = 1 << _U1MODE_STSEL_POSITION; // 2 stop bits

    U1STASET = _U1STA_URXEN_MASK;
    U1STACLR = _U1STA_OERR_MASK;

    // clear interrupt flags
    IFS1CLR = _IFS1_U1RXIF_MASK | _IFS1_U1EIF_MASK;

    // enable interrupts
    IEC1SET = _IEC1_U1RXIE_MASK | _IEC1_U1EIE_MASK;

    // set interrupt priority
    IPC8CLR = _IPC8_U1IP_MASK;
    IPC8SET = (1 << _IPC8_U1IP_POSITION); // priority 1
    // set interrupt subpriority
    IPC8CLR =  _IPC8_U1IS_MASK;
    IPC8SET = (0 << _IPC8_U1IS_POSITION); // subpriority 0

    U1MODESET = _U1MODE_ON_MASK; // enable
}

void input_err_isr(void)
{
    if ((U1STA & _U1STA_FERR_MASK) != 0)
    {
        input.errs |= INPUT_FRAMING_ERROR_MASK;
    }
    else if ((U1STA & _U1STA_PERR_MASK) != 0)
    {
        input.errs |= INPUT_PARITY_ERROR_MASK;
    }
    else if ((U1STA & _U1STA_OERR_MASK) != 0)
    {
        input.errs |= INPUT_OVERFLOW_MASK;
        U1STACLR = _U1STA_OERR_MASK;
    }
}

void input_rx_isr(void)
{
    while((U1STA & _U1STA_URXDA_MASK) != 0)
    {
        volatile unsigned char c = (unsigned char)U1RXREG;
        if (cirq_space(&input.buffer.q) != 0)
        {
            cirq_push_back(&input.buffer.q, &c);
        }
    }
}
