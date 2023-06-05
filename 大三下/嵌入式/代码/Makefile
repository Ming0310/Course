.PHONY:all clean
ifneq ($(KERNELRELEASE),)
obj-m := mydev.o
else
EXTRA_CFLAGS += -DDEBUG
KDIR := /home/ming/linux-5.15.30
all:
	make  CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm -C $(KDIR) M=$(PWD) modules
clean:
	rm -fr *.ko *.o *.mod.o *.mod.c *.symvers *.order .*.ko .tmp_versions
endif