import argparse
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.colors import LogNorm
import scienceplots
import seaborn as sns
import pandas as pd
import os

TOTAL_SNAPSHOTS = 100

key_a = ''
key_b = ''

tag_to_label = {
    '29$22_21$14' : 'Comet Lake',
    '33$24_23$14' : 'Sunny Cove',
    '23_15'       : 'Golden Cove',
    '21_13'       : 'Lion Cove',
    '21_13_dir'   : 'Lion Cove$^\dagger$',
    '24_15'       : 'Gracemont',
    '25_16'       : 'Crestmont\n\& Skymont'
}


def create_graph(df : pd.DataFrame, output_base, a_is_module, b_is_module, suffix, title):

    global TOTAL_SNAPSHOTS
    print("="*40)
    print(f"a_is_module = {a_is_module} b_is_module = {b_is_module}")

    df = df[(df[f'{key_a}_is_module'] == a_is_module) & (df[f'{key_b}_is_module'] == b_is_module)]
    orig_df = df

    # ---------------------------------
    tag_match_fields = [name for name in df.columns if name.startswith('tag_match_')]

    # quickfix: hardcode order
    if  'tag_match_21_13_dir' in tag_match_fields:
        tag_match_fields = ['tag_match_29$22_21$14',
                            'tag_match_33$24_23$14',
                            'tag_match_23_15',
                            'tag_match_21_13',
                            'tag_match_21_13_dir',
                            'tag_match_24_15',
                            'tag_match_25_16'
                            ]

    count_series = df.groupby([f'{key_a}_orig_address', f'{key_a}_module_name', f'{key_b}_orig_address', f'{key_b}_module_name'] + tag_match_fields).size()

    df = count_series.to_frame(name = 'n_snapshots').reset_index()

    if df['n_snapshots'].max() > TOTAL_SNAPSHOTS:
        TOTAL_SNAPSHOTS = df['n_snapshots'].max()

    average_collisions = {name: int(round(len(orig_df[orig_df[name] == True]) / TOTAL_SNAPSHOTS, 0)) for name in tag_match_fields}
    average_collisions = pd.DataFrame([average_collisions])

    # Select the columns you are interested in

    # Group by 'n_snapshots' and count the True values for each 'tag_match' column
    result = df.groupby('n_snapshots')[tag_match_fields].apply(lambda x: (x == True).sum()).reset_index()

    # Now bin them in 5 categories

    # Define the bins for percentages (0-10%, 10-20%, etc.)
    bins = [0, 25, 50, 75, 100]
    labels = ['0-25\%', '25-50\%', '50-75\%', '75-100\%']

    # Create a new column for the percentage category
    df['snapshots_percentage'] = pd.cut(df['n_snapshots'], bins=bins, labels=labels, include_lowest=True)

    # Group by the percentage category and count the True values for each tag_match_* column
    result = df.groupby('snapshots_percentage', observed=False)[tag_match_fields].apply(lambda x: (x == True).sum()).reset_index()

    # Remove tag match
    to_rename = {}
    for name in df.columns:
        if name.startswith('tag_match_'):
            to_rename[name] = name.removeprefix('tag_match_')

    result = result.rename(columns=to_rename).rename(columns=tag_to_label)
    average_collisions = average_collisions.rename(columns=to_rename).rename(columns=tag_to_label)
    # Display the result
    print("Average collisions:")
    print(average_collisions)

    average_collisions['snapshots_percentage'] = 'AVG/\nsnapshot'
    # average_collisions['snapshots_percentage'] = 'AVG'
    # result = pd.concat([result, average_collisions]) # AVG COLUMN

    print(result)


    # Create a heatmap. The index will be the 'snapshots_percentage' column (previously created).
    # Assume 'snapshots_percentage' is in the index of the result
    heatmap_data = result.set_index('snapshots_percentage')

    # transpose
    heatmap_data = heatmap_data.T

    # Set up the figure and the heatmap
    # plt.style.use(['science'])
    plt.rcParams["font.size"] = 8
    plt.rcParams["font.family"] = "serif"
    # plt.rcParams["mathtext.fontset"] = "dejavuserif"
    plt.rcParams["text.usetex"] = True
    # plt.rcParams['text.latex.preamble'] = "\\usepackage{amsmath} \\usepackage{amssymb}"

    # plt.figure(figsize=(int(10 * 0.6), int(7 * 0.6)))
    # plt.figure(figsize=(3.3, 2.5))
    if key_a == 'a':
        plt.figure(figsize=(3.3, 1.5)) # 1,7 indirect
        # plt.figure(figsize=(3.3, 1.8)) # for extra input
    else:
        plt.figure(figsize=(3.3, 0.9)) # 1.1 its

    plt.rcParams.update({
            'figure.dpi': '300'
        })


    cmap = sns.color_palette("Blues", as_cmap=True)
    cmap.set_under('white')

    # Use seaborn's heatmap for better visual appeal
    ax = sns.heatmap(
        heatmap_data,
        annot=True,    # To show the counts inside the heatmap cells
        fmt=',',       # Display full numbers, not scientific notation
        cmap=cmap,     # Use the custom colormap
        cbar=False,     # Show the color bar
        linewidths=0.5,# Optional: adds lines between cells
        # vmin=0.1,       # Set minimum value for color mapping so 0 stays white
        norm=LogNorm(vmin=0.1, vmax=heatmap_data.max().max()),  # Apply log scale to color mapping
        # mask=mask
    )

    # Add a vertical line between the last bucket and the "AVG" column
    # plt.axvline(x=len(labels), color='black', linestyle='-', linewidth=0.1)
    # plt.axvline(x=heatmap_data.shape[1] - 1.5, color='black', linestyle='--', linewidth=1)


    # Manually annotate zeros to ensure they are visible
    for i in range(heatmap_data.shape[0]):
        for j in range(heatmap_data.shape[1]):
            if heatmap_data.iloc[i, j] == 0:
                ax.text(j + 0.5, i + 0.5, '0', color='black', ha='center', va='center')

    plt.gca().invert_yaxis()
    # plt.gca().invert_xaxis()
    plt.yticks(rotation=0)
    plt.xticks(rotation=0)

    # Set axis labels and title
    # plt.title(title)
    # plt.xlabel('Microarchitectures')
    # plt.xticks(rotation=45, ha='right')
    plt.xlabel('Percentage of Snapshots')
    # ax.xaxis.set_label_coords(0.4, -0.28) # AVG COLUMN

    plt.tight_layout()

    plt.savefig(f'{output_base}_{suffix}.pdf')


