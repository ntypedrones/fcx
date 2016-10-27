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

#include "pwm.h"

void pwm_width_set(const unsigned char channel_id,
    const unsigned long int width)
{
    switch(channel_id)
    {
    case 0:
        OC1RS = width;
        break;
    case 1:
        OC2RS = width;
        break;
    case 2:
        OC3RS = width;
        break;
    case 3:
        OC4RS = width;
        break;
    }
}

void pwm_start(const unsigned char channel_count)
{
    switch(channel_count) // note the absence of break
    {
    case 4:
        OC4CONSET = _OC4CON_ON_MASK; // enable
    case 3:
        OC3CONSET = _OC3CON_ON_MASK; // enable
    case 2:
        OC2CONSET = _OC2CON_ON_MASK; // enable
    case 1:
        OC1CONSET = _OC1CON_ON_MASK; // enable
    }

    T2CONSET = 1 << _T2CON_ON_POSITION; // enable timer
}

void pwm_init(const unsigned char channel_count,
    const unsigned long int timer_period)
{
    switch(channel_count) // note the absence of break
    {
    case 4:
        OC4CONCLR = 0xffffffff; // disable and reset
        __asm__ __volatile__("nop");
        OC4CONSET = 0x06 << _OC4CON_OCM_POSITION; // PWM mode
    case 3:
        OC3CONCLR = 0xffffffff; // disable and reset
        __asm__ __volatile__("nop");
        OC3CONSET = 0x06 << _OC3CON_OCM_POSITION; // PWM mode
    case 2:
        OC2CONCLR = 0xffffffff; // disable and reset
        __asm__ __volatile__("nop");
        OC2CONSET = 0x06 << _OC2CON_OCM_POSITION; // PWM mode
    case 1:
        OC1CONCLR = 0xffffffff; // disable and reset
        __asm__ __volatile__("nop");
        OC1CONSET = 0x06 << _OC1CON_OCM_POSITION; // PWM mode
    }

    T2CONCLR = 0xffffffff; // disable and reset
    __asm__ __volatile__("nop");
    T2CONSET = 0x03 << _T2CON_TCKPS_POSITION; // 1:8 prescaler
    TMR2 = 0;
    PR2 = timer_period;
}
