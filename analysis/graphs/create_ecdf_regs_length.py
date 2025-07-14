import argparse
import scienceplots
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# line_colors = ['#0571b0', '#ca0020', '#f4a582']
line_colors = ['#000000', '#ca0020', '#f4a582']
# line_colors = ['k', 'r', 'b', 'g']

def load_datasets(data_file):

    df = pd.read_csv(data_file, delimiter = ';')
    return df



def main(data_files, output):

    dataset = load_datasets(data_files[0][0])


    plt.style.use(['science', 'ieee'])

    plt.rcParams.update({
        'figure.dpi': '300'
    })

    fig, ax1 = plt.subplots(figsize=(3.3, 1))

    columns = dataset.columns

    # Plot for the first column on the main x-axis
    x1 = np.sort(dataset[columns[0]])
    y1 = np.arange(1, x1.size + 1)
    line1, = ax1.plot(x1, y1, label='1', color=line_colors[1], ls='--')

    # ax1.grid(axis='y', which='major', linewidth = 0.1, alpha=0.3)
    ax1.set_xticks(range(1, 8 + 1))


    # Create a twin axis sharing the y-axis for the second column
    ax2 = ax1.twiny()
    x2 = np.sort(dataset[columns[1]])
    y2 = np.arange(1, x2.size + 1)
    line2, = ax2.plot(x2, y2, label='2', color=line_colors[0])

    # Setting labels
    ax1.set_ylabel('Cum. \# gadgets')
    ax1.set_xlabel('Number of Sufficiently Controlled Registers')
    ax2.set_xlabel('Number of Taken Branches')

    ax1.set_yticks([0, 75, 150])

    # Adding legends
    fig.legend([line1, line2], ['Number of Controlled Registers', 'Number of Taken Branches'], loc='lower right', bbox_to_anchor=(0.9,0.15))

    # Save figure
    fig.savefig(output)



if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser(description='Create an ECDF')

    arg_parser.add_argument('-df', '--data-file', help='path to CSV data file', required=True, nargs=1, action='append')
    arg_parser.add_argument('-o' , '--output', required=False, default='')


    args = arg_parser.parse_args()

    if not args.output:
        output = 'figures/ecdf_plot_with_secondary_xaxis.pdf'
    else:
        output = args.output

    main(args.data_file, output)
