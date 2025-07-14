import argparse
import pandas as pd
from itertools import combinations


# uarch           ,Set Index  ,Tag                    ,Target Length
# Skylake         ,[13:5]     ,[29:22]$\oplus$[21:14] ,32 / 10
# Sunny Cove      ,[13:5]     ,[33:24]$\oplus$[23:14] ,32 / 11
# Golden Gove (P) ,[14:5]     ,[23:15]                ,32
# Gracemont (E)   ,[14:5]     ,[24:15]                ,32
# Crestmont (E)   ,[15:6]     ,[25:16]                ,32
# Lion Cove (P)   ,[14:5]     ,[21:15]                ,32
# Skymont (E)     ,[15:6]     ,[25:16]                ,32

ENABLED_TAGS = {"21_13"}

BTB_TAG = {
    # [29:22] XOR [21:14] (9th + 10th gen Intel CPU)
    "29$22_21$14" :
        {
        "PART1_START": 14,
        "PART1_END"  : 21,
        "PART2_START": 22,
        "PART2_END"  : 29
        },
    # [33:24] XOR [23:14] (11th gen Intel CPU)
    "33$24_23$14" :
        {
        "PART1_START": 14,
        "PART1_END"  : 23,
        "PART2_START": 24,
        "PART2_END"  : 33
        },
    # 12th / 13th / 14th gen INTEL CPU
    "23_15" :
        {
        "PART1_START": 15,
        "PART1_END"  : 23,
        "PART2_START": -1,
        "PART2_END"  : -1
        },
    # Lion cove
    "21_13" :
        {
        "PART1_START": 13,
        "PART1_END"  : 21,
        "PART2_START": -1,
        "PART2_END"  : -1
        },
    # e-cores
    "24_15" :
        {
        "PART1_START": 15,
        "PART1_END"  : 24,
        "PART2_START": -1,
        "PART2_END"  : -1
        },
    "25_16" :
        {
        "PART1_START": 16,
        "PART1_END"  : 25,
        "PART2_START": -1,
        "PART2_END"  : -1
        }
    }

BTB_SET = {
    "13_5" : # 9th / 10th / 11th gen INTEL CPU
        {
        "BTB_SET_START": 5,
        "BTB_SET_END"  : 13
        },
    "14_5" : # 12th / 13th / 14th gen INTEL CPU
        {
        "BTB_SET_START": 5,
        "BTB_SET_END"  : 14
        },
    "12_4" : # lion cove
        {
        "BTB_SET_START": 4,
        "BTB_SET_END"  : 12
        },
    "15_5" : # Crestmont / Skymont E Core
        {
        "BTB_SET_START": 5, # Its bit 6 (5 to match offset)
        "BTB_SET_END"  : 15
        },
    }

# Mapping from TAG to SET
BTB_TAG_TO_SET = {
    "29$22_21$14" : "13_5",
    "33$24_23$14" : "13_5",
    "23_15" : "14_5",
    "24_15" : "14_5",
    "21_13" : "12_4",
    "25_16" : "15_5",
    }


def get_mask(start_bit, end_bit):
    return (((1 << (end_bit - start_bit + 1)) - 1) << (start_bit))

BTB_SET_ABOVE_CL_START = 6
BTB_SET_ABOVE_CL_END   = 13
BTB_SET_ABOVE_CL_MASK  = get_mask(BTB_SET_ABOVE_CL_START, BTB_SET_ABOVE_CL_END)

CACHE_LINE_MSB = 5
CACHE_LINE_MSB_MASK  = (1 << CACHE_LINE_MSB)

BTB_OFFSET_START = 0
BTB_OFFSET_END   = 4
BTB_OFFSET_MASK  = get_mask(BTB_OFFSET_START, BTB_OFFSET_END)

# short bit range
SHORT_BRANCH_BITS = [9, 11]

BRANCH_TYPE_DIRECT = 0
BRANCH_TYPE_INDIRECT = 1

DEFAULT_TEXT_BASE = 0xffffffff81000000


def get_mask(start_bit, end_bit):
    return (((1 << (end_bit - start_bit + 1)) - 1) << (start_bit))

