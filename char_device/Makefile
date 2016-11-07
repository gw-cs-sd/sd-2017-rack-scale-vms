obj-m :=chr_drv_udev.o

KDIR= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.o *~ core .depend *.mod.o .*.cmd *.ko *.mod.c \
		.tmp_versions *.markers *.symvers modules.order
