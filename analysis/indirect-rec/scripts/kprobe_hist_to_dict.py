#!/usr/bin/env python3
import argparse
import json


def main(hist_file, output_file):

    ind_branches = {}

    with open(hist_file) as f:
        for line in f:
            # { __probe_ip: ffffffff81001969, target: ffffffffc09740e0 }
            values = line.split()

            try:
                ind_address = int(values[2].removesuffix(','), 16)
                target =  int(values[4].removeprefix(','), 16)
            except ValueError as e:
                print("ERROR: Invalid line: ", line.strip())
                print(e)

            ind_branches.setdefault(hex(ind_address), []).append(hex(target))

    print("Number of indirect branch targets:", len(ind_branches))
    targets = [x for targets in ind_branches.values() for x in targets]
    print("Number of targets:", len(targets))
    print("Number of unique targets:", len(set(targets)))


    print("Distribution of number targets {#targets: #ind_branches,} :", len(targets))
    number_of_targets = sorted([len(x) for x in ind_branches.values()])

    print({i : number_of_targets.count(i) for i in number_of_targets})


    # for k, v in ind_branches.items():
    #     if len(v) > 100:
    #         print(k)

    with open(output_file, 'w') as file:
        file.write(json.dumps(ind_branches, indent=4))


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser(description='Convert kprobe hist to dict -> {indirect branch : [list of targets],}')
    arg_parser.add_argument('hist_file')
    arg_parser.add_argument('output_file')



    args = arg_parser.parse_args()

    main(args.hist_file, args.output_file)
