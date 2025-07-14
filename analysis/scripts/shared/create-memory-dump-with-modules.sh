#!/bin/bash
# -----------------------------------------------------------------------------
# Create memory dump of kernel image
# -----------------------------------------------------------------------------

set -e

N_SNAPSHOTS=99
DIR_WORK="${DIR_WORK:-/work-dir}"

if [ ! -f $DIR_WORK/images/snapshot_0${N_SNAPSHOTS}/text_region.txt ]; then

    cd /vm

    # Finish setting up the rootfs (from syzkaller create-img script).
    if [ ! -f bullseye.img ]; then
        echo "[+] Created symlink to bullseye.img -> bookworm.img"
        ln -s bookworm.img bullseye.img
    fi

    DISTRO_VERSION=jammy
    echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION} restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
    echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-updates restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
    echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-security restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
    echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu ${DISTRO_VERSION}-backports restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
    chroot chroot /bin/bash -c "apt update && apt -y install linux-modules-${LINUX_VERSION}-${LINUX_FLAVOUR}"
    chroot chroot /bin/bash -c "apt -y install linux-modules-extra-${LINUX_VERSION}-${LINUX_FLAVOUR}"

    sudo cp $DIR_WORK/images/loaded_modules_i9-13900K.txt chroot/root/

    echo "[+] Mouting rootfs"
    mount -o loop bullseye.img /mnt/chroot
    cp -a chroot/. /mnt/chroot/.
    umount /mnt/chroot

    for idx in $(seq -f "%03g" 0 $N_SNAPSHOTS)
    do
        SNAPSHOT_FOLDER=snapshot_$idx

        if [ ! -f $DIR_WORK/images/$SNAPSHOT_FOLDER/text_region.txt ]; then


            echo "[+] Creating memory dump for $SNAPSHOT_FOLDER"

            # Create dump
            ./run-vm-256mb.sh $DIR_WORK/images/vmlinuz 2>&1 > /dev/null &
            sleep 15

            echo "[+] Installing default modules bare-metal"
            ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "cat loaded_modules_i9-13900K.txt | xargs -L1 modprobe" || true

            echo "[+] Getting module addresses"

            ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "cat /proc/kallsyms | grep '_stext' " | sudo tee text_region.txt
            ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "cat /proc/kallsyms | grep '_etext' " | sudo tee -a text_region.txt
            ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "cat /proc/kallsyms" > kallsyms.txt

            ssh -i bookworm.id_rsa -p 7777 -o "StrictHostKeyChecking no" root@localhost -t "head /sys/module/*/sections/.*" > module_sections.txt.tmp
            cp module_sections.txt.tmp module_sections_original.txt

            cat module_sections.txt.tmp   | sed -e 's/==> //' -e 's/\r//' | sed -ze 's/ <==\n/ /g' | awk NF | awk -F'/' '{print $4 " " $0'} | awk '{print $3 " " $1 " " $2}' > module_sections.txt
            rm module_sections.txt.tmp

            cat module_sections.txt | awk '{print $2}' | sort -u > module_names.txt


            mkdir -p $DIR_WORK/images/$SNAPSHOT_FOLDER

            mv *.txt $DIR_WORK/images/$SNAPSHOT_FOLDER

            python3 dump-memory.py dump_vmlinux
            mv dump_vmlinux $DIR_WORK/images/$SNAPSHOT_FOLDER/dump_vmlinux_modules

        else
            echo "[+] [CACHED] Reusing memory dump for $SNAPSHOT_FOLDER"
        fi

    done

    echo "[+] Done with creating memory dumps!"

else
    echo "[+] [CACHED] Resuing vmlinux memory dump from work directory"
fi
