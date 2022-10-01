#!python3
import argparse
import itertools
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
    n_cols = len(datasets)
    n_rows = 1

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 4.2*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, dataset in enumerate(datasets):
        ax = axs[col]
        for l1 in l1_models:
            for l2 in l2_models:
                data = df[
                    (df['dataset']==dataset) &
                    (df['layer1']==l1) &
                    (df['layer2']==l2)
                ]
                if not data.empty:
                    ax.plot(data[x], data[y], label=f'{l1}$\mapsto${l2}', color=colors[(l1,l2)])

        # Title
        ax.set_title(dataset)

        # Labels
        if col==0:
            ax.set_xlabel(xlabel)
        ax.set_ylabel(ylabel)

        # Visuals
        ax.set_xscale('log', base=2)
        ax.set_yscale('log')

        # Legend
        if col==0:
            fig.legend(ncol=len(l1_models)*len(l2_models), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_paper(x, y, xlabel, ylabel, filename):
    l1_groups = [['CS','LR'],['LS','RX']]
    l2_models = ['LR','LS']

    n_cols = len(l1_groups)
    n_rows = len(datasets)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(4*n_cols, 2.7*n_rows), sharey='row', sharex=True)
    fig.tight_layout()

    for row, dataset in enumerate(datasets):
        for col, l1_models in enumerate(l1_groups):
            ax = axs[row,col]

            for l1 in l1_models:
                for l2 in l2_models:
                    data = df[
                        (df['dataset']==dataset) &
                        (df['layer1']==l1) &
                        (df['layer2']==l2)
                    ]
                    if not data.empty:
                        ax.plot(data[x], data[y], label=f'{l1}$\mapsto${l2}', color=colors[(l1,l2)])

            # Title
            ax.set_title(dataset)

            # Labels
            if row==n_rows - 1:
                ax.set_xlabel(xlabel)
            if col==0:
                ax.set_ylabel(ylabel)

            # Visuals
            ax.set_xscale('log', base=2)
            ax.set_yscale('log')
            if dataset in ['fb','osmc']:
                ax.set_ylim(bottom=0.7)

            # Legend
            if row==0 and col==n_cols - 1:
                fig.legend(ncol=4, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')

if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_errors.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')

    # Replace datasets and model names
    dataset_dict = {
        "books_200M_uint64": "books",
        "fb_200M_uint64": "fb",
        "osm_cellids_200M_uint64": "osmc",
        "wiki_ts_200M_uint64": "wiki"
    }
    model_dict = {
        "linear_regression": "LR",
        "linear_spline": "LS",
        "cubic_spline": "CS",
        "radix": "RX"
    }
    df.replace({**dataset_dict, **model_dict}, inplace=True)

    # Define variable lists
    datasets = sorted(df['dataset'].unique())
    l1_models = sorted(df['layer1'].unique())
    l2_models = sorted(df['layer2'].unique())

    # Set colors
    colors = {}
    cmap = cm.get_cmap('tab10')
    n_colors = 10
    for i, (l1, l2) in enumerate(itertools.product(l1_models, l2_models)):
        colors[(l1,l2)] = cmap(i/n_colors)

    if args['paper']:
        # Plot median absolute error
        filename = 'rmi_errors-median_absolute_error.pdf'
        print(f'Plotting median absolute error to \'{filename}\'...')
        plot_paper('n_models', 'median_ae', '# of segments', 'Median absolute error', filename)
    else:
        # Plot median absolute error
        filename = 'rmi_errors-median_absolute_error.pdf'
        print(f'Plotting median absolute error to \'{filename}\'...')
        plot('n_models', 'median_ae', '# of segments', 'Median absolute error', filename)

        # Plot mean absolute error
        filename = 'rmi_errors-mean_absolute_error.pdf'
        print(f'Plotting mean absolute error to \'{filename}\'...')
        plot('n_models', 'mean_ae', '# of segments', 'Mean absolute error', filename)

        # Plot max absolute error
        filename = 'rmi_errors-max_absolute_error.pdf'
        print(f'Plotting max absolute error to \'{filename}\'...')
        plot('n_models', 'max_ae', '# of segments', 'Maximum absolute error', filename)
