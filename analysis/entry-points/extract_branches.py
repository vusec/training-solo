import argparse
import string
import pandas as pd

dir_branches = ['ja', 'jb', 'jc', 'je', 'jg', 'jl', 'jn', 'jo', 'jp', 'js',
                'jz', 'jmp', 'call']
ind_branches = ['jmp', 'ljmp', 'call']

ignore = ['vmmcall', 'vmcall']

def extract_branches(filename):

    branch_info = []

    with open(filename) as file:
        for line in file:
            values = line.split()
            n_values = len(values)

            if n_values < 3:
                continue

            address = f"0x{values[0].removesuffix(':')}"

            try:
                int(address, 16)
            except ValueError:
                # line = line.strip()
                # if "format" in line or "section" in line:
                #     print("Error, invalid line:", line)
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

            if mnemonic in ignore:
                continue

            is_indirect = False
            if "*" in line:
                is_indirect = True

            if (is_indirect and mnemonic not in ind_branches):
                print("Unknown indirect mnemonic:", line.strip())
                continue

            if not is_indirect and not any([mnemonic.startswith(x) for x in dir_branches]):
                # Just a normal instruction
                continue

            operands = ' '.join(values[instruction_length + 2: ])


            target = ""
            if not is_indirect:
                target = values[instruction_length + 2]


            # print(line.strip(), f"Target {target}")


            info = {
                "address"  : address,
                "pc" : hex(int(address, 16) + instruction_length - 1),
                "ins_length" : instruction_length,
                "branch_type" : "call" if "call" in mnemonic else "jump",
                "target_type" : "indirect" if is_indirect else "direct",
                "target": target,
                "mnemonic" : mnemonic,
                "operands" : operands,

            }

            branch_info.append(info)

    return pd.DataFrame.from_dict(branch_info, orient='columns')



def main(input_file, output_file):


    df = extract_branches(input_file)

    df.to_csv(output_file, sep=';', index=False)



if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('input')
    arg_parser.add_argument('output')

    args = arg_parser.parse_args()
    main(args.input, args.output)
