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


# create normal user user
# RUN sudo chroot chroot /bin/bash -c "adduser --gecos '' --disabled-password user"
# RUN sudo chroot chroot /bin/bash -c "chpasswd <<<'user:user'"
# RUN mkdir -p ./chroot/home/user/.ssh/
# RUN cat bookworm.id_rsa.pub | tee ./chroot/home/user/.ssh/authorized_keys

RUN cp /syzkaller/bin/linux_amd64/syz-execprog ./chroot/root/
RUN cp /syzkaller/bin/linux_amd64/syz-executor ./chroot/root/
COPY indirect-rec/scripts/start-record.sh ./chroot/root/
# RUN sudo chroot chroot /bin/bash -c "chown -R user: /home/user "


COPY vm /vm
COPY scripts /scripts
COPY entry-points /entry-points
COPY indirect-rec/scripts/. /scripts/.


