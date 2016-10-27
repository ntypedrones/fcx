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

#include "target.h"

#include <stdlib.h>
#include <string.h>

unsigned long int DEVCFG0_VAL __attribute__ ((section(".devcfg0"))) = 0xFFFFFFFB;
unsigned long int DEVCFG1_VAL __attribute__ ((section(".devcfg1"))) = 0xFF7FCE5B;
unsigned long int DEVCFG2_VAL __attribute__ ((section(".devcfg2"))) = 0xFFF9FFD9;
unsigned long int DEVCFG3_VAL __attribute__((section(".devcfg3"))) = 0x3FFF2448;

extern char _rom_data_copy __attribute__((section(".data")));
extern char _fdata __attribute__((section(".data")));
extern char _edata __attribute__((section(".data")));

void __attribute__((section(".boot"))) software_init_hook(void)
{
	// copy initialised data from its LMA to VMA
	memcpy(&_fdata, &_rom_data_copy, (unsigned long int)&_edata -
        (unsigned long int)&_fdata);
}

unsigned char target_data_interrupt_get(void)
{
	return (PORTBbits.RB7 != 0) ? 1 : 0;
}

void target_gie(void)
{
    __asm__ __volatile__("ei");
    __asm__ __volatile__("ehb");
}

void target_gid(void)
{
    __asm__ __volatile__("di");
    __asm__ __volatile__("ehb");
}

enum target_results target_idle(struct target* const t)
{
	(void)t;
	return TARGET_BUSY;
}

enum target_results target_start_idle(struct target* const t,
	struct target_start* const s)
{
	switch (s->state)
	{
	case TARGET_GYRO_START:
		switch (gyro_start_idle(&t->gyro, &s->gyro))
		{
		case GYRO_BUSY:
			return TARGET_BUSY;
		case GYRO_SUCCESS:
			s->state = TARGET_GYRO_CALIB_READ;
			gyro_calib(&t->gyro, &s->gyro_calib, 100, 2000);
			return TARGET_BUSY;
		default:
			break;
		}
		break;
	case TARGET_GYRO_CALIB_READ:
		if (target_data_interrupt_get() != 0)
		{
			s->state = TARGET_GYRO_CALIB_WAIT;
			gyro_calib_read(&t->gyro, &s->gyro_calib);
		}
		return TARGET_BUSY;
	case TARGET_GYRO_CALIB_WAIT:
		switch (gyro_calib_idle(&t->gyro, &s->gyro_calib))
		{
		case GYRO_CALIB_STEP:
			s->state = TARGET_GYRO_CALIB_READ;
			// FALLS THROUGH
		case GYRO_BUSY:
			return TARGET_BUSY;
		case GYRO_SUCCESS:
			return TARGET_SUCCESS;
		default:
			break;
		}
		break;
	}
	return TARGET_COMM_ERR;
}

void target_start(struct target* const t, struct target_start* const s)
{
	(void)t;
    pwm_start(4);
	s->state = TARGET_GYRO_START;
	gyro_start(&t->gyro, &s->gyro);
}

enum target_results target_reset_idle(struct target* const t,
	struct target_reset* const r)
{
	switch (r->state)
	{
		case TARGET_GYRO_WAIT1:
			if (timer_timeout_ms(SYS_CLK, r->wait_start, 100) != 0)
			{
				r->state = TARGET_GYRO_RESET;
				gyro_reset(&t->gyro, &r->gyro);
			}
			return TARGET_BUSY;
		case TARGET_GYRO_RESET:
			switch (gyro_reset_idle(&t->gyro, &r->gyro))
			{
				case GYRO_BUSY:
					return TARGET_BUSY;
			    case GYRO_SUCCESS:
					r->wait_start = timer_counter_get();
					r->state = TARGET_GYRO_WAIT2;
					return TARGET_BUSY;
				default:
					break;
			}
			break;
		case TARGET_GYRO_WAIT2:
			return (timer_timeout_ms(SYS_CLK, r->wait_start, 100) != 0) ?
				TARGET_SUCCESS : TARGET_BUSY;
	}
	return TARGET_COMM_ERR;
}

void target_reset(struct target* const t, struct target_reset* const r)
{
	(void)t;
	r->wait_start = timer_counter_get();
	r->state = TARGET_GYRO_WAIT1;
}

void target_init(struct target* const t)
{
	// ansel to digital
	ANSELA = 0;
	ANSELB = 0;

	// single vector mode
	target_gid();

	// set ebase
	mips32_set_c0(C0_EBASE, 0x9d000000);
	const unsigned long int tmp = mips32_get_c0(C0_CAUSE) | 0x00800000;
	mips32_set_c0(C0_CAUSE, tmp);

	INTCONCLR = _INTCON_MVEC_MASK;

	// PPS
	// sbus input
	U1RXR = 0x02; // RPA4

	// PWM output
	RPA0R = 0x05; // OC1
	RPA1R = 0x05; // OC2
	RPB14R = 0x05; // OC3
	RPB13R = 0x05; // OC4

	// i2c is fixed

	// setup input for mpu6050 interrupt
	TRISBSET = _TRISB_TRISB7_MASK;

	// init
	leds_init();
	input_init(SYS_CLK);
	pwm_init(4, 20000);
	pwm_width_set(0, 5000);
	pwm_width_set(1, 5000);
	pwm_width_set(2, 5000);
	pwm_width_set(3, 5000);
	gyro_init(&t->gyro, SYS_CLK);

	// enable global interrupts
	target_gie();
}

void __attribute__ ((interrupt, keep_interrupts_masked)) _mips_interrupt(void)
{
    if (((IEC1 & _IEC1_I2C1BIE_MASK) != 0) && ((IFS1 & _IFS1_I2C1BIF_MASK) != 0)) // I2C1 bus collision
    {
		i2c_col_isr();
        IFS1CLR = _IFS1_I2C1BIF_MASK;
    }
    if (((IEC1 & _IEC1_I2C1MIE_MASK) != 0) && ((IFS1 & _IFS1_I2C1MIF_MASK) != 0)) // I2C1 master
    {
        i2c_mstr_isr();
        IFS1CLR = _IFS1_I2C1MIF_MASK;
	}
	if (((IEC1 & _IEC1_U1EIE_MASK) != 0) && ((IFS1 & _IFS1_U1EIF_MASK) != 0)) // UART1 error
    {
        input_err_isr();
        IFS1CLR = _IFS1_U1EIF_MASK;
    }
	if (((IEC1 & _IEC1_U1RXIE_MASK) != 0) && ((IFS1 & _IFS1_U1RXIF_MASK) != 0)) // UART1 receive
	{
		input_rx_isr();
		IFS1CLR = _IFS1_U1RXIF_MASK;
	}
}
