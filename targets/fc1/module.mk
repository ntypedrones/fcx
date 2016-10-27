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

MIPS_MTI_ELF_ROOT ?= /opt/imgtec/Toolchains/mips-mti-elf/2016.05-03
# Overrides the system value that is set to the img-elf toolchain by default
MIPS_ELF_ROOT := $(MIPS_MTI_ELF_ROOT)

ROMABLE := 1

DEFINES += SKIP_COPY_TO_RAM

ABI := 32
APP_START := 0x9d001000
ISR_VECTOR_COUNT := 1
MEMORY_SIZE := 32K
LDSCRIPT := targets/fc1/pic32mx_uhi.ld
include ${MIPS_ELF_ROOT}/share/mips/rules/mipshal.mk

include common/sensors/sbus/module.mk
include common/sensors/mpu6050/module.mk

CFLAGS += -march=m4k -EL -msoft-float
LDFLAGS += -march=m4k -EL -msoft-float -Wl,--defsym,__use_excpt_boot=0
LDFLAGS += -Wl,-Map,output.map

IDIRS += targets/fc1
SRCS +=	targets/fc1/target.c
SRCS +=	targets/fc1/i2c.c
SRCS +=	targets/fc1/input.c
SRCS +=	targets/fc1/pwm.c
SRCS +=	targets/fc1/p32mx250f128b.S
SRCS +=	targets/fc1/excpt_isr.S
SRCS +=	targets/fc1/reset_mod.S

OBJCOPY_TO_PHY := --change-section-lma .bootflash-0xa0000000
OBJCOPY_TO_PHY += --change-section-lma .exception_vector-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .text-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .init-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .fini-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .eh_frame-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .jcr-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .ctors-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .dtors-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .rodata-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .data-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .bss-0x80000000
OBJCOPY_TO_PHY += --change-section-lma .startdata-0x80000000

all:
	$(OBJCOPY) $(OBJCOPY_TO_PHY) $(TARGET) $(TARGET:%.elf=%_physical.elf)
	objcopy -O ihex $(TARGET:%.elf=%_physical.elf) $(TARGET:%.elf=%_pickit.hex)

RM_ON_CLEAN += $(TARGET:%.elf=%_physical.elf) $(TARGET:%.elf=%_pickit.hex)
RM_ON_CLEAN += output.map
