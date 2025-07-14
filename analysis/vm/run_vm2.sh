#!/bin/sh
KERN_IMAGE=$1
KERN_RFS="./bookworm.img"
KERN_FLAGS="console=ttyS0 root=/dev/sda earlyprintk=serial net.ifnames=0 nokaslr mitigations=off" # nospectre_v1 spectre_v2=retpoline spec_rstack_overflow=off"


qemu-system-x86_64 \
	-m 32G \
	-smp `nproc` \
	-kernel $KERN_IMAGE \
  -append "$KERN_FLAGS" \
	-drive file=$KERN_RFS,format=raw \
  -net user,hostfwd=tcp::7777-:22 -net nic \
  -netdev tap,id=tap0 -device e1000,netdev=tap0 \
	-enable-kvm \
  -cpu host \
	-nographic \
	-pidfile vm.pid


  # -net user,host=10.0.2.10,hostfwd=tcp:127.0.0.1:10021-:22 \
	# -net nic,model=e1000 \


# taskset -c 0 qemu-system-x86_64 \
#   -kernel $KERN_IMAGE \
#   -drive file=$KERN_RFS,index=0,media=disk,format=raw \
#   -nographic \
#   -append "$KERN_FLAGS" \
#   -m 4096 \
#   -smp 1 \
#   --enable-kvm \
#   -cpu host \
#   -s

  # -trace enable=exec_tb,file=trace.out \
  # -d cpu,nochain -D trace.out\
  # -trace enable=exec_tb,file=trace.out \
  # -d nochain,trace:exec_tb -D trace.out\

  #   -singlestep
  # trace-event exec_tb on
  # info trace-events exec_tb

