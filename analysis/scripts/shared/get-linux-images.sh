#!/bin/bash
# -----------------------------------------------------------------------------
# Retreive the linux images
# -----------------------------------------------------------------------------

set -e

DIR_WORK="${DIR_WORK:-/work-dir}"

mkdir -p $DIR_WORK/images

if [ ! -f $DIR_WORK/images/vmlinuz ] || [ ! -f $DIR_WORK/images/vmlinux ] || [ ! -f $DIR_WORK/images/vmlinux-dbg ]; then

    cd $DIR_WORK/images

    echo $LINUX_DEB_URL > url.txt
    echo $LINUX_DDEB_URL >> url.txt

    # Download and extract vmlinuz and vmlinuz file
    wget $LINUX_DEB_URL -O linux-image.deb
    dpkg-deb --fsys-tarfile linux-image.deb | tar Ox --wildcards  './boot/vmlinuz-*' > vmlinuz
    /usr/src/linux-headers-$(uname -r)/scripts/extract-vmlinux vmlinuz > vmlinux

    # Download and extract vmlinux debug file
    wget $LINUX_DDEB_URL -O linux-image-dbgsym.ddeb
    dpkg-deb --fsys-tarfile linux-image-dbgsym.ddeb | tar Ox --wildcards  './usr/lib/debug/boot/vmlinux-*' > vmlinux-dbg
else
    echo "[+] [CACHED] Resuing linux images from work directory"
fi
