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


def plot_ours_full(filename='rmi_build-ours_full.pdf'):
    rmi='ours'

    n_rows = len(datasets)
    n_cols = len(l1models) * len(l2models)

    configs = itertools.product(l1models, l2models)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 4.2*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, (l1, l2) in enumerate(configs):
        for row, dataset in enumerate(datasets):
            ax = axs[row,col]
            for bound in bounds:
                data = df[
                        (df['dataset']==dataset) &
                        (df['rmi']==rmi) &
                        (df['layer1']==l1) &
                        (df['layer2']==l2) &
                        (df['bounds']==bound)
                ]
                if not data.empty:
                    ax.plot(data['size_in_MiB'], data['build_in_s'], c=bound_colors[bound], marker=rmi_markers['ours'], label=bound)

            # Title
            ax.set_title(f'{dataset} ({l1}$\mapsto${l2})')

            # Labels
            if col==0:
                ax.set_ylabel('Build time [s]')
            if row==n_rows - 1:
                ax.set_xlabel('Index size [MiB]')

            # Visuals
            ax.set_ylim(bottom=0)
            ax.set_xscale('log')

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=len(bounds), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_comp_full(filename='rmi_build-comp_full.pdf'):
    n_rows = len(datasets)
    n_cols = len(l1models) * len(l2models)

    bounds = ['NB','LAbs']
    configs = itertools.product(l1models, l2models)

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(5*n_cols, 4.2*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, (l1, l2) in enumerate(configs):
        for row, dataset in enumerate(datasets):
            ax = axs[row,col]
            for rmi in rmis:
                for bound in bounds:
                    data = df[
                            (df['dataset']==dataset) &
                            (df['rmi']==rmi) &
                            (df['layer1']==l1) &
                            (df['layer2']==l2) &
                            (df['bounds']==bound)
                    ]
                    if not data.empty:
                        ax.plot(data['size_in_MiB'], data['build_in_s'], c=bound_colors[bound], marker=rmi_markers[rmi], label=f'{bound} ({rmi})')

            # Title
            ax.set_title(f'{dataset} ({l1}$\mapsto${l2})')

            # Labels
            if col==0:
                ax.set_ylabel('Build time [s]')
            if row==n_rows - 1:
                ax.set_xlabel('Index size [MiB]')

            # Visuals
            ax.set_ylim(bottom=0)
            ax.set_xscale('log')

            # Legend
            if row==0 and col==0:
                fig.legend(ncol=len(bounds), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_models(dataset, models, bound, filename):
    rmi = 'ours'

    n_cols = 1
    n_rows = 1

    fig, ax = plt.subplots(n_rows, n_cols, figsize=(2.7*n_cols, 2.3*n_rows))
    fig.tight_layout()

    for l1, l2 in models:
        data = df[
                (df['dataset']==dataset) &
                (df['rmi']==rmi) &
                (df['layer1']==l1) &
                (df['layer2']==l2) &
                (df['bounds']==bound)
        ]
        if not data.empty:
            ax.plot(data['size_in_MiB'], data['build_in_s'], c=model_colors[(l1,l2)], label=f'{l1}$\mapsto${l2}')

    # Title
    ax.set_title(f'{dataset} ({bound})')

    # Labels
    ax.set_ylabel('Build time [s]')
    ax.set_xlabel('Index size [MiB]')

    # Visuals
    ax.set_xscale('log')
    ax.set_ylim(bottom=0, top=10)

    # Legend
    fig.legend(ncol=2, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_bounds(dataset, model, filename):
    rmi = 'ours'
    l1, l2 = model

    n_cols = 1
    n_rows = 1

    fig, ax = plt.subplots(n_rows, n_cols, figsize=(2.7*n_cols, 2.3*n_rows))
    fig.tight_layout()

    for bound in bounds:
        data = df[
                (df['dataset']==dataset) &
                (df['rmi']==rmi) &
                (df['layer1']==l1) &
                (df['layer2']==l2) &
                (df['bounds']==bound)
        ]
        if not data.empty:
            ax.plot(data['size_in_MiB'], data['build_in_s'], c=bound_colors[bound], label=bound)

    # Title
    ax.set_title(f'{dataset} ({l1}$\mapsto${l2})')

    # Labels
    ax.set_ylabel('Build time [s]')
    ax.set_xlabel('Index size [MiB]')

    # Visuals
    ax.set_xscale('log')
    ax.set_ylim(bottom=0, top=10)

    # Legend
    fig.legend(ncol=3, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_comp(dataset, models, bound, filename):
    n_cols = 1
    n_rows = 1

    fig, ax = plt.subplots(n_rows, n_cols, figsize=(2.7*n_cols, 2.3*n_rows))
    fig.tight_layout()

    for rmi in rmis:
        for l1, l2 in models:
            data = df[
                    (df['dataset']==dataset) &
                    (df['rmi']==rmi) &
                    (df['layer1']==l1) &
                    (df['layer2']==l2) &
                    (df['bounds']==bound)
            ]
            if not data.empty:
                ax.plot(data['size_in_MiB'], data['build_in_s'], c=model_colors[(l1,l2)], marker=rmi_markers[rmi], label=f'{l1}$\mapsto${l2} ({rmi})')

    # Title
    ax.set_title(f'{dataset} ({bound})')

    # Labels
    ax.set_ylabel('Build time [s]')
    ax.set_xlabel('Index size [MiB]')

    # Axes
    ax.set_xscale('log')
    ax.set_ylim(bottom=0)

    # Legend
    fig.legend(ncol=2, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_build.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')

    # Compute median of lookup times
    df = df.groupby(['dataset','rmi','layer1','layer2','n_models','bounds']).median().reset_index()

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

    # Compute metrics
    df['size_in_MiB'] = df['size_in_bytes'] / (1024 * 1024)
    df['build_in_s'] = df['build_time'] / 1_000_000_000

    # Define variable lists
    datasets = sorted(df['dataset'].unique())
    rmis = sorted(df['rmi'].unique())
    bounds = sorted(df['bounds'].unique())
    l1models = sorted(df['layer1'].unique())
    l2models = sorted(df['layer2'].unique())

    # Set colors and markers
    model_colors = {}
    cmap = cm.get_cmap('tab10')
    n_colors = 10
    for i, (l1, l2) in enumerate(itertools.product(l1models, l2models)):
        model_colors[(l1,l2)] = cmap(i/n_colors)
    bound_colors = {}
    cmap = cm.get_cmap('Dark2')
    n_colors = 8
    for i, bound in enumerate(bounds):
        bound_colors[bound] = cmap(i/n_colors)
    rmi_markers = {'ours': '.', 'ref': 'x'}

    if args['paper']:
        # Plot layer1
        filename = 'rmi_build-layer1.pdf'
        print(f'Plotting build times by layer 1 to \'{filename}\'...')
        plot_models('books', [('CS','LR'),('LR','LR'),('LS','LR'),('RX','LR')], 'NB', filename)

        # Plot layer2
        filename = 'rmi_build-layer2.pdf'
        print(f'Plotting build times by layer 2 to \'{filename}\'...')
        plot_models('books', [('LS','LS'),('LS','LR'),('RX','LS'),('RX','LR')], 'NB', filename)

        # Plot bounds
        filename = 'rmi_build-bounds.pdf'
        print(f'Plotting build times by error bounds to \'{filename}\'...')
        plot_bounds('books', ('LS','LR'), filename)

        # Plot comparison NB
        filename = 'rmi_build-comp_nb.pdf'
        print(f'Plotting build time comparison to reference implementation (NB) to \'{filename}\'...')
        plot_comp('books', [('LS','LR'),('RX','LS')], 'NB', filename)

        # Plot comparison LAbs
        filename = 'rmi_build-comp_labs.pdf'
        print(f'Plotting build time comparison to reference implementation (LAbs) to \'{filename}\'...')
        plot_comp('books', [('LS','LR'),('RX','LS')], 'LAbs', filename)
    else:
        # Plot ours
        filename = 'rmi_build-ours_full.pdf'
        print(f'Plotting full build time results to \'{filename}\'...')
        plot_ours_full(filename)

        # Plot reference
        filename = 'rmi_build-comp_full.pdf'
        print(f'Plotting full build time comparison to reference implementation to \'{filename}\'...')
        plot_comp_full(filename)
