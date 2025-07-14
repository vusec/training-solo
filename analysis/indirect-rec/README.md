# Indirect Recorder

Simple indirect branch recorder for the Linux Kernel. We use kprobes
to log all indirect branch and return targets while we are executing
Syzkaller test cases.
