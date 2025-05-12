# Indirect Target Selection VMM Tests

This folder contains the tests and PoC for the Indirect Target Selection (ITS)
vulnerability in VMM (guest-hypervisor) scenario.
We create a custom VMM call in the host kernel, which is directed to
our host kernel module which contains the targeted victim indirect branch for the VMM scenario.
In the guest vm we run our user program which, in turn, may execute the installed
guest kernel module to test in kernel or perform a VMM call.

With these test we can train in either USER or KERNEL and place our victim in
USER, KERNEL or VMM. The leakage rate is printed if the victim is the VMM.
The training/testing in kernel/user can also be executed outside the VM.

## Requirements

```sh
sudo apt-get install build-essential libncurses-dev bison flex libssl-dev libelf-dev zstd qemu-system-x86 debootstrap wget
```

## Build custom kernel

Our test requires a patch for the host kernel which creates
a custom VMM call. We use version 6.8.0-38-generic. Please build
the kernel and reboot the host with this new installed kernel.

```sh
./install_host_kernel.sh
```

## Setup guest (VM)

We use the [create-image.sh](https://github.com/google/syzkaller/blob/master/tools/create-image.sh)
script from syzkaller to setup a minimal debian image.
Run the setup script `setup-vm.sh` and copy the code to the image by executing
`update-img.sh`.

```sh
cd vm
./setup-vm.sh
./update-image.sh
```

## Execute tests

Install the kernel module on the host kernel:

```sh
cd host-kernel
sudo ./run.sh
```

Start the VM and run the tests.

```sh
cd vm
./run-vm.sh
# Login with user root (no password needed)
root
# Install kernel modules
cd its-vmm
./setup.sh
# Run all tests
ARCH=INTEL_10_GEN ./run_all.sh
```

You can also run a single test, choose between `TEST_USER_USER`,
`TEST_USER_KERNEL`, `TEST_USER_VMM`, `TEST_KERNEL_KERNEL`, `TEST_KERNEL_VMM`.

```sh
ARCH=INTEL_10_GEN TEST=TEST_KERNEL_VMM ./run.sh
```

If needed you can also ssh into the VMM (better terminal):

```sh
cd vm
ssh -i bookworm.id_rsa -p 7777 root@localhost
```

## Results

We ran the test on Intel CPUs i9-9900k, i7-10700k, and i7-11700.
On the i7-10700k the VMM isolation is broken, an attacker can train
from both guest-user as guest-kernel. Although, for training from
guest-user we observed a less stable and significant lower hit-rate
then training from guest-kernel.

A reference run for TEST_KERNEL_VMM can be found in `output.ref`.
