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
 
#ifndef TIMER_H
#define TIMER_H

#include "target.h"

#include <mips/m32c0.h>

typedef reg_t counter_t;

static inline counter_t timer_counter_get(void)
{
	return mips32_get_c0(C0_COUNT);
}

static inline unsigned char timer_timeout(const counter_t start,
	const counter_t timeout)
{
	return ((mips32_get_c0(C0_COUNT) - start) >= timeout) ? 1 : 0;
}

static inline unsigned char timer_timeout_ms(const unsigned long int sys_clk,
	const counter_t start, const unsigned int timeout)
{
	return timer_timeout(start, ((sys_clk / 2) / 1000) * timeout);
}

static inline void timer_delay_ms(const unsigned long int sys_clk,
	const unsigned int delay)
{
    const counter_t start = timer_counter_get();
    while (timer_timeout_ms(sys_clk, start, delay) == 0)
    {
    }
}

#endif
