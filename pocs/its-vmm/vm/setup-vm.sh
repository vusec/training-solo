#!/bin/bash
set -e

wget https://raw.githubusercontent.com/google/syzkaller/32d786e786e2caf2ba9704bf55562e65b1a4e70c/tools/create-image.sh -O create-image.sh
chmod +x create-image.sh
./create-image.sh -d bookworm -f full
mv bookworm chroot


# apt download linux-image-6.8.0-38-generic --print-uris
wget http://archive.ubuntu.com/ubuntu/pool/main/l/linux-signed/linux-image-6.8.0-38-generic_6.8.0-38.38_amd64.deb -O linux-image-6.8.0-38-generic_6.8.0-38.38_amd64.deb

# Extract vmlinuz
dpkg-deb --fsys-tarfile linux-image-6.8.0-38-generic_6.8.0-38.38_amd64.deb | tar Ox --wildcards  './boot/vmlinuz-*' > vmlinuz
# /usr/src/linux-headers-$(uname -r)/scripts/extract-vmlinux vmlinuz > vmlinux # optional: also extract vmlinux


# We now want to install the kernel modules and headers in the chroot.
# The next approach is a bit crude, but it works best to avoid dependency problems

echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu noble restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu noble-updates restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu noble-security restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
echo "deb [trusted=yes] http://nl.archive.ubuntu.com/ubuntu noble-backports restricted main universe multiverse" | sudo tee -a chroot/etc/apt/sources.list
sudo chroot chroot /bin/bash -c "apt update && apt -y install gcc-13 linux-modules-6.8.0-38-generic linux-headers-6.8.0-38-generic"
