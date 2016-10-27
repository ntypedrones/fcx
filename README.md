<!---
Copyright 2016 Julian Ingram

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--->

# FCX Multicopter Flight Controller #

## PID ##

This code is entirely integer based, so the PID settings are a little different
to most.

The settings can be modified in `config.mk` and can also be overriden as
arguments to make.

* `PID_X_P` Proportional weight.
* `PID_X_I` Integral weight.
* `PID_X_D` Derivative weight.
* `PID_X_DIV` PID output divisor.
* `PID_X_LIM` PID output limit.
* `PID_X_ILIM` Integral error limit.

Where `X` represents `P` for Pitch, `R` for Roll or `Y` for Yaw.

##### Detailed docs and schematics coming soon. #####