def load_file(filename):

    addresses = []

    with open(filename) as file:
        for line in file:
            value = line.rstrip().split()[0]
            addresses.append(int(value, 16))

    return addresses

def get_tag_for_pc(tm: pd.Series, tag):
    mask_part1 = get_mask(tag['PART1_START'], tag['PART1_END'])
    part1 = (tm['entry_point'] & mask_part1) >> (tag['PART1_START'])

    if tag['PART2_START'] >= 0:
        mask_part2 = get_mask(tag['PART2_START'], tag['PART2_END'])
        part2 = (tm['entry_point'] & mask_part2) >> (tag['PART2_START'])

        return (part1 ^ part2)

    else:
        return part1


def get_mask_of_field(tm: pd.Series, mask, bit_start, field):
    return (tm[field] & mask) >> bit_start


duplicate_short_targets = 0

def create_collision_df(ind_a: pd.Series, ind_b: pd.Series, tag_id_match, full_match):
    global duplicate_short_targets

    if ind_a['is_module'] == 1 and ind_b['is_module'] == 0:
        df_a = ind_b.to_frame().T
        df_b = ind_a.to_frame().T
    else:
        df_a = ind_a.to_frame().T
        df_b = ind_b.to_frame().T

    df_a.columns = 'a_' + df_a.columns.values
    df_b.columns = 'b_' + df_b.columns.values

    # Create the 32-bit collision

    df_col = pd.concat([df_a, df_b.set_index(df_a.index)], axis=1)

    for tag_id in BTB_TAG:
      df_col[f'tag_match_{tag_id}'] = tag_id == tag_id_match

    df_col['full_match'] = full_match
    df_col['target_bit_length'] = 32
    df_col['target'] = ind_b['target']

    return df_col


def main(input, branch_entry_points, output, text_base):

    df_entry_info = pd.read_csv(branch_entry_points, sep=";")
    df_branch_info = pd.read_csv(input, sep=";")

    df = pd.merge(df_branch_info, df_entry_info, how='inner', on='address')

    df['target_type'] = df['target_type'].map({'direct': BRANCH_TYPE_DIRECT, 'indirect': BRANCH_TYPE_INDIRECT})

    df = df.drop_duplicates()
    integer_cols = ['address', 'pc', 'target', 'entry_point']

    # Transform columns.
    for i in integer_cols:
        df[i] = df[i].fillna('0')
        df[i] = df[i].apply(int, base=16)

    df = df.fillna('')

    duplicated_addresses = df[df.duplicated(subset=['address', 'entry_point'], keep=False)]
    print(f"Number of duplicated addresses: {len(duplicated_addresses)}\n")

    print(f"{'# total branches:':30} {len(df)}")
    print(f"{'# DIR branches:':30} {len(df[df['target_type'] == BRANCH_TYPE_DIRECT])}")
    print(f"{'# IND branches:':30} {len(df[df['target_type'] == BRANCH_TYPE_INDIRECT])}")

    # We only handle indirect branches
    df = df[df['target_type'] == BRANCH_TYPE_INDIRECT]

    print(f"{'# Kernel IND branches:':30} {len(df[df['is_module'] == 0])}")
    print(f"{'# Module IND branches:':30} {len(df[df['is_module'] == 1])}")

    if text_base:
        print(f"Setting text base to {text_base}")
        text_base = int(text_base, 16)
        offset = text_base - DEFAULT_TEXT_BASE
        print(f"  Adding Offset: {hex(offset)}")
        df['orig_address'] = df['address']
        df['orig_pc'] = df['pc']
        df['orig_entry_point'] = df['entry_point']

        df.loc[df['is_module'] == 0, 'address'] += offset
        df.loc[df['is_module'] == 0, 'pc'] += offset
        df.loc[df['is_module'] == 0, 'entry_point'] += offset


        df.loc[df['is_module'] == 1, 'orig_address'] &= (1 << 12) - 1
        df.loc[df['is_module'] == 1, 'orig_pc'] &= (1 << 12) - 1


    for tag_id in BTB_TAG:
        df[f'tag_{tag_id}'] = df.apply(get_tag_for_pc, args=(BTB_TAG[tag_id],), axis=1)

    for set_id in BTB_SET:
        set_mask = get_mask(BTB_SET[set_id]['BTB_SET_START'], BTB_SET[set_id]['BTB_SET_END'])
        df[f'set_{set_id}'] = df.apply(get_mask_of_field, args=(set_mask, BTB_SET[set_id]['BTB_SET_START'], 'entry_point'), axis=1)

    df['set_ab_cl'] = df.apply(get_mask_of_field, args=(BTB_SET_ABOVE_CL_MASK, BTB_SET_ABOVE_CL_START, 'entry_point'), axis=1)
    df['offset'] = df.apply(get_mask_of_field, args=(BTB_OFFSET_MASK, BTB_OFFSET_START, 'entry_point'), axis=1)
    df['cache_line_msb'] = df.apply(get_mask_of_field, args=(CACHE_LINE_MSB_MASK, CACHE_LINE_MSB, 'entry_point'), axis=1)

    df['pc_tag'] = df.apply(get_mask_of_field, args=(((1 << 10) - 1), 0, 'pc'), axis=1)



