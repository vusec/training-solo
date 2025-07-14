#!/bin/bash
set -e

export DIR_WORK=/work-dir

# -----------------------------------------------------------------------------
# 1: Retreive the linux images
# -----------------------------------------------------------------------------

/scripts/shared/get-linux-images.sh

# -----------------------------------------------------------------------------
# 2: Create memory dump of kernel image
# -----------------------------------------------------------------------------

/scripts/shared/create-memory-dump.sh

# -----------------------------------------------------------------------------
# 3: Create kprobe_events
# -----------------------------------------------------------------------------

mkdir -p $DIR_WORK/entry-points

if [ ! -f $DIR_WORK/entry-points/vmlinux_asm ] || [ ! -f $DIR_WORK/entry-points/branches.csv ]; then
    cd $DIR_WORK/entry-points


    START_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".text" | awk '{print "0x"$5'}`
    END_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".rodata" | awk '{print "0x"$5'}`

    objdump -D $DIR_WORK/images/dump_vmlinux --start-address=$START_ADDRESS --stop-address=$END_ADDRESS > vmlinux_asm

    # Extract branches from assembly output
    python3 /entry-points/extract_branches.py vmlinux_asm branches.csv

else
    echo "[+] [CACHED] Resuing entry-point lists from work directory"
fi

# -----------------------------------------------------------------------------
# 1: Download syzkaller corpus
# -----------------------------------------------------------------------------

mkdir -p $DIR_WORK/indirect-recorder


if [ ! -f $DIR_WORK/indirect-recorder/ci-upstream-kasan-gce-root-corpus.db ] || [ ! -f $DIR_WORK/indirect-recorder/kprobe_events ]; then
    echo "[+] Downloading syzkaler corpus"

    cd $DIR_WORK/indirect-recorder

    echo "[+] Generating kprobe_events file"

    objdump -d ../images/vmlinux --no-show-raw-insn | grep "cs call" | awk '{print $1}' | tr -d ':' > cs_call.txt

    grep indirect $DIR_WORK/entry-points/branches.csv | grep -v -f cs_call.txt |  /scripts/create_kprobes.py > kprobe_events
    grep indirect $DIR_WORK/entry-points/branches.csv | grep -v -f cs_call.txt |  /scripts/create_kprobes_ret.py > kprobe_events_ret

    echo "[+] Downloading syzkaler corpus"

    wget https://storage.googleapis.com/syzkaller/corpus/ci-upstream-kasan-gce-root-corpus.db

else
    echo "[+] [CACHED] Resuing syzkaller corpus from work directory"
fi

# -----------------------------------------------------------------------------
# 3: Install the modules inside vm chroot
# -----------------------------------------------------------------------------
cd /vm

cp $DIR_WORK/indirect-recorder/* ./chroot/root

# Not elegant at all, but for now an easy fix to get the modules installed
DISTRO_VERSION=jammy
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION} restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-updates restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-security restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-backports restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
chroot chroot /bin/bash -c "apt update && apt -y install linux-modules-${LINUX_VERSION}-${LINUX_FLAVOUR}"

# chroot chroot /bin/bash -c "cd /root; curl -o modules-${LINUX_VERSION}.deb ${LINUX_MODULES_URL}; dpkg -i modules-${LINUX_VERSION}.deb"

echo "[+] Mouting rootfs"
mount -o loop bookworm.img /mnt/chroot
cp -a chroot/. /mnt/chroot/.
umount /mnt/chroot

# -----------------------------------------------------------------------------
# 1: Run corpus and enable kprobe
# -----------------------------------------------------------------------------
cd /vm

echo "[+] Starting VM"
./run_vm2.sh $DIR_WORK/images/vmlinuz 2>&1 > /dev/null &

sleep 15s

echo "[+] Starting recording in VM"
ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "./start-record.sh"

scp -i bookworm.id_rsa -P 7777 -o "StrictHostKeyChecking no" root@localhost:/root/kprobe_hist $DIR_WORK/indirect-recorder/
scp -i bookworm.id_rsa -P 7777 -o "StrictHostKeyChecking no" root@localhost:/root/kprobe_hist_full $DIR_WORK/indirect-recorder/
scp -i bookworm.id_rsa -P 7777 -o "StrictHostKeyChecking no" root@localhost:/root/kprobe_hist_ret $DIR_WORK/indirect-recorder/

python3 /scripts/kprobe_hist_to_dict.py $DIR_WORK/indirect-recorder/kprobe_hist $DIR_WORK/indirect-recorder/indirect_branch_targets.json


