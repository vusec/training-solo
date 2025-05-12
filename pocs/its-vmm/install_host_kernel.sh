#!/bin/bash
set -e

# -- Download kernel source
# git clone --depth 1 --branch Ubuntu-6.8.0-38.38 git://git.launchpad.net/~ubuntu-kernel/ubuntu/+source/linux/+git/noble linux-6.8.0-38-generic

# Get default config
wget 'http://nl.archive.ubuntu.com/ubuntu/pool/main/l/linux/linux-headers-6.8.0-38-generic_6.8.0-38.38_amd64.deb' -O linux-headers-6.8.0-38-generic_6.8.0-38.38_amd64.deb
dpkg-deb --fsys-tarfile linux-headers-6.8.0-38-generic_6.8.0-38.38_amd64.deb | tar Ox --wildcards './usr/src/*/.config' > .config
mv .config linux-6.8.0-38-generic

# Apply patch
cd linux-6.8.0-38-generic
patch -p1 < ../kernel-vmm.patch

# -- Setup config

cd linux-6.8.0-38-generic

# Fix ubuntu errors
scripts/config -d CONFIG_SYSTEM_REVOCATION_KEYS -d CONFIG_SYSTEM_TRUSTED_KEYS

# Disable modules signing, slow and can cause errors
scripts/config -d SECURITY_LOCKDOWN_LSM -d MODULE_SIG -d MODULE_SIG_ALL

make olddefconfig

# # Build and install
make -j `nproc`
sudo make modules_install -j `nproc`
sudo make install -j `nproc`

echo "Please reboot with the just installed kernel (Linux 6.8.8+)"