# -----------------------------------------------------------------------------
    n_collisions = 0

    all_collisions = pd.DataFrame()

    for tag_id in BTB_TAG:

        if tag_id not in ENABLED_TAGS:
            continue

        print("\nGrouping...")

        # To speed up processing we reduce the groups by filtering them
        # for at least one indirect branch. Faster way to do this?
        grouped = df.groupby([f'tag_{tag_id}', f'set_{BTB_TAG_TO_SET[tag_id]}', f'offset'])

        df_filtered = grouped.filter(lambda x : len(x) > 1)

        # Now group it again
        grouped = df_filtered.groupby([f'tag_{tag_id}', f'set_{BTB_TAG_TO_SET[tag_id]}', f'offset'])
        print(f"{f'# Groups for tag {tag_id}:':30} {len(grouped.groups)}")


        for id, group in grouped:


            for collision_combo in combinations(group.iterrows(), 2):

                ind_a = collision_combo[0][1]
                ind_b = collision_combo[1][1]

                if ind_a['pc_tag'] != ind_b['pc_tag']:
                    continue

                if not all_collisions.empty and not all_collisions[
                    ((all_collisions['a_address'] == ind_a['address']) & (all_collisions['b_address'] == ind_b['address'])) |
                    ((all_collisions['a_address'] == ind_b['address']) & (all_collisions['b_address'] == ind_a['address']))
                    ].empty:

                    # Collision pair already exists, we just set the tag match to true
                    all_collisions.loc[
                    ((all_collisions['a_address'] == ind_a['address']) & (all_collisions['b_address'] == ind_b['address'])) |
                    ((all_collisions['a_address'] == ind_b['address']) & (all_collisions['b_address'] == ind_a['address']))
                    , f'tag_match_{tag_id}'] = True
                    continue

                n_collisions += 1


                collisions = create_collision_df(ind_a, ind_b, tag_id, True)


                collisions = collisions.astype(all_collisions.dtypes)
                all_collisions = pd.concat([all_collisions, collisions])

    print("=" * 20)


    tag_match_list = [f'tag_match_{tag_id}' for tag_id in BTB_TAG]



    # df_unique = all_collisions.drop_duplicates(keep=False, subset=all_collisions.columns.difference(['tag_match_29$22_21$14', 'tag_match_33$24_23$14', 'tag_match_23$15']))
    # df_duplicates = all_collisions[all_collisions.duplicated(subset=all_collisions.columns.difference(['tag_match_29$22_21$14', 'tag_match_33$24_23$14', 'tag_match_23$15']))].copy()
    # print(f"{'Collisions with all tags matching:':30} {len(df_duplicates)}")


    # for tag_id in BTB_TAG:
    #     df_duplicates[f'tag_match_{tag_id}'] = True

    # final = pd.concat([df_unique, df_duplicates])
    # final = all_collisions[[all_collisions['a_is_module'] != 1 | all_collisions['b_is_module'] != 1]]

    # Filter out collisions from both modules
    # final = all_collisions[ (all_collisions['a_is_module'] == 0) | (all_collisions['b_is_module'] == 0)   ]
    final = all_collisions

    module_col = final[ (final['a_is_module'] == 1) | (final['b_is_module'] == 1)   ]


    ind_addresses = pd.concat([final['b_address'],final['a_address']])

    print("")
    print(f"{'Total collisions:':30} {n_collisions}")
    print(f"{'Total collisions rows:':30} {len(final)}")
    print(f"{'Collisions module branch:':30} {len(module_col)}")

    print(f"{'Unique IND branches:':30} {ind_addresses.nunique()}")


    for tag_id in BTB_TAG:

        if tag_id not in ENABLED_TAGS:
            continue

        col_tag = final[ final[f'tag_match_{tag_id}'] == True]

        # col_tag_text = col_tag[]
        # col_tag_modules = final[ final[f'tag_match_{tag_id}'] == True]

        ind_addresses_tag = pd.concat([col_tag['b_address'],col_tag['a_address']])
        ind_addresses_tag_kernel = pd.concat([col_tag[col_tag['b_is_module'] == 0]['b_address'],col_tag[col_tag['a_is_module'] == 0]['a_address']])
        ind_addresses_tag_module = pd.concat([col_tag[col_tag['b_is_module'] == 1]['b_address'],col_tag[col_tag['a_is_module'] == 1]['a_address']])

        module_col_tag = col_tag[ (col_tag['a_is_module'] == 1) | (col_tag['b_is_module'] == 1)   ]

        print(f"\nTAG {tag_id:>11}, SET {BTB_TAG_TO_SET[tag_id]}, OFFSET 0:5")

        n_all = len(col_tag)
        n_kernel_kernel = len(col_tag[ (col_tag['a_is_module'] == 0) & (col_tag['b_is_module'] == 0)   ])
        n_module_module = len(col_tag[ (col_tag['a_is_module'] == 1) & (col_tag['b_is_module'] == 1)   ])
        print(f"{'  Total collisions:':30} {n_all}")
        print(f"{'   - kernel - kernel:':30} {n_kernel_kernel}")
        print(f"{'   - kernel - module:':30} {n_all - n_module_module - n_kernel_kernel}")
        print(f"{'   - module - module:':30} {n_module_module}")
        print(f"{'  Unique IND branches:':30} {ind_addresses_tag.nunique()}")
        print(f"{'  - Kernel IND branches:':30} {ind_addresses_tag_kernel.nunique()}")
        print(f"{'  - Module IND branches:':30} {ind_addresses_tag_module.nunique()}")
        print(f"\n{'  Collisions module branch:':30} {len(module_col_tag)}")



        # if n_all == 36:


    grouped = final.groupby(['a_address', 'b_address'])

    for _, group in grouped:
        if len(group) > 1:
            print("ERROR: Group containing more than 1!", group)
            print(group[tag_match_list])



# -----------------------------------------------------------------------------

    columns_to_hex = ['a_address', 'b_address', 'a_pc', 'b_pc', 'a_target', 'b_target', 'a_entry_point', 'b_entry_point', 'target']
    if text_base:
        columns_to_hex += ['a_orig_address', 'b_orig_address', 'a_orig_pc', 'b_orig_pc', 'a_orig_entry_point', 'b_orig_entry_point']

    # Transform columns.
    for i in columns_to_hex:
        # df_targets[i] = df_targets[i].apply(int)
        final[i] = final[i].apply(hex)

    # import IPython
    # IPython.embed()

    final.to_csv(output, sep=';', index=False)

    final.drop(columns=['a_entry_point', 'b_entry_point', 'a_orig_entry_point', 'b_orig_entry_point', 'a_pc_tag', 'b_pc_tag'], inplace=True)
    final.to_csv(f"{output}.compatible", sep=';', index=False)

if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('input')
    arg_parser.add_argument('branch_entry_points')
    arg_parser.add_argument('output')
    arg_parser.add_argument('--text-base', required=False, default='', help='new base address of kernel text')


    args = arg_parser.parse_args()
    main(args.input, args.branch_entry_points, args.output, args.text_base)
