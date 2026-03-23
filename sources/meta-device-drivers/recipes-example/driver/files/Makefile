obj-m := firstModule.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c *.mod *.mod.o
	rm -f Module.markers Module.symvers modules.order Modules.symvers
	rm -rf .tmp_versions
