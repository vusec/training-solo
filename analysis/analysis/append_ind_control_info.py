import argparse
import pandas as pd
import struct

IS_TFP = False

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


def get_register_requirement_from_expr(expr : str):

        if 'UNINITIALIZED' in expr:
            return None

        if 'gs' in expr:
            return None

        splitted = expr.split(' ')

        if len(splitted) != 2 or not splitted[0].startswith('<BV') or not splitted[1].endswith('>'):
            if 'mem@' in expr:
                # known case, ignore
                return None
            print("ERROR: Could not extract register from requirement:", r)
            return None

        return splitted[1].removesuffix('>')


def get_direct_register_requirement_list(requirements):
    regs = []

    for reg_expr in requirements:
        reg = get_register_requirement_from_expr(reg_expr)
        if reg != None:
            regs.append(reg)

    return regs

def get_indirect_register_requirement_list(requirements):

    regs = []

    for reg_offsets in requirements:
        splitted = reg_offsets.split(">:")

        if len(splitted) != 2:
            print("ERROR: Could not extract register with offset requirement", reg_offsets)
            continue

        reg_expr = f'{splitted[0]}>'

        offsets = eval(splitted[1])
        reg = get_register_requirement_from_expr(reg_expr)
        if reg == None:
            continue

        if not offsets:
            # If the offsets are unkown (e.g, symbolic), we assume
            # a direct requirement
            regs.append(f"{reg}")
            continue

        for offset_str in offsets:

            offset = int(offset_str, 16)

            # Check signed bit, convert to negative if set
            if offset & (1<< 63):
                offset = -( (offset ^ ((1 << 64 ) - 1)) + 1)

            regs.append(f"{reg}_{offset}")

    return regs


def get_register_requirement_list(direct_requirements, indirect_requirements):

    direct_regs = get_direct_register_requirement_list(direct_requirements)

    indirect_regs = get_indirect_register_requirement_list(indirect_requirements)

    # import IPython
    # IPython.embed()

    return tuple(set(direct_regs + indirect_regs))




def is_base_controlled(t : pd.Series):
    return t['base_control'] in ['ControlType.CONTROLLED', 'ControlType.REQUIRES_MEM_LEAK']

def convert_list_str_to_tuple(t : pd.Series, column):

    lst = eval(t[column])
    assert(type(lst) == list)

    return tuple(set(lst))


def get_controlled_requirements_regs(t : pd.Series):

    requirement_groups = []

    if IS_TFP:
        all_requirements = get_register_requirement_list(eval(t['requirements_direct_regs']), eval(t['requirements_indirect_regs']))
        return all_requirements, (all_requirements,)

    else:
        requirement_groups.append(get_register_requirement_list(eval(t['secret_address_requirements_direct_regs']), eval(t['secret_address_requirements_indirect_regs'])))

        base_group = tuple()
        if is_base_controlled(t) and t['is_max_secret_too_high']:
            base_group = get_register_requirement_list(eval(t['base_requirements_direct_regs']), eval(t['base_requirements_indirect_regs']))


        if t['transmitter'] == 'TransmitterType.SECRET_DEP_BRANCH':

            if is_base_controlled(t) and \
                t['base_control_type'] in ('BaseControlType.BASE_INDEPENDENT_FROM_SECRET', 'BaseControlType.COMPLEX_TRANSMISSION'):
                # We dont require both controlled base and controlled cmp_value, add to base group
                base_group += get_register_requirement_list(eval(t['controlled_cmp_value_requirements_direct_regs']), eval(t['controlled_cmp_value_requirements_indirect_regs']))

            elif t['cmp_value_control'] == "ControlType.CONTROLLED" and \
                (t['secret_address_requirements_indirect_regs'] != t['cmp_value_requirements_indirect_regs'] or \
                t['secret_address_requirements_direct_regs'] != t['cmp_value_requirements_direct_regs']):

                # We depend on that the cmp_value is controlled, add the requirements as a separate group
                requirement_groups.append(get_register_requirement_list(eval(t['controlled_cmp_value_requirements_direct_regs']), eval(t['controlled_cmp_value_requirements_indirect_regs'])))

        if base_group:
            requirement_groups.append(tuple(set(base_group)))

        requirement_groups = tuple(set(requirement_groups))

        return tuple(set(sum(requirement_groups, ()))), requirement_groups





def load_df(file_name):

    df_header = pd.read_csv(file_name, delimiter=';', low_memory=False)
    types_dict = {col: 'UInt64' if col in integer_cols else df_header[col].dtype.name for col in df_header}

    df = pd.read_csv(file_name, delimiter=';', dtype=types_dict)

    return df



