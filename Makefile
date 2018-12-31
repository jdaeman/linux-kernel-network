#object
obj-m := lookpkt.o

#get sys call table address
SYS_CALL_TABLE = 0x$(subst R sys_call_table,,$(shell grep sys_call_table /boot/System.map-$(shell uname -r)))

#when code is complied, add this statment -> #define SYS_CALL_TABLE = 0x"sys_call_table address"
CFLAGS_lookpkt.o += -DSYS_CALL_TABLE=$(SYS_CALL_TABLE)

#kernel directory path
KDIR := /lib/modules/$(shell uname -r)/build

#current directory path
PWD := $(shell pwd) #current directory

#kernel module compile
default : 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

#kernel module file remove
clean :
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean

#kernel module insert
insert :
	sudo insmod lookpkt.ko

#kernel module remove
remove :
	sudo rmmod lookpkt

#compile test program
test :
	gcc example.c
