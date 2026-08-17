# Makefile for the tiny RISC-V bare-metal shell
# Toolchain installed via: brew install riscv64-elf-gcc
# Emulator installed via:  brew install qemu

CROSS   = riscv64-elf-
CC      = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy

CFLAGS  = -nostdlib -nostartfiles -ffreestanding -O2 -Wall -g -mcmodel=medany
LDFLAGS = -T linker.ld -nostdlib -ffreestanding

QEMU    = qemu-system-riscv64

OBJS = boot.o uart.o shell.o

all: shell.elf

boot.o: boot.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c uart.h
	$(CC) $(CFLAGS) -c $< -o $@

shell.elf: $(OBJS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)

run: shell.elf
	$(QEMU) -M virt -nographic -bios none -kernel shell.elf

clean:
	rm -f *.o shell.elf

.PHONY: all run clean
