import argparse
import pandas as pd

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
    }

def get_mask(start_bit, end_bit):
    return (((1 << (end_bit - start_bit + 1)) - 1) << (start_bit))

BTB_SET_START = 5
BTB_SET_END   = 13
BTB_SET_MASK  = get_mask(BTB_SET_START, BTB_SET_END)

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
    mask_part2 = get_mask(tag['PART2_START'], tag['PART2_END'])

    part1 = (tm['pc'] & mask_part1) >> (tag['PART1_START'])
    part2 = (tm['pc'] & mask_part2) >> (tag['PART2_START'])

    tag_hash = (part1 ^ part2)
    tag_set_ab_cl = (tag_hash << (BTB_SET_ABOVE_CL_END - BTB_SET_ABOVE_CL_START + 1)) + tm['set_ab_cl']
    tag_set_offset = (tag_hash << tag['PART1_START']) + (tm['set'] << BTB_SET_START) + tm['offset']

    return tag_hash, tag_set_ab_cl, tag_set_offset

def get_mask_of_pc(tm: pd.Series, mask, bit_start):
    return (tm['pc'] & mask) >> bit_start

duplicate_short_targets = 0

def create_collision_df(ind: pd.Series, dir: pd.Series, tag_id_match, full_match):
    global duplicate_short_targets

    new_collisions = []

    df_ind = ind.to_frame().T
    df_ind.columns = 'ind_' + df_ind.columns.values
    df_dir = dir.to_frame().T
    df_dir.columns = 'dir_' + df_dir.columns.values

    # Create the 32-bit collision

    df_col = pd.concat([df_ind, df_dir.set_index(df_ind.index)], axis=1)

    for tag_id in BTB_TAG:
      df_col[f'tag_match_{tag_id}'] = tag_id == tag_id_match

    df_col['full_match'] = full_match
    df_col['target_bit_length'] = 32
    df_col['target'] = dir['target']

    new_collisions = [df_col]

    # Create the short collisions if any
    for last_bit in SHORT_BRANCH_BITS:
        short_branch_mask = get_mask(0, last_bit)

        if (dir['orig_pc'] & ~short_branch_mask) == (dir['target'] & ~short_branch_mask):


            target = (ind['orig_pc'] & ~short_branch_mask) + (dir['target'] & short_branch_mask)

            df_col = pd.concat([df_ind, df_dir.set_index(df_ind.index)], axis=1)

            for tag_id in BTB_TAG:
                df_col[f'tag_match_{tag_id}'] = tag_id == tag_id_match

            df_col['full_match'] = full_match
            df_col['target_bit_length'] = last_bit + 1
            df_col['target'] = target

            if target == dir['target']:
                duplicate_short_targets += 1
                continue

            new_collisions.append(df_col)

    return new_collisions


def main(input, output, text_base):

    print("[+] Reading csv")

    df = pd.read_csv(input, sep=";")

    df = df.drop_duplicates()

    df['target_type'] = df['target_type'].map({'direct': BRANCH_TYPE_DIRECT, 'indirect': BRANCH_TYPE_INDIRECT})

    integer_cols = ['address', 'pc', 'target']



    # Transform columns.
    for i in integer_cols:
        df[i] = df[i].fillna('0')
        df[i] = df[i].apply(int, base=16)

    df = df.fillna('')

    print(f"{'# total branches:':30} {len(df)}")
    df_ind = df[df['target_type'] == BRANCH_TYPE_INDIRECT]
    df_dir = df[df['target_type'] == BRANCH_TYPE_DIRECT]
    print(f"{'# Total IND branches:':30} {len(df_ind)}")
    print(f"{'# Total DIR branches:':30} {len(df_dir)}")

    print(f"{'# Kernel IND branches:':30} {len(df_ind[df_ind['is_module'] == 0])}")
    print(f"{'# Kernel DIR branches:':30} {len(df_dir[df_dir['is_module'] == 0])}")
    print(f"{'# Module IND branches:':30} {len(df_ind[df_ind['is_module'] == 1])}")
    print(f"{'# Module DIR branches:':30} {len(df_dir[df_dir['is_module'] == 1])}")

    df['orig_address'] = df['address']
    df['orig_pc'] = df['pc']

    if text_base:
        print(f"Setting text base to {text_base}")
        text_base = int(text_base, 16)
        offset = text_base - DEFAULT_TEXT_BASE
        print(f"  Adding Offset: {hex(offset)}")

        df.loc[df['is_module'] == 0, 'address'] += offset
        df.loc[df['is_module'] == 0, 'pc'] += offset

        df.loc[df['is_module'] == 1, 'orig_address'] &= (1 << 12) - 1
        df.loc[df['is_module'] == 1, 'orig_pc'] &= (1 << 12) - 1


    df['set'] = df.apply(get_mask_of_pc, args=(BTB_SET_MASK, BTB_SET_START), axis=1)
    df['set_ab_cl'] = df.apply(get_mask_of_pc, args=(BTB_SET_ABOVE_CL_MASK, BTB_SET_ABOVE_CL_START), axis=1)
    df['offset'] = df.apply(get_mask_of_pc, args=(BTB_OFFSET_MASK, BTB_OFFSET_START), axis=1)
    df['cache_line_msb'] = df.apply(get_mask_of_pc, args=(CACHE_LINE_MSB_MASK, CACHE_LINE_MSB), axis=1)

    for tag_id in BTB_TAG:
        df[[f'tag_{tag_id}', f'tag_{tag_id}_set_ab_cl', f'tag_{tag_id}_set_offset']] = df.apply(get_tag_for_pc, args=(BTB_TAG[tag_id],), axis=1, result_type="expand")



