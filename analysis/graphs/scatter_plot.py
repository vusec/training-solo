import argparse
import scienceplots
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

line_colors = ['#000000', '#ca0020', '#f4a582']

def load_datasets(data_file):
    print(data_file)
    df = pd.read_csv(data_file, delimiter=';')
    return df

def main(data_files, output):
    dataset = load_datasets(data_files[0][0])

    plt.style.use(['science', 'ieee'])
    plt.rcParams.update({'figure.dpi': '300'})

    fig, ax = plt.subplots(figsize=(3.3, 1))

    # Scatter plot with "Number of Registers Sufficiently Controlled" on x-axis
    # and "Number of Controlled Flow Changes" on y-axis
    ax.scatter(
        dataset['Number of Registers Sufficiently Controlled'],
        dataset['Number of Controlled Flow Changes'],
        edgecolor='k', s=30, alpha=0.7,
        color='#1f77b4'
        # color=line_colors[1], alpha=0.7, edgecolor='k', s=30
    )

    # Setting labels
    ax.set_xlabel('Number of Sufficiently Controlled Registers')
    ax.set_ylabel('\# Taken Branches')
    # ax.grid(axis='y', which='major', linewidth=0.1, alpha=0.3)
    ax.set_xticks(range(1, 8 + 1))
    ax.set_yticks(range(0, 75 + 1, 25))

    # Save figure
    fig.savefig(output)

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser(description='Create a Scatter Plot')
    arg_parser.add_argument('-df', '--data-file', help='path to CSV data file', required=True, nargs=1, action='append')
    arg_parser.add_argument('-o', '--output', required=False, default='figures/scatter_plot_correlation.pdf')

    args = arg_parser.parse_args()
    main(args.data_file, args.output)
