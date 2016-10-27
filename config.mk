# Copyright 2016 Julian Ingram
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# 	http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Hardware target
HW ?= fc1

# Pitch weights
PID_P_P ?= 16
PID_P_I ?= 1
PID_P_D ?= 16
PID_P_DIV ?= 16
PID_P_LIM ?= 20000
PID_P_ILIM ?= 42

# Yaw weights
PID_Y_P ?= 16
PID_Y_I ?= 1
PID_Y_D ?= 16
PID_Y_DIV ?= 16
PID_Y_LIM ?= 20000
PID_Y_ILIM ?= 42

# Roll weights
PID_R_P ?= PID_P_P
PID_R_I ?= PID_P_I
PID_R_D ?= PID_P_D
PID_R_DIV ?= PID_P_DIV
PID_R_LIM ?= PID_P_LIM
PID_R_ILIM ?= PID_P_ILIM
