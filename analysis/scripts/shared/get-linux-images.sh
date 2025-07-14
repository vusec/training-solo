#!/bin/bash
# -----------------------------------------------------------------------------
# Retreive the linux images
# -----------------------------------------------------------------------------

set -e

DIR_WORK="${DIR_WORK:-/work-dir}"

# http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux/?C=M;O=D
# http://ddebs.ubuntu.com/pool/main/l/linux/?C=M;O=D
# LINUX_DEB_URL=http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux/linux-image-unsigned-6.5.0-26-generic_6.5.0-26.26_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux/linux-image-unsigned-6.5.0-26-generic-dbgsym_6.5.0-26.26_amd64.ddeb

# LINUX_DEB_URL=http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux/linux-image-unsigned-6.5.0-33-generic_6.5.0-33.33_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux/linux-image-unsigned-6.5.0-33-generic-dbgsym_6.5.0-33.33_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-35-generic_6.5.0-35.35~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-35-generic-dbgsym_6.5.0-35.35~22.04.1_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-28-generic_6.5.0-28.29~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-28-generic-dbgsym_6.5.0-28.29~22.04.1_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-27-generic_6.5.0-27.28~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-27-generic-dbgsym_6.5.0-27.28~22.04.1_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-26-generic_6.5.0-26.26~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-26-generic-dbgsym_6.5.0-26.26~22.04.1_amd64.ddeb



# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-25-generic_6.5.0-25.25~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-25-generic-dbgsym_6.5.0-25.25~22.04.1_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.2/linux-image-6.2.0-37-generic_6.2.0-37.38~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.2/linux-image-unsigned-6.2.0-37-generic-dbgsym_6.2.0-37.38~22.04.1_amd64.ddeb

#LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.8/linux-image-6.8.0-38-generic_6.8.0-38.38~22.04.1_amd64.deb
#LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.8/linux-image-unsigned-6.8.0-38-generic-dbgsym_6.8.0-38.38~22.04.1_amd64.ddeb

# NOT ANALYZED YET:
#LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-14-generic_6.5.0-14.14~22.04.1_amd64.deb
#LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-14-generic-dbgsym_6.5.0-14.14~22.04.1_amd64.ddeb

#LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.2/linux-image-6.2.0-39-generic_6.2.0-39.40~22.04.1_amd64.deb
#LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.2/linux-image-unsigned-6.2.0-39-generic-dbgsym_6.2.0-39.40~22.04.1_amd64.ddeb


#LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.2/linux-image-6.2.0-31-generic_6.2.0-31.31~22.04.1_amd64.deb
#LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.2/linux-image-unsigned-6.2.0-31-generic-dbgsym_6.2.0-31.31~22.04.1_amd64.ddeb

#LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-6.5/linux-image-6.5.0-44-generic_6.5.0-44.44~22.04.1_amd64.deb
#LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-6.5/linux-image-unsigned-6.5.0-44-generic-dbgsym_6.5.0-44.44~22.04.1_amd64.ddeb

# LINUX_DEB_URL=http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed-hwe-5.19/linux-image-5.19.0-46-generic_5.19.0-46.47~22.04.1_amd64.deb
# LINUX_DDEB_URL=http://ddebs.ubuntu.com/pool/main/l/linux-hwe-5.19/linux-image-unsigned-5.19.0-46-generic-dbgsym_5.19.0-46.47~22.04.1_amd64.ddeb

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
