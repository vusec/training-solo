obj-m += execute_caller_vm_module.o

execute_caller_vm_module-objs := execute_caller.o leak.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
