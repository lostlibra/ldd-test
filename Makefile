KVERS = $(shell uname -r)

# Kernel modules
obj-m += zzw_platform_driver.o
obj-m += zzw_platform_device.o

# Specify flags for the module compilation.
#EXTRA_CFLAGS=-g -O0

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean
