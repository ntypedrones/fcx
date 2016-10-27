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

CONFIG ?= config.mk

-include $(CONFIG)

DEFINES += PID_P_P=\($(PID_P_P)\)
DEFINES += PID_P_I=\($(PID_P_I)\)
DEFINES += PID_P_D=\($(PID_P_D)\)
DEFINES += PID_P_DIV=\($(PID_P_DIV)\)
DEFINES += PID_P_LIM=\($(PID_P_LIM)\)
DEFINES += PID_P_ILIM=\($(PID_P_ILIM)\)
DEFINES += PID_R_P=\($(PID_R_P)\)
DEFINES += PID_R_I=\($(PID_R_I)\)
DEFINES += PID_R_D=\($(PID_R_D)\)
DEFINES += PID_R_DIV=\($(PID_R_DIV)\)
DEFINES += PID_R_LIM=\($(PID_R_LIM)\)
DEFINES += PID_R_ILIM=\($(PID_R_ILIM)\)
DEFINES += PID_Y_P=\($(PID_Y_P)\)
DEFINES += PID_Y_I=\($(PID_Y_I)\)
DEFINES += PID_Y_D=\($(PID_Y_D)\)
DEFINES += PID_Y_DIV=\($(PID_Y_DIV)\)
DEFINES += PID_Y_LIM=\($(PID_Y_LIM)\)
DEFINES += PID_Y_ILIM=\($(PID_Y_ILIM)\)

MODULES := targets/$(HW)
SRCS := main.c
TARGET := fcx.elf
RM := rm -rf
MKDIR := mkdir -p
BUILDDIR := build
DEPDIR := dep
CFLAGS :=
LDFLAGS :=
IDIRS :=
DEPFLAGS = -MMD -MP -MF $(@:$(BUILDDIR)/%.o=$(DEPDIR)/%.d)
RM_ON_CLEAN := $(TARGET) $(DEPDIR) $(BUILDDIR)

-include $(MODULES:=/module.mk)

CFLAGS += -g3 -O2 -Werror -Wall -Wextra -Wpedantic -Wconversion -Wcast-align
CFALGS += -Wint-to-pointer-cast -Wstrict-prototypes -Wcast-qual -Wundef
CFLAGS += -Wstrict-overflow=5 -Wwrite-strings -Wshadow -Wfloat-equal
CFLAGS += -pedantic-errors -std=c11 $(IDIRS:%=-I%/) $(DEFINES:%=-D%)
LDFLAGS += -g3 -O2

SRCS := $(filter %.c %.S %.s, $(SRCS))

OBJS := $(SRCS:%.c=$(BUILDDIR)/%.o)
OBJS := $(OBJS:%.S=$(BUILDDIR)/%.o)
OBJS := $(OBJS:%.s=$(BUILDDIR)/%.o)
DEPS := $(OBJS:$(BUILDDIR)/%.o=$(DEPDIR)/%.d)

.PHONY: all
all: $(TARGET)

# link
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

# compile and/or generate dep files
$(BUILDDIR)/%.o: %.c
	$(MKDIR) $(BUILDDIR)/$(dir $<)
	$(MKDIR) $(DEPDIR)/$(dir $<)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o: %.S
	$(MKDIR) $(BUILDDIR)/$(dir $<)
	$(MKDIR) $(DEPDIR)/$(dir $<)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(RM_ON_CLEAN)

-include $(DEPS)
