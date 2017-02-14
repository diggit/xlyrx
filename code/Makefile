# Copyright (c) 2017 Patrik Bachan
#
# GNU GENERAL PUBLIC LICENSE
#    Version 3, 29 June 2007
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

TOOLCHAIN		= arm-none-eabi
export CC		= $(TOOLCHAIN)-gcc
LD				= $(CC)
OBJCOPY			= $(TOOLCHAIN)-objcopy
SIZE			= $(TOOLCHAIN)-size
OPTIMIZE		= 2
LSCRIPT			= stm32f103.ld
JOBS			= 9

SIZE_HISTORY	= size.log
BUILD_NUM		= $(shell wc -l $(SIZE_HISTORY)|cut -d " " -f1)

export CFLAGS		= -D'BUILD=$(BUILD_NUM)' -c -Wunused -Wall -Wstrict-prototypes -Wundef -Werror -pedantic -g -O$(OPTIMIZE) -I./ -mcpu=cortex-m3 -mthumb -flto -funsigned-char -funsigned-bitfields -fpack-struct -fdata-sections -ffunction-sections -fdiagnostics-color=always -std=gnu99
# -mcall-prologues
LDFLAGS			= -mcpu=cortex-m3 -mthumb -O$(OPTIMIZE) -nostartfiles -flto -fdiagnostics-color=always -Wl,--gc-sections -T$(LSCRIPT)
TARGET			= main


export OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))

.NOTPARALLEL:

all: $(TARGET).hex

$(TARGET): clean compile_objects
	@echo "---------------------LINK-------------------------"
	@echo "BUILD $(BUILD_NUM)"
	$(LD) -o $@.elf $(OBJECTS) $(LDFLAGS)
	@$(SIZE) -B -d $(TARGET).elf | tail -n 1 >> $(SIZE_HISTORY)

.PHONY: compile_objects
compile_objects:
	@echo "--------------------COMPILE-----------------------"
	@$(MAKE) -j $(JOBS) -f Makefile.compile


%.hex: $(TARGET) size
	@echo "---------------------HEX--------------------------"
	$(OBJCOPY) -Oihex  $<.elf $@

.PHONY: clean
clean:
	rm -rf *.o $(TARGET).elf $(TARGET).hex

.PHONY: size
size: $(TARGET)
	@echo "---------------------SIZE-------------------------"
	@$(SIZE) -B -d $(TARGET).elf

.PHONY: burn
burn: $(TARGET).hex
	@echo "---------------------BURN-------------------------"
	openocd -f stm32f1xx_ftdi_swd.cfg -c "init; reset halt; flash write_image erase $<; reset run; exit"

.PHONY: reset
reset: $(TARGET).hex
	openocd -f stm32f1xx_ftdi_swd.cfg -c "init; reset run; exit"

.PHONY: debug
debug: $(TARGET).hex
	openocd -f stm32f1xx_ftdi_swd.cfg -c "init; reset halt; flash write_image erase $<"&
	arm-none-eabi-gdb main.elf -ex "target extended-remote localhost:3333" -ex "monitor reset halt"
	echo "shutdown" | nc localhost 4444


.PHONY: peek
peek: $(TARGET).hex
	openocd -f stm32f1xx_ftdi_swd.cfg -c "init"&
	arm-none-eabi-gdb main.elf -ex "target extended-remote localhost:3333"
	echo "shutdown" | nc localhost 4444
