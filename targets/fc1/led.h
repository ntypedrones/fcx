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

#ifndef LED_H
#define	LED_H

#include "p32mx250f128b.h"

static inline void led_set(const unsigned char id)
{
    PORTBSET = 1u << id;
}

static inline void led_clear(const unsigned char id)
{
    PORTBCLR = 1u << id;
}

static inline void led_toggle(const unsigned char id)
{
    PORTBINV = 1u << id;
}

static inline void leds_init(void)
{
    TRISBCLR = (1u << 0u) | (1u << 1u) | (1u << 2u) | (1u << 3u);
    PORTBCLR = (1u << 0u) | (1u << 1u) | (1u << 2u) | (1u << 3u);
}

#endif	/* LED_H */
