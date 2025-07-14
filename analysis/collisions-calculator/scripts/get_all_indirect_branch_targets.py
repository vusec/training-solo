import argparse
import re
import sys

FUNCTION_OFFSET_FROM_SID = 12

assembly_output = """
ffffffffc030807d:       0f 1f 40 00             nopl   0x0(%rax)
ffffffffc0308084:       0f 1f 00                nopl   (%rax)
ffffffffc030809c:       4c 89 ff                mov    %r15,%rdi
ffffffffc030809f:       be 7e 00 00 00          mov    $0x7e,%esi
ffffffffc03080a4:       41 ba fa 88 bb 75       mov    $0x75bb88fa,%r10d
ffffffffc083fe6d:       41 ba 80 cb f0 ff       mov    $0xfff0cb80,%r10d
ffffffffc083e532:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc083e532:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc092d3a4:       41 ba 00 10 00 00       mov    $0x1000,%r10d
ffffffffc0796880:       44 8b 54 24 20          mov    0x20(%rsp),%r10d
ffffffffc07babd3:       44 0f b6 56 05          movzbl 0x5(%rsi),%r10d
ffffffffc092b2d9:       44 0f b6 93 83 00 00    movzbl 0x83(%rbx),%r10d
"""

assembly_target = """
ffffffffc084c524:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc0844024:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc0844184:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc083e462:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc083e4b4:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc083e532:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc083e584:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc084c524:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc0844024:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc0844184:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc083e462:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc083e4b4:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
ffffffffc083e532:       41 ba 04 47 09 5d       mov    $0x5d094704,%r10d
ffffffffc083e584:       41 81 ea 04 47 09 5d    sub    $0x5d094704,%r10d
"""

def parse_kallsym_data(kallsym_data):
    # Dictionary to store parsed symbols by address
    symbols = {}

    pattern = re.compile(r'([0-9a-f]{16})\s+\w+\s+(\S+)(?:\s+\[(\S+)\])?')

    for line in kallsym_data.strip().splitlines():
        match = pattern.match(line)
        if match:
            address = int(match.group(1), 16)  # Convert hex address to an integer
            symbol = match.group(2)           # Symbol name
            module = match.group(3) or 'None'   # Module name (if any)

            symbols[address] = {"symbol": symbol, "module": module}

    return symbols

def main(source_asm_file, target_asm_file, kallsyms_file):

    with open(source_asm_file, 'r') as file:
        asm_source = file.read()

    with open(target_asm_file, 'r') as file:
        asm_target = file.read()

    with open(kallsyms_file, 'r') as file:
        symbol_info = parse_kallsym_data(file.read())

    # f = open(cfi_source_asm, "r")
    # print(f.read())

    sid_pattern = re.compile(r'mov\s+\$(0x[0-9a-fA-F]{8}),%r10d\b')
    sid_address_pattern_mov = re.compile(r'([0-9a-f]{16}):\s+.*?mov\s+\$(0x[0-9a-fA-F]{8}),%r10d\b')

    call_sids = [match for match in sid_pattern.findall(asm_source)]
    call_sids = set(call_sids)

    sid_to_callers = {}

    for match in sid_address_pattern_mov.findall(asm_source):
        sid_to_callers.setdefault(match[1], []).append(int(match[0], 16))

    print(len(call_sids), "Unique SIDS called", file=sys.stderr)
    print("", file=sys.stderr)

    sid_address_pattern_sub = re.compile(r'([0-9a-f]{16}):\s+.*?sub\s+\$(0x[0-9a-fA-F]{8}),%r10d\b')

    sid_target_entries = [{"address": match[0], "sid": match[1]} for match in sid_address_pattern_sub.findall(asm_target)]
    # sid_entries = [{"address": match[0], "sid": match[1]} for match in sid_address_pattern.findall(assembly_target)]

    target_sids = [entry['sid'] for entry in sid_target_entries]
    target_sids = set(target_sids)


    print(len(sid_target_entries), "Targets with SIDS", file=sys.stderr)
    print(len(target_sids), "Unique target SIDS", file=sys.stderr)
    print("", file=sys.stderr)


    sid_target_entries_filtered = [entry for entry in sid_target_entries if entry['sid'] in call_sids]
    # sid_target_entries_filtered = sid_target_entries

    target_sids_filtered = [entry['sid'] for entry in sid_target_entries_filtered]
    target_sids_filtered = set(target_sids_filtered)

    print(len(sid_target_entries_filtered), "Targets SIDS filtered", file=sys.stderr)
    print(len(target_sids_filtered), "Unique filtered target SIDS", file=sys.stderr)

    OUTPUT_CALLER_INFO = True

    if OUTPUT_CALLER_INFO:
        print("caller;target")
    else:
        print("address;name;module;sid")


    for entry in sid_target_entries_filtered:

        f_address = int(entry['address'], 16) + FUNCTION_OFFSET_FROM_SID
        sid = entry['sid']

        if f_address in symbol_info:
            name =  symbol_info[f_address]['symbol']
            module = symbol_info[f_address]['module']

            if module != 'None':
                name =  f'{module}-{name}'
        else:
            print("No symbol found!", f_address, file=sys.stderr)
            name = f_address
            module = 'None'

        if not OUTPUT_CALLER_INFO:
            print(f"{hex(f_address)};{name};{module};{sid}")
        else:
            for caller in sid_to_callers[sid]:
                print(f"{hex(caller)};{hex(f_address)}")


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('asm_callers_file')
    arg_parser.add_argument('asm_targets_file')
    arg_parser.add_argument('kallsyms')

    args = arg_parser.parse_args()
    main(args.asm_callers_file, args.asm_targets_file, args.kallsyms)
