import argparse
import pandas as pd

PAGE_BITS = 12
PAGE_MASK = (1 << PAGE_BITS) - 1

def get_x86_registers():
    return  ["rax", "rbx", "rcx", "rdx", "rsi",
             "rdi", "rbp", "rsp", "r8" , "r9",
             "r10", "r11", "r12", "r13", "r14", "r15"]

integer_cols = []

for reg in get_x86_registers():
    for with_branches in [False, True]:
        integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_min')
        integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_max')
        integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_window')
        integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_stride')
        # integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_and_mask')
        # integer_cols.append(f'{reg}_range{"" if not with_branches else "_with_branches"}_or_mask')
        integer_cols.append(f'{reg}_controlled_range{"" if not with_branches else "_with_branches"}_min')
        integer_cols.append(f'{reg}_controlled_range{"" if not with_branches else "_with_branches"}_max')
        integer_cols.append(f'{reg}_controlled_range{"" if not with_branches else "_with_branches"}_window')
        integer_cols.append(f'{reg}_controlled_range{"" if not with_branches else "_with_branches"}_stride')


def get_page_mask(tm: pd.Series, column):
    return (int(tm[column], 16) & PAGE_MASK)



def load_df(file_name):

    df_header = pd.read_csv(file_name, delimiter=';', low_memory=False)
    types_dict = {col: 'UInt64' if col in integer_cols else df_header[col].dtype.name for col in df_header}

    df = pd.read_csv(file_name, delimiter=';', dtype=types_dict)

    return df

def main(caller_to_target_csv, victims_csv, in_csv, out_csv):
    global IS_TFP

    caller_to_target_df = load_df(caller_to_target_csv)
    # victims_df = load_df(victims_csv)

    gadgets_df = load_df(in_csv)
    print()
    print(f"   [+] {'Loaded caller-target (rows):':<55} {  len(caller_to_target_df) }")
    # print(f"   [+] {'Loaded victims (rows):':<55} {  len(victims_df) }")
    print(f"   [+] {'Loaded gadgets (rows):':<55} {  len(gadgets_df) }")


    caller_to_target_df['page_mask'] = caller_to_target_df.apply(get_page_mask, args=('caller',), axis=1)
    # victims_df['page_mask'] = victims_df.apply(get_page_mask, args=('pc',), axis=1)

    merged_df = pd.merge(gadgets_df, caller_to_target_df, left_on='address', right_on='target')

    print(f"   [+] {'Merged gadgets (rows):':<55} {  len(merged_df) }")

    merged_df.to_csv(out_csv, sep=';', index=False)


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('victims_csv')
    arg_parser.add_argument('caller_to_target_csv')
    arg_parser.add_argument('in_csv')
    arg_parser.add_argument('out_csv')


    args = arg_parser.parse_args()
    main(args.caller_to_target_csv, args.victims_csv, args.in_csv, args.out_csv)
