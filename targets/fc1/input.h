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

#ifndef INPUT_H
#define INPUT_H

static const unsigned char INPUT_FRAMING_ERROR_MASK = (1 << 0);
static const unsigned char INPUT_PARITY_ERROR_MASK = (1 << 1);
static const unsigned char INPUT_OVERFLOW_MASK = (1 << 2);

// get the channel values
int input_get(int* const channels);
// flush the input buffer
void input_flush(void);
// get error flags
unsigned char input_errs_get(void);
// clear error flags
void input_errs_clear(const unsigned char);
void input_init(const unsigned long int clock_frequency);
void input_err_isr(void);
void input_rx_isr(void);

#endif
