#!/bin/sh
KERN_IMAGE=$1
KERN_IMAGE="${KERN_IMAGE:=vmlinuz}"
KERN_RFS="./bookworm.img"
KERN_FLAGS="console=ttyS0 root=/dev/sda earlyprintk=serial net.ifnames=0" # nospectre_v1 spectre_v2=retpoline spec_rstack_overflow=off"


taskset -c 2 qemu-system-x86_64 \
	-m 8G \
	-smp 1 \
	-kernel $KERN_IMAGE \
  	-append "$KERN_FLAGS" \
	-drive file=$KERN_RFS,format=raw \
  	-net user,hostfwd=tcp::7777-:22 -net nic \
  	-netdev tap,id=tap0 -device e1000,netdev=tap0 \
	-enable-kvm \
  	-cpu host \
	-nographic -s

