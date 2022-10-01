#!python3
import argparse
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import os
import pandas as pd
import warnings

plt.style.use(os.path.join('scripts', 'matplotlibrc'))

# Ignore warnings
warnings.filterwarnings( "ignore")

# Argparse
parser = argparse.ArgumentParser()
parser.add_argument('-p', '--paper', help='produce paper plots', action='store_true')
args = vars(parser.parse_args())


def plot(x, y, xlabel, ylabel, filename):
    n_cols = len(configs)
    n_rows = len(datasets)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 4.2*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for row, dataset in enumerate(datasets):
        for col, config in enumerate(configs):
            ax = axs[row,col]
            for bound in bounds:
                data = df[
                    (df['dataset']==dataset) &
                    (df['config']==config) &
                    (df['bounds']==bound)
                ]
                if not data.empty:
                    ax.plot(data[x], data[y], label=bound, color=colors[bound])

            # Title
            ax.set_title(f'{dataset} ({config})')

            # Labels
            if row==n_rows - 1:
                ax.set_xlabel(xlabel)
            if col==0:
                ax.set_ylabel(ylabel)

            # Visuals
            ax.set_xscale('log')
            ax.set_yscale('log')

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=len(bounds), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_paper(x, y, xlabel, ylabel, filename):
    configs = ['LS$\mapsto$LR', 'RX$\mapsto$LS']
    datasets = ['books', 'osmc', 'wiki']

    n_cols = len(configs)
    n_rows = len(datasets)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(4*n_cols, 2.7*n_rows), sharey='row', sharex=True)
    fig.tight_layout()

    for row, dataset in enumerate(datasets):
        for col, config in enumerate(configs):
            ax = axs[row,col]
            for bound in bounds:
                data = df[
                    (df['dataset']==dataset) &
                    (df['config']==config) &
                    (df['bounds']==bound)
                ]
                if not data.empty:
                    ax.plot(data[x], data[y], label=bound, color=colors[bound])

            # Title
            ax.set_title(f'{dataset} ({config})')

            # Labels
            if row==n_rows - 1:
                ax.set_xlabel(xlabel)
            if col==0:
                ax.set_ylabel(ylabel)

            # Visuals
            ax.set_xscale('log')
            ax.set_yscale('log')
            if col==n_cols - 1:
                ax.set_ylim(bottom=4, top=None)

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=len(bounds), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_intervals.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')

    # Replace datasets, model names, and bounds
    dataset_dict = {
        "books_200M_uint64": "books",
        "fb_200M_uint64": "fb",
        "osm_cellids_200M_uint64": "osmc",
        "wiki_ts_200M_uint64": "wiki"
    }
    model_dict = {
        "cubic_spline": "CS",
        "linear_spline": "LS",
        "linear_regression": "LR",
        "radix": "RX"
    }
    bounds_dict = {
        "labs": "LAbs",
        "lind": "LInd",
        "gabs": "GAbs",
        "gind": "GInd",
        "none": "NB"
    }
    df.replace({**dataset_dict, **model_dict, **bounds_dict}, inplace=True)

    # Compute model combinations and metrics
    df['config'] = df['layer1'] + '$\mapsto$' + df['layer2']
    df['size_in_MiB'] = df['size_in_bytes'] / (1024 * 1024)

    # Define varibale lists
    datasets = sorted(df['dataset'].unique())
    configs = sorted(df['config'].unique())
    bounds = sorted(df['bounds'].unique())

    # Set colors
    colors = {}
    cmap = cm.get_cmap('Dark2')
    n_colors = 8
    for i, bound in enumerate(bounds):
        colors[bound] = cmap(i/n_colors)

    if args['paper']:
        #  Plot median interval size
        filename = 'rmi_intervals-median_interval.pdf'
        print(f'Plotting median interval to \'{filename}\'...')
        plot_paper('size_in_MiB', 'median_interval', 'Index size [MiB]', 'Median search\ninterval size', filename)
    else:
        #  Plot mean interval size
        filename = 'rmi_intervals-mean_interval.pdf'
        print(f'Plotting mean interval to \'{filename}\'...')
        plot('size_in_MiB', 'mean_interval', 'Index size [MiB]', 'Mean search\ninterval size', filename)

        #  Plot median interval size
        filename = 'rmi_intervals-median_interval.pdf'
        print(f'Plotting median interval to \'{filename}\'...')
        plot('size_in_MiB', 'median_interval', 'Index size [MiB]', 'Median search\ninterval size', filename)

        #  Plot max interval size
        filename = 'rmi_intervals-max_interval.pdf'
        print(f'Plotting max interval to \'{filename}\'...')
        plot('size_in_MiB', 'max_interval', 'Index size [MiB]', 'Max search\ninterval size', filename)
