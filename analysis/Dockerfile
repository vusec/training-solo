#
# Prepare the environment for eval.
#
FROM ubuntu:22.04 AS base

# -----------------------------------------------------------------------------
# Packages
# -----------------------------------------------------------------------------
RUN apt update && \
    apt install -y build-essential libncurses-dev bison flex libssl-dev \
    libelf-dev zstd qemu-system-x86 debootstrap wget bc sudo python3-pip \
    openssh-client git sqlite3 vim linux-headers-$(uname -r) gdb

# Python dependencies
RUN pip3 install qemu qemu.qmp pandas ipython tqdm

# -----------------------------------------------------------------------------
# VM Setup
# -----------------------------------------------------------------------------

COPY vm /vm
WORKDIR /vm
# Reuse syzkaller scripts to easily create a rootfs.
RUN ./create-image.sh -d bullseye

# -----------------------------------------------------------------------------
# Copy local directories
# -----------------------------------------------------------------------------
COPY /inspectre-gadget/requirements.txt requirements.txt
RUN pip3 install -r requirements.txt

# Copy required folders
COPY scanner /scanner
COPY scripts /scripts
COPY analysis /analysis
COPY entry-points /entry-points
COPY inspectre-gadget /inspectre-gadget


