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
RUN pip3 install qemu qemu.qmp pandas ipython tqdm capstone lief pyelftools

# -----------------------------------------------------------------------------
# Syzkaller Setup
# -----------------------------------------------------------------------------

RUN wget https://go.dev/dl/go1.22.5.linux-amd64.tar.gz
RUN tar -C /usr/local -xzf go1.22.5.linux-amd64.tar.gz; export PATH=$PATH:/usr/local/go/bin
ENV PATH /usr/local/go/bin:$PATH

RUN git clone https://github.com/google/syzkaller.git --depth 1
WORKDIR /syzkaller
RUN make -j`nproc`

# -----------------------------------------------------------------------------
# VM Setup
# -----------------------------------------------------------------------------

WORKDIR /vm
COPY vm/create-image.sh /vm

# Reuse syzkaller scripts to easily create a rootfs.
# -s 32 GB
RUN ./create-image.sh --feature full --distribution bookworm -s 16384

COPY vm /vm
COPY scripts /scripts
COPY entry-points /entry-points
COPY collisions-calculator/scripts/. /scripts/.


