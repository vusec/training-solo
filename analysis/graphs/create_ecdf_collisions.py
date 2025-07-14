import argparse
import scienceplots
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

# line_colors = ['#000000', '#0571b0', '#ca0020', '#f4a582']
line_colors = ['#000000', '#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']

def load_datasets(data_files):

    datasets = []

    for file, name in data_files:
        df = pd.read_csv(file, delimiter = ';')
        datasets.append({
            "name" : name,
            "data": df
        })

    return datasets


def main(data_files : dict, output):

    datasets = load_datasets(data_files)

    plt.style.use(['science', 'ieee'])

    plt.rcParams.update({
        'figure.dpi': '300'
    })


    for column in datasets[0]['data'].columns:

        max_x_value = max(data['data'][column].max() for data in datasets)

        print(max_x_value)


        if column == 'label':
            continue

        fig, ax = plt.subplots(figsize=(3.3, 1))

        for idx, data in enumerate(datasets):

            x = np.sort(data['data'][column])

            if x[-1] < max_x_value:
                x = np.append(x, max_x_value)

            y = np.arange(1, x.size+1)
            ax.plot(x, y, label=data['name'], color=line_colors[idx % len(line_colors)])

        ax.legend(loc = "upper left", bbox_to_anchor=(-0.1, 1.4), ncol=4, handletextpad=0.5, columnspacing=1)
        ax.autoscale(tight=True)
        ax.set_xlabel(column)

        ax.set_ylabel('Cum. \# gadgets')



        fig.savefig(f'{output}{column.replace(" ", "_").lower()}.pdf')


    return

if __name__ == '__main__':

    arg_parser = argparse.ArgumentParser(description='Create an ECDF')

    arg_parser.add_argument('-df', '--data-file', help='path to CSV data file followed by the data name', required=True, nargs=2, action='append')


    arg_parser.add_argument('-o' , '--output', required=False, default='')


    args = arg_parser.parse_args()

    if not args.output:
        output = 'figures/ecdf_'
    else:
        output = args.output

    main(args.data_file, output)