# -----------------------------------------------------------------------------
    df_ind = df[df['target_type'] == BRANCH_TYPE_INDIRECT]
    df_dir = df[df['target_type'] == BRANCH_TYPE_DIRECT]


    n_collisions = 0

    collision_df = pd.DataFrame()
    all_collisions = []

    # -----------------------------------------------------------------------------
    # Full match targets (only 9th gen cpu)
    print("\nCollecting full match targets...")
    tag_id = "29$22_21$14"

    print("Grouping...")

    # First filtering with isin is much faster then doing group by on tag + set
    ind_set_offset = df_ind[f'tag_{tag_id}_set_offset'].unique()
    df_dir_matching = df_dir[df_dir[f'tag_{tag_id}_set_offset'].isin(ind_set_offset)]

    dir_set_offset = df_dir_matching[f'tag_{tag_id}_set_offset'].unique()
    df_ind_matching = df_ind[df_ind[f'tag_{tag_id}_set_offset'].isin(dir_set_offset)]

    # Now group it
    df_filtered = pd.concat([df_dir_matching, df_ind_matching])
    grouped = df_filtered.groupby([f'tag_{tag_id}_set_offset'])
    print(f"{f'# Full match Groups for tag {tag_id}:':30} {len(grouped.groups)}")

    for _, group in grouped:
        ind_branches = group[group["target_type"] == BRANCH_TYPE_INDIRECT]
        dir_branches = group[group["target_type"] == BRANCH_TYPE_DIRECT]


        for _, ind in ind_branches.iterrows():

            for _, dir in dir_branches.iterrows():
                n_collisions += 1

                collisions = create_collision_df(ind, dir, tag_id, True)

                if collision_df.empty:
                    collision_df = pd.concat(collisions)

                for col in collisions:
                    all_collisions.append(col.astype(collision_df.dtypes))

    print(f"{'Total full match collisions:':30} {n_collisions}")
    print(f"{'Total full match targets:':30} {len(all_collisions)}")

    if n_collisions:
        collision_df = pd.concat(all_collisions)
        print(f"{'Unique targets:':30} {collision_df['target'].nunique()}")


    # -----------------------------------------------------------------------------
    # Non-full match targets
    # Now we can discard all non matching MSB
    print("\nProcessing non-full match targets...")

    df_ind = df_ind[df_ind['cache_line_msb'] == 0]
    df_dir = df_dir[df_dir['cache_line_msb'] == 1]
    print(f"{'# Total IND branches PC[5]==0:':30} {len(df_ind)}")
    print(f"{'# Total DIR branches PC[5]==1:':30} {len(df_dir)}")


    for tag_id in BTB_TAG:


        print("\nGrouping...")

        # First filtering with isin is much faster then doing group by on tag + set
        ind_set_offset = df_ind[f'tag_{tag_id}_set_ab_cl'].unique()
        df_dir_matching = df_dir[df_dir[f'tag_{tag_id}_set_ab_cl'].isin(ind_set_offset)]

        dir_set_offset = df_dir_matching[f'tag_{tag_id}_set_ab_cl'].unique()
        df_ind_matching = df_ind[df_ind[f'tag_{tag_id}_set_ab_cl'].isin(dir_set_offset)]

        # Now group it
        df_filtered = pd.concat([df_dir_matching, df_ind_matching])
        grouped = df_filtered.groupby([f'tag_{tag_id}_set_ab_cl'])
        print(f"{f'# Groups for tag {tag_id}:':30} {len(grouped.groups)}")

        idx = 0
        for _, group in grouped:
            idx += 1
            ind_branches = group[group["target_type"] == BRANCH_TYPE_INDIRECT]
            dir_branches = group[group["target_type"] == BRANCH_TYPE_DIRECT]

            # print(f"\rProcessing {idx:4} ind: {len(ind_branches):3} dir: {len(dir_branches):3}", end='')

            for _, ind in ind_branches.iterrows():

                colliding_branches = dir_branches[(dir_branches['set_ab_cl'] == ind['set_ab_cl']) &
                                        (dir_branches[f'tag_{tag_id}'] == ind[f'tag_{tag_id}'])]

                for _, dir in dir_branches.iterrows():

                    n_collisions += 1

                    collisions = create_collision_df(ind, dir, tag_id, False)

                    if collision_df.empty:
                        collision_df = pd.concat(collisions)

                    for col in collisions:
                        all_collisions.append(col.astype(collision_df.dtypes))


    collision_df = pd.concat(all_collisions)

    df_unique = collision_df.drop_duplicates(keep=False, subset=collision_df.columns.difference(['tag_match_29$22_21$14', 'tag_match_33$24_23$14']))
    df_duplicates = collision_df[collision_df.duplicated(subset=collision_df.columns.difference(['tag_match_29$22_21$14', 'tag_match_33$24_23$14']))].copy()
    print(f"\n{'Collisions with all tags matching:':30} {len(df_duplicates)}")


    for tag_id in BTB_TAG:
        df_duplicates[f'tag_match_{tag_id}'] = True

    final = pd.concat([df_unique, df_duplicates])
    # final = final[ (final['ind_is_module'] == 0) | (final['dir_is_module'] == 0)]

    module_module_col = final[ (final['ind_is_module'] == 1) & (final['dir_is_module'] == 1)   ]
    text_text_col = final[ (final['ind_is_module'] == 0) & (final['dir_is_module'] == 0)   ]
    text_module_col = final[ (final['ind_is_module'] == 0) & (final['dir_is_module'] == 1)   ]
    module_text_col = final[ (final['ind_is_module'] == 1) & (final['dir_is_module'] == 0)   ]
    collision_pairs = final[['ind_pc', 'dir_pc']]
    collision_pairs = collision_pairs.drop_duplicates()


    print("")
    print(f"{'Total collisions:':30} {n_collisions}")
    print(f"{'  Unique Collision pairs:':30} {len(collision_pairs)}")
    print(f"{'Total text-text collisions:':30} {len(text_text_col)}")
    print(f"{'Total text-module collisions:':30} {len(text_module_col)}")
    print(f"{'Total module-text collisions:':30} {len(module_text_col)}")
    print(f"{'Total mod-mod collisions:':30} {len(module_module_col)}")
    print(f"{'Total targets (rows):':30} {len(final)}")
    print(f"{'Unique targets:':30} {final['target'].nunique()}")
    print(f"{'Unique IND branches:':30} {final['ind_address'].nunique()}")
    print(f"{'Unique DIR branches:':30} {final['dir_address'].nunique()}")
    print(f"{'Skipped short targets (dup.):':30} {duplicate_short_targets}")

    for tag_id in BTB_TAG:
        col_tag = final[ final[f'tag_match_{tag_id}'] == True]
        module_col = col_tag[ (col_tag['ind_is_module'] == 1) | (col_tag['dir_is_module'] == 1) ]
        collision_pairs = col_tag[['ind_pc', 'dir_pc']]
        collision_pairs.drop_duplicates()

        print(f"\nTAG {tag_id}:")


        print(f"{'  Unique IND branches:':30} {col_tag['ind_address'].nunique()}")
        print(f"{'  Unique DIR branches:':30} {col_tag['dir_address'].nunique()}")
        print(f"{'  Unique Collision pairs:':30} {len(collision_pairs)}")

        print(f"{'  Total targets:':30} {len(col_tag)}")
        print(f"{'  Unique targets:':30} {col_tag['target'].nunique()}")
        print(f"{'  Total module collisions:':30} {len(module_col)}")
        print(f"{'  Full-match targets:':30} {len(col_tag[ col_tag['full_match'] == 1])}")
        print(f"{'  Non Full-match targets:':30} {len(col_tag[ col_tag['full_match'] == 0])}")

        for target_bit_length in SHORT_BRANCH_BITS + [31]:
            target_bit_length += 1

            df_bit_length = col_tag[ col_tag['target_bit_length'] == target_bit_length]

            print(f"{f'  Targets bit-length {target_bit_length}:':30} {len(df_bit_length)}")
            print(f"{f'                     - Unique:':30} {df_bit_length['target'].nunique()}")


    grouped = final.groupby(['ind_pc', 'dir_pc', 'target_bit_length'])

    for _, group in grouped:
        if len(group) > 1:
            print("ERROR: Group containing more than 1!", group)


# -----------------------------------------------------------------------------
    columns_to_hex = ['ind_address', 'dir_address', 'ind_pc', 'dir_pc', 'dir_target', 'target']
    if text_base:
        columns_to_hex += ['ind_orig_address', 'dir_orig_address', 'ind_orig_pc', 'dir_orig_pc']

    # Transform columns.
    for i in columns_to_hex:
        final[i] = final[i].apply(hex)

    collision_pairs = final.drop_duplicates(subset=['ind_pc', 'dir_pc'])

    final.to_csv(output, sep=';', index=False)

    output_unique = f"{output.removesuffix('.csv')}_unique_pairs.csv"
    collision_pairs.to_csv(output_unique, sep=';', index=False)


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('input')
    arg_parser.add_argument('output')
    arg_parser.add_argument('--text-base', required=False, default='', help='new base address of kernel text')


    args = arg_parser.parse_args()
    main(args.input, args.output, args.text_base)