def main(input, input2, output_folder):
    global key_a
    global key_b

    print("[+] Reading csv")

    orig_df = pd.read_csv(input, sep=";")
    orig_df = orig_df.fillna('')

    if input2:
        df2 = pd.read_csv(input2, sep=";")
        df2 = df2.fillna('')

        df2 = df2.rename(columns={"tag_match_21_13" : "tag_match_21_13_dir"})
        orig_df = pd.concat([orig_df, df2])
        orig_df = orig_df.fillna('')


    print(f"Loaded {len(orig_df)} rows")

    if 'a_orig_address' in orig_df.columns:
        output_base = os.path.join(output_folder, 'fig6_heatmap_ind_collisions')
        key_a = 'a'
        key_b = 'b'

        # Collisions across branch types not posable for pc-based collisions
        orig_df = orig_df[ orig_df['a_branch_type'] == orig_df['b_branch_type']]

    else:
        output_base = os.path.join(output_folder, 'fig9_heatmap_its_collisions')
        key_a = 'ind'
        key_b = 'dir'

        orig_df = orig_df[orig_df['full_match'] == False]
        print(f"Filtered out full match, remaining: {len(orig_df)} rows")

    primary_fields = [f'{key_a}_orig_address',f'{key_b}_orig_address', 'snapshot_id',f'{key_a}_module_name',f'{key_b}_module_name']

    duplications = orig_df[orig_df.duplicated(subset=primary_fields, keep=False)]
    print(f"Number of duplicated orig addresses: {len(duplications)} ({len(duplications) / 2} unique)")

    df = orig_df.drop_duplicates(subset=primary_fields)

    print(f"Remaining: {len(df)} rows")

    create_graph(df, output_base, 0, 0, "text_to_text", "Kernel Text Collisions")

    if key_a == 'a':
        create_graph(df, output_base, 0, 1, "text_to_module", "Kernel Text to Module Region Collisions")


if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('input')
    arg_parser.add_argument('output_folder')
    arg_parser.add_argument('--extra-input', help='optional second input, will be merged with first input')

    args = arg_parser.parse_args()
    main(args.input, args.extra_input, args.output_folder)
