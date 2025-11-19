KDIR = /linux 
kbuild:
	make -C $(KDIR) M=$(shell pwd)
clean:
	make -C $(KDIR) M=$(shell pwd) clean 