def main(victims_csv, in_csv, out_csv):
    global IS_TFP

    victims_df = load_df(victims_csv)
    gadgets_df = load_df(in_csv)
    print()
    print(f"   [+] {'Loaded victims (rows):':<55} {  len(victims_df) }")
    print(f"   [+] {'Loaded gadgets (rows):':<55} {  len(gadgets_df) }")

    if 'base_control' in gadgets_df.columns:
        assert('controlled' not in gadgets_df.columns)
        print("[-] Loaded gadget type 'gadgets'")
    elif 'controlled' in gadgets_df.columns:
        assert('base_control' not in gadgets_df.columns)
        print("[-] Loaded gadget type 'tfps'")
        IS_TFP = True

    if 'ind_orig_address' in gadgets_df.columns:
        ind_address_key = 'ind_orig_address'
    else:
        ind_address_key = 'ind_address'


    print(f"[-] Performing analysis...")

    gadgets_df[['controlled_requirements_regs', 'controlled_requirements_regs_groups']] = gadgets_df.apply(get_controlled_requirements_regs, axis=1, result_type="expand")
    victims_df['controlled_sufficiently'] = victims_df.apply(convert_list_str_to_tuple, axis=1, args=('controlled_sufficiently',))
    victims_df['controlled_sufficiently_indirect'] = victims_df.apply(convert_list_str_to_tuple, axis=1, args=('controlled_sufficiently_indirect',))
    victims_df['controlled_sufficiently_all'] = victims_df.apply(convert_list_str_to_tuple, axis=1, args=('controlled_sufficiently_all',))

    controlled_direct_victims_df = victims_df[ victims_df['n_controlled_sufficiently'] > 0 ]
    controlled_indirect_victims_df = victims_df[ victims_df['n_controlled_sufficiently_indirect'] > 0 ]
    controlled_all_victims_df = victims_df[ victims_df['n_controlled_sufficiently_all'] > 0 ]

    print(f"   [+] {'Number of unique victim PCs:':<55} {len(victims_df['pc'].unique())}")
    print(f"   [+] {'Number of unique victim PCs with direct control:':<55} {len(controlled_direct_victims_df['pc'].unique())}")
    print(f"   [+] {'Number of unique victim PCs with indirect control:':<55} {len(controlled_indirect_victims_df['pc'].unique())}")
    print(f"   [+] {'Number of unique victim PCs with any control:':<55} {len(controlled_all_victims_df['pc'].unique())}")
    print(f"   [+] {'Number of unique ind_address:':<55} {len(gadgets_df['ind_address'].unique())}")

    gadgets_wo_controlled_victim =  gadgets_df.loc[ ~gadgets_df[ind_address_key].isin(controlled_all_victims_df['pc'])]
    gadgets_w_controlled_victim =  gadgets_df.loc[ gadgets_df[ind_address_key].isin(controlled_all_victims_df['pc'])]
    gadgets_w_victim =  gadgets_df.loc[ gadgets_df[ind_address_key].isin(victims_df['pc'])]
    print("")
    print(f"   [+] {'Number of unique ind_address WITHOUT a controlled victims:':<55} {  len(gadgets_wo_controlled_victim['ind_address'].unique()) }")
    print(f"   [+] {'Number of ind_address WITH controlled victims:':<55} {  len(gadgets_w_controlled_victim) }")
    print(f"   [+] {'Number of unique ind_address WITH controlled victims:':<55} {  len(gadgets_w_controlled_victim['ind_address'].unique()) }")
    print(f"   [+] {'Number of unique dir_address WITH controlled victims:':<55} {  len(gadgets_w_controlled_victim['dir_address'].unique()) }")

    print(f"[-] Grouping...")

    victims_by_pc = victims_df[['pc', 'name', 'uuid', 'controlled_sufficiently', 'controlled_sufficiently_indirect', 'controlled_sufficiently_all', 'controlled']].groupby('pc').agg(lambda x: set(y for y in x if y))
    victims_by_pc['n_name'] = victims_df[['pc', 'name', 'uuid', 'controlled_sufficiently', 'controlled']].groupby('pc').name.nunique()


    gadgets_w_victim = pd.merge(gadgets_w_victim, victims_by_pc, left_on=ind_address_key, right_on='pc', suffixes=('', '_victim'))
    gadgets_w_victim.rename(columns={"name_victim": "victims_all_name", "uuid_victim": "victims_all_uuid",
                        "controlled_sufficiently_victim" : "victims_all_regs_sufficient", "controlled_sufficiently_indirect_victim" : "victims_all_regs_sufficient_indirect",
                        "controlled_sufficiently_all_victim" : "victims_all_regs_sufficient_all", "controlled_victim" : "victims_all_regs"}, inplace=True)

    gadgets_grouped_df = gadgets_w_victim.groupby([ind_address_key, 'controlled_requirements_regs_groups'])



    all_gadgets = pd.DataFrame()

    print(f"[-] Matching ind_address with victims pc...")

    victims_grouped_df = controlled_all_victims_df[['pc', 'name', 'uuid', 'controlled_sufficiently', 'controlled_sufficiently_indirect', 'controlled_sufficiently_all', 'controlled']].groupby(['pc', 'controlled_sufficiently', 'controlled_sufficiently_indirect', 'controlled_sufficiently_all'])



    required_regs_non_matching = set()

    for (ind_address, required_regs_groups,), gadgets in gadgets_grouped_df:

        victims_matching = {'uuid' : set(), 'name' : set(), 'regs' : set(), 'regs_suf' : set(), 'regs_matching' : set()}

        for (victim_pc, controlled_sufficiently, controlled_sufficiently_indirect, controlled_sufficiently_all), victims in victims_grouped_df:
            if victim_pc != ind_address:
                continue

            matching_regs = []

            ok = True
            for required_regs in required_regs_groups:

                # For each group we need at least 1 match
                group_match = False

                for r in required_regs:

                    # First check against direct regs
                    # Convert indirect requirements to direct reg
                    reg_direct = r.split('_')[0]

                    if reg_direct in controlled_sufficiently:
                        matching_regs.append(reg_direct)
                        group_match = True

                    elif r in controlled_sufficiently_indirect:
                        matching_regs.append(r)
                        group_match = True

                if not group_match:
                    # Not a single register in the group matched
                    ok = False
                    break

            if not ok:
                continue

            # At least 1 overlap with each group requirement
            victims_matching['uuid'].update(set(victims['uuid']))
            victims_matching['name'].update(victims['name'])
            victims_matching['pc'] = victims['pc']
            victims_matching['regs'].update([tuple(set(victims['controlled']))])
            victims_matching['regs_suf'].update([tuple(set(controlled_sufficiently_all))])
            victims_matching['regs_matching'].update([tuple(set(matching_regs))])

        if not victims_matching['uuid']:
            required_regs_non_matching.update([tuple(set(required_regs_groups))])

        gadgets['victims_uuid'] = str(victims_matching['uuid'])
        gadgets['victims_name'] = str(victims_matching['name'])
        gadgets['victims_regs'] = str(victims_matching['regs'])
        gadgets['victims_regs_suf'] = str(victims_matching['regs_suf'])
        gadgets['victims_regs_matching'] = str(victims_matching['regs_matching'])
        gadgets['n_victims_name'] = len(victims_matching['name'])
        gadgets['n_victims_uuid'] = len(victims_matching['uuid'])

        all_gadgets = pd.concat([all_gadgets, gadgets])

    all_gadgets = pd.concat([all_gadgets, gadgets_wo_controlled_victim])

    all_gadgets_w_controlled_victim = all_gadgets[ all_gadgets['n_victims_name'] > 0 ]

    # print(f"   [+] {'Required regs without a matching victim:':<55} {required_regs_non_matching}")

    print(f"   [+] {'Number of gadgets (rows):':<55} {  len(all_gadgets) }")
    print(f"   [+] {'Number of unique gadgets (PC) with a victim:':<55} {  len(gadgets_w_victim['pc'].unique()) }")
    print(f"   [+] {'Number of unique gadgets (PC) with matching victim:':<55} {  len(all_gadgets_w_controlled_victim['pc'].unique()) }")


    if not IS_TFP:
        print(all_gadgets[ all_gadgets['n_victims_name'] > 0 ].groupby(['transmitter']).size())

    print(f"   [+] {'Matching registers:':<55} ")

    matching_regs = list(all_gadgets['victims_regs_matching'].unique())
    matching_regs.remove('set()')
    print(matching_regs)

    # Save to new file.
    print(f"[-] Saving to {out_csv}")

    all_gadgets.to_csv(out_csv, sep=';', index=False)


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('victims_csv')
    arg_parser.add_argument('in_csv')
    arg_parser.add_argument('out_csv')


    args = arg_parser.parse_args()
    main(args.victims_csv, args.in_csv, args.out_csv)
