obj-m += its_vmm_host_module.o

its_vmm_host_module-objs := its_host_helper.o snippets/snippet.o


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
