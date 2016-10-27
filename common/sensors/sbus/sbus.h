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

#ifndef SBUS_H
#define	SBUS_H

#include <stdint.h>

static const unsigned char SBUS_STARTBYTE = 0x0f;
static const unsigned char SBUS_ENDBYTE = 0x00;

enum sbus_flags
{
    sbus_failsafe = 0x10,
    sbus_frame_lost = 0x20,
    sbus_ch18 = 0x40,
    sbus_ch17 = 0x80,
};

static inline unsigned int sbus_get_channel8(const unsigned char* buf,
    const unsigned char channel)
{
    switch (channel)
    {
    case 0:
        return (buf[0] | (((unsigned int)buf[1]) << 8)) & 0x7FF;
    case 1:
        return ((buf[1] >> 3) | (((unsigned int)buf[2]) << 5)) & 0x7FF;
    case 2:
        return ((buf[2] >> 6) | (((unsigned int)buf[3]) << 2) |
                (((unsigned int)buf[4]) << 10)) & 0x7FF;
    case 3:
        return ((buf[4] >> 1) | (((unsigned int)buf[5]) << 7)) & 0x7FF;
    case 4:
        return ((buf[5] >> 4) | (((unsigned int)buf[6]) << 4)) & 0x7FF;
    case 5:
        return ((buf[6] >> 7) | (((unsigned int)buf[7]) << 1) |
                (((unsigned int)buf[9]) << 9)) & 0x7FF;
    case 6:
        return ((buf[8] >> 2) | (((unsigned int)buf[9]) << 6)) & 0x7FF;
    case 7:
        return (buf[9] >> 5) | (((unsigned int)buf[10]) << 3);
    }
    return 0;
}

static inline unsigned int sbus_get_channel(const unsigned char* buf,
    const unsigned char channel)
{
    return sbus_get_channel8((channel >= 8) ? buf + 11: buf, channel % 8);
}

#endif
