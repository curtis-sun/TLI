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


def plot_full(filename='rmi_lookup-full.pdf'):
    n_rows = len(datasets)
    n_cols = len(l1models) * len(l2models)

    configs = itertools.product(l1models, l2models)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 4.2*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, (l1, l2) in enumerate(configs):
        for row, dataset in enumerate(datasets):
            ax = axs[row,col]
            for bound in bounds:
                for search in searches:
                    data = df[
                            (df['dataset']==dataset) &
                            (df['layer1']==l1) &
                            (df['layer2']==l2) &
                            (df['bounds']==bound) &
                            (df['search']==search)
                    ]
                    if not data.empty:
                        ax.plot(data['size_in_MiB'], data['lookup_in_ns'], label=f'{bound}+{search}', color=corr_colors[(bound,search)])

            # Title
            ax.set_title(f'{dataset} ({l1}$\mapsto${l2})')

            # Labels
            if row==n_rows-1:
                ax.set_xlabel('Index size [MiB]')
            if col==0:
                ax.set_ylabel('Lookup time [ns]')

            # Visuals
            ax.set_ylim(bottom=0)
            ax.set_xscale('log')

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=len(corr_configs), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_models(filename='rmi_lookup-model_types.pdf', bounds='NB', search='MExp'):
    l1_groups = [['CS','LR'],['LS','RX']]
    l2_models = ['LR','LS']

    n_rows = len(datasets)
    n_cols = len(l1_groups)

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
                        (df['layer2']==l2) &
                        (df['bounds']==bounds) &
                        (df['search']==search)
                    ]
                    if not data.empty:
                        ax.plot(data['size_in_MiB'], data['lookup_in_ns'], label=f'{l1}$\mapsto${l2}', c=model_colors[(l1,l2)])

            # Title
            ax.set_title(dataset)

            # Labels
            if col==0:
                ax.set_ylabel('Lookup time [ns]')
            if row==n_rows - 1:
                ax.set_xlabel('Index size [MiB]')

            # Visuals
            ax.set_xscale('log')
            if col==n_cols - 1:
                if dataset=='books':
                    ax.set_ylim(bottom=0, top=850)
                elif dataset=='fb':
                    ax.set_ylim(bottom=0, top=1850)
                elif dataset=='osmc':
                    ax.set_ylim(bottom=0, top=1500)
                elif dataset=='wiki':
                    ax.set_ylim(bottom=0, top=1000)

            # Legend
            if row==0 and col==n_cols - 1:
                fig.legend(ncol=4, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_correction(filename='rmi_lookup-error_correction.pdf'):
    models = [('LS', 'LR'),('RX','LS')]
    datasets = ['books','osmc','wiki']

    n_rows = len(datasets)
    n_cols = len(models)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(4*n_cols, 2.7*n_rows), sharey='row', sharex=True)
    fig.tight_layout()

    for row, dataset in enumerate(datasets):
        for col, model in enumerate(models):
            ax = axs[row,col]
            l1, l2 = model
            for bound in bounds:
                for search in searches:
                    data = df[
                            (df['dataset']==dataset) &
                            (df['layer1']==l1) &
                            (df['layer2']==l2) &
                            (df['bounds']==bound) &
                            (df['search']==search)
                    ]
                    if not data.empty:
                        ax.plot(data['size_in_MiB'], data['lookup_in_ns'], label=f'{bound}+{search}', color=corr_colors[(bound,search)])

            # Title
            ax.set_title(f'{dataset} ({l1}$\mapsto${l2})')

            # Labels
            if row==n_rows-1:
                ax.set_xlabel('Index size [MiB]')
            if col==0:
                ax.set_ylabel('Lookup time [ns]')

            # Visuals
            ax.set_xscale('log')
            if col==n_cols - 1:
                if dataset=='books':
                    ax.set_ylim(bottom=0, top=850)
                elif dataset=='osmc':
                    ax.set_ylim(bottom=0, top=1400)
                elif dataset=='wiki':
                    ax.set_ylim(bottom=0, top=1000)

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=4, bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_lookup.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')

    # Compute median of lookup times
    df = df.groupby(['dataset','layer1','layer2','n_models','bounds','search']).median().reset_index()

    # Replace datasets, model names, bounds, and searches
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
    search_dict = {
        "binary": "Bin",
        "model_biased_binary": "MBin",
        "model_biased_exponential": "MExp",
        "model_biased_linear": "MLin"
    }
    df.replace({**dataset_dict, **model_dict, **bounds_dict, **search_dict}, inplace=True)

    # Compute metrics
    df['size_in_MiB'] = df['size_in_bytes'] / (1024 * 1024)
    df['lookup_in_ns'] = df['lookup_time'] / df['n_samples']

    # Define variable lists
    datasets = sorted(df['dataset'].unique())
    bounds = sorted(df['bounds'].unique())
    searches = sorted(df['search'].unique())
    l1models = sorted(df['layer1'].unique())
    l2models = sorted(df['layer2'].unique())
    corr_configs = [
        ('GAbs','Bin'),
        ('GInd','Bin'),('GInd','MBin'),
        ('LAbs','Bin'),
        ('LInd','Bin'),('LInd','MBin'),
        ('NB','MExp'),('NB','MLin'),
    ]

    # Set colors
    model_colors = {}
    cmap = cm.get_cmap('tab10')
    n_colors = 10
    for i, (l1, l2) in enumerate(itertools.product(l1models, l2models)):
        model_colors[(l1,l2)] = cmap(i/n_colors)
    corr_colors = {}
    cmap = cm.get_cmap('Dark2')
    n_colors = len(corr_configs)
    for i, (bound, search) in enumerate(corr_configs):
        corr_colors[(bound,search)] = cmap(i/n_colors)

    if args['paper']:
        # Plot model types
        filename = 'rmi_lookup-model_types.pdf'
        print(f'Plotting lookup time by model types to \'{filename}\'...')
        plot_models(filename)

        # Plot error correction
        filename = 'rmi_lookup-error_correction.pdf'
        print(f'Plotting lookup time by error correction to \'{filename}\'...')
        plot_correction(filename)
    else:
        # Plot full results
        filename = 'rmi_lookup-full.pdf'
        print(f'Plotting full lookup time results to \'{filename}\'...')
        plot_full(filename)
