obj-m += pageman.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	sudo insmod pageman.ko

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
