#!/usr/bin/env python3

import sys
import string

for line in sys.stdin:

    values = line.split()
    n_values = len(values)

    if n_values < 3:
        continue

    address = f"0x{values[0].removesuffix(':')}"

    try:
        int(address, 16)
    except ValueError:
        print("Error, invalid line:", line.strip())
        continue

    instruction_length = 0

    for x in values[1:]:
        if all(c in string.hexdigits for c in x):
            instruction_length += 1
        else:
            break

    if n_values < instruction_length + 2:
        # No mnemonic
        continue

    mnemonic = values[instruction_length + 1]

    print(f"{address},{instruction_length}")
