#!/bin/bash
# -----------------------------------------------------------------------------
# Create memory dump of kernel image
# -----------------------------------------------------------------------------

set -e

DIR_WORK="${DIR_WORK:-/work-dir}"

if [ ! -f $DIR_WORK/images/dump_vmlinux ]; then

    cd /vm

    # Finish setting up the rootfs (from syzkaller create-img script).
    echo "[+] Mouting rootfs"
    if [ ! -f bullseye.img ]; then
        echo "[+] Created symlink to bullseye.img -> bookworm.img"
        ln -s bookworm.img bullseye.img
    fi

    mount -o loop bullseye.img /mnt/chroot

    cp -a chroot/. /mnt/chroot/.
    umount /mnt/chroot

    # Create dump
    ./run-vm.sh $DIR_WORK/images/vmlinuz 2>&1 > /dev/null &
    sleep 10 && python3 dump-memory.py dump_vmlinux
    mv dump_vmlinux $DIR_WORK/images/dump_vmlinux

else
    echo "[+] [CACHED] Resuing vmlinux memory dump from work directory"
fi
