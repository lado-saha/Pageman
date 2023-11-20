# A simple Makefile to compile the module 

obj-m += pageman.o 
all:
	# This builds the module and load it into the directory for kernel modules 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	# Delete all build files from the kernel modules directory 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

