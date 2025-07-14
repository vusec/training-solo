import argparse
import pandas as pd
from itertools import combinations


def get_mask(start_bit, end_bit):
    return (((1 << (end_bit - start_bit + 1)) - 1) << (start_bit))

def get_mask_of_pc(tm: pd.Series, mask, bit_start):
    return (tm['pc'] & mask) >> bit_start

CACHE_LINE_MSB = 5
CACHE_LINE_MSB_MASK  = (1 << CACHE_LINE_MSB)

CACHE_LINE_MASK  = get_mask(0, CACHE_LINE_MSB)


# short bit range
SHORT_BRANCH_BITS = [9, 11]
MAX_SHORT_BRANCH_BITS = max(SHORT_BRANCH_BITS)

def load_file(filename):

    addresses = []

    with open(filename) as file:
        for line in file:
            value = line.rstrip().split()[0]
            addresses.append(int(value, 16))

    return addresses

def main(input, output):

    df = pd.read_csv(input, sep=";")
    df = df.drop_duplicates()

    # We only handle indirect branches
    df = df[df['target_type'] == 'indirect']

    # Transform columns.
    integer_cols = ['address', 'pc']

    for i in integer_cols:
        df[i] = df[i].fillna('0')
        df[i] = df[i].apply(int, base=16)


    df['cache_line_msb'] = df.apply(get_mask_of_pc, args=(CACHE_LINE_MSB_MASK, CACHE_LINE_MSB), axis=1)



    print(f"{'# IND branches:':30} {len(df)}")

    df = df[df['cache_line_msb'] == 0]

    print(f"{'# IND branches PC[5] == 0:':30} {len(df)}")

    targets = []


    for _, row in df.iterrows():
        pc = row['pc']

        # The direct branch has to be at the upper cache halve (PC[5] == 1)
        # thus, since we can only jump forward in cBPF, our possible targets
        # start at 0x21

        target = (pc & ~CACHE_LINE_MASK) + 0x21


        while(1):

            msb_different_bit = (pc ^ target).bit_length() - 1

            if msb_different_bit > MAX_SHORT_BRANCH_BITS:
                break

            targets.append({
                "ind_address" : row['address'],
                "ind_pc" : row['pc'],
                "target" : target,
                "msb_difference" : msb_different_bit,
                "offset" : target - row['pc'],
            })

            target += 1


    targets = pd.DataFrame(targets)


# -----------------------------------------------------------------------------
    print(f"{'Total targets:':30} {len(targets)}")
    print(f"{'Unique targets:':30} {targets['target'].nunique()}")

    for max_msb_dif in SHORT_BRANCH_BITS:
        print(f"{'Unique targets MSB dif <=' + str(max_msb_dif) + ':':30} {targets[targets['msb_difference'] <= max_msb_dif]['target'].nunique()}")


    # Transform columns.
    for i in ['ind_address', 'ind_pc', 'target']:
        targets[i] = targets[i].apply(hex)

    targets.to_csv(output, sep=';', index=False)


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('input')
    arg_parser.add_argument('output')

    args = arg_parser.parse_args()
    main(args.input, args.output)
