obj-m += pageman.o
TEST_PATH := /home/sih/linux-6.5.3/mm/page_man/ch8/page_exact_loop/
KDIR := /lib/modules/$(shell uname -r)/build

all:
	# sudo insmod pageman.ko
	make -C $(KDIR) M=$(PWD) modules

start_test:
	make -C $(KDIR) M=$(TEST_PATH)
	sudo insmod $(TEST_PATH)*.ko
	sudo dmesg 

stop_test: 
	sudo rmmod $(TEST_PATH)*.ko
	make -C $(KDIR) M=$(TEST_PATH) clean
	sudo dmesg

clean:
	make -C $(KDIR) M=$(PWD) clean
