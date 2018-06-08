obj-m := test.o  
KERNELDR := /usr/src/kernels/3.9.10-100.fc17.x86_64
PWD := $(shell pwd)  
modules:  
	$(MAKE) -C $(KERNELDR) M=$(PWD) modules  
moduels_install:  
	$(MAKE) -C $(KERNELDR) M=$(PWD) modules_install  
clean:  
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions modules.order  Module.symvers 

