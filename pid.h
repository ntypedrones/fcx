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

#ifndef PID_H
#define	PID_H

struct pid
{
    int ierr;
    int last;
    int p;
    int i;
    int d;
    int div;
    int lim;
    int ierr_lim;
};

static inline int pid_restrict(const int val, const int lim)
{
    return (val > lim) ? lim : ((val < -lim) ? -lim : val);
}

static inline int pid(struct pid * const j, const int desired,
                       const int actual)
{
    const int err = desired - actual;
    j->ierr = pid_restrict(j->ierr + err, j->ierr_lim);
    const int out = (err * j->p) + (j->ierr * j->i) + ((actual - j->last) *
        j->d);
    j->last = actual;
    return pid_restrict(out / j->div, j->lim);
}

#endif
