#!/usr/bin/env python3
import sys

# address;pc;ins_length;branch_type;target_type;target;mnemonic;operands
IDX_ADDRESS = 0
IDX_BRANCH_TYPE = 3
IDX_TARGET_TYPE = 4
IDX_OPERANDS = 7

x86_registers =  {'rax' : 'ax', 'rbx' : 'bx', 'rcx' : 'cx', 'rdx' : 'dx', 'rsi' : 'si',
             'rdi' : 'di', 'r8' : 'r8' , 'r9' : 'r9',
             'r10' : 'r10', 'r11' : 'r11', 'r12' : 'r12',
             'r13' : 'r13', 'r14' : 'r14', 'r15' : 'r15'}


def output_kprobe_for_branch(line):

    # address;pc;ins_length;branch_type;target_type;target;mnemonic;operands

    line = line.strip()
    values = line.split(';')

    if len(values) != 8:
        print("Invalid line: ", line, file=sys.stderr)
        return

    if values[IDX_TARGET_TYPE] != 'indirect':
        return

    operand = values[IDX_OPERANDS]

    if not operand.startswith('*%'):
        print(f"Invalid operand: {operand} ({line})", file=sys.stderr)
        return

    register = operand.removeprefix('*%')

    if register not in x86_registers.keys():
        print(f"Invalid register: {register} ({line})", file=sys.stderr)
        return

    address = values[IDX_ADDRESS]

    print(f"p:ind_{address.removeprefix('0x')} {address} target=%{x86_registers[register]}:x64")

try:
    for line in sys.stdin:
        output_kprobe_for_branch(line)
except (BrokenPipeError, IOError):
    pass
