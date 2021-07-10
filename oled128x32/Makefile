KERNELDIR :=/home/zyy/zyy/px30-linux-sync/kernel

CURRENT_PATH := $(shell pwd)

obj-m := oled.o

build : kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
