KVERS = $(shell uname -r)
#KVERS = /work/linux-kernel/linux-2.6.32.16/
#KDIR ?= /lib/modules/$(KVERS)/build
KDIR ?= /work/linux-kernel/linux-2.6.32.2

ARCH ?= arm
CROSS_COMPILE ?= arm-linux-

# Kernel modules
obj-m += button_input_dev.o

# Specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0

build: kernel_modules

kernel_modules:
	make -C $(KDIR) M=$(CURDIR) modules

clean:
	make -C $(KDIR) M=$(CURDIR) clean
