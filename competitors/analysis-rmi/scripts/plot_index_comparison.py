#!python3
import argparse
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import numpy as np
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


def compute_pareto_frontier(source, cost, value):
    frontier = list()
    source = source.sort_values(cost)
    old_val = float('inf')
    for index, row in source.iterrows():
        curr_val = row[value]
        if curr_val < old_val:
            old_val = curr_val
            frontier.append(row)
    result = pd.DataFrame(frontier)
    return result


def plot_lookup(filename='index_comparison-lookup_time.pdf', width_fact=5, height_fact=4.2):
    n_rows = 2
    n_cols = 2

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for i, dataset in enumerate(datasets):
        row = int(i / 2)
        col = int(i % 2)
        ax = axs[row,col]

        # Scatter indexes
        for index in index_dict.keys():
            data = df[
                (df['dataset']==dataset) &
                (df['index']==index)
            ]
            if not data.empty and index!='Binary search':
                if index=='Compact Hist-Tree' or index=='RadixSpline':
                    data = compute_pareto_frontier(data, 'size_in_MiB', 'lookup_in_ns')
                    data = data.sort_values('size_in_MiB')
                    ax.plot(data['size_in_MiB'], data['lookup_in_ns'], color=colors[index], label=index_dict[index], alpha=0.9)
                else:
                    data = data.sort_values('size_in_MiB')
                    ax.plot(data['size_in_MiB'], data['lookup_in_ns'], color=colors[index], label=index_dict[index], alpha=0.9)

        # Title
        ax.set_title(dataset)

        # Labels
        if row==n_rows - 1:
            ax.set_xlabel('Index size [MiB]')
        if col==0:
            ax.set_ylabel('Lookup time [ns]')

        # Visuals
        ax.set_xscale('log')
        ax.set_ylim(bottom=0, top=1250)

        # Legend
        if row==0 and col==0:
            fig.legend(ncol=4, bbox_to_anchor=(0.5, 1), loc='lower center')

        # Binary search
        if True:
            data = df[
                (df['dataset']==dataset) &
                (df['index']=='Binary search')
            ].iloc[0]
            ax.axhline(y=data['lookup_in_ns'], marker='None', color='.2', dashes=(2, 1), label='Binary search')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_build(filename='index_comparison-build_time.pdf', width_fact=5, height_fact=4.2):
    n_cols = 2
    n_rows = 2

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for i, dataset in enumerate(datasets):
        row = int(i / 2)
        col = int(i % 2)
        ax = axs[row,col]

        # Scatter indexes
        for index in index_dict.keys():
            data = df[
                (df['dataset']==dataset) &
                (df['index']==index)
            ]
            if not data.empty and index!='Binary search':
                if index=='Compact Hist-Tree' or index=='RadixSpline':
                    data = compute_pareto_frontier(data, 'size_in_MiB', 'lookup_in_ns')
                    data = data.sort_values('size_in_MiB')
                    ax.plot(data['size_in_MiB'], data['build_in_s'], color=colors[index], label=index_dict[index], alpha=0.9)
                else:
                    data = data.sort_values('size_in_MiB')
                    ax.plot(data['size_in_MiB'], data['build_in_s'], color=colors[index], label=index_dict[index], alpha=0.9)

        # Title
        ax.set_title(dataset)

        # Labels
        if row==n_rows - 1:
            ax.set_xlabel('Index size [MiB]')
        if col==0:
            ax.set_ylabel('Build time [s]')

        # Visuals
        ax.set_xscale('log')
        ax.set_ylim(bottom=-1, top=30)

        # Legend
        if row==0 and col==0:
            fig.legend(ncol=4, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_lookup_shares(filename, width_fact=5, height_fact=4.2):
    n_cols = len(datasets)
    n_rows = 1

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=True, sharex=False)
    fig.tight_layout()

    for col, dataset in enumerate(datasets):
        ax = axs[col]

        # Gather data of fastest configuration per index
        labels = []
        evals = []
        searches = []
        bar_colors = []

        # Binary search
        row = df[
            (df['dataset']==dataset) &
            (df['index']=='Binary search')
        ].iloc[0]
        labels.append('Binary search')
        evals.append(0)
        searches.append(row['lookup_in_ns'])
        bar_colors.append('.2')

        for index in index_dict.keys():
            data = df[
                (df['dataset'] == dataset) &
                (df['index'] == index)
            ]
            labels.append(index_dict[index])
            bar_colors.append(colors[index])
            if not data.empty:
                data = data.sort_values(['lookup_in_ns']).reset_index()
                row = data.iloc[0] # fastest configuration
                evals.append(row['eval_in_ns'])
                searches.append(row['search_in_ns'])
            else:
                evals.append(0)
                searches.append(0)

        # Plot results
        ax.bar(labels, evals, color='0.6', edgecolor=bar_colors, linewidth=1, label='Evaluation')
        ax.bar(labels, searches, color='0.9', edgecolor=bar_colors, linewidth=1, label='Search', bottom=evals)

        # Title
        ax.set_title(dataset)

        # Labels
        if col==0:
            ax.set_ylabel('Lookup time [ns]')

        # Visuals
        ax.grid(False, axis='x')
        ax.set_xticklabels(labels=labels, rotation=90)

        # Legend
        if col==0:
            fig.legend(ncol=2, bbox_to_anchor=(0.5, 1), loc='lower center')

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'index_comparison.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')
    df = df.replace({np.nan: '-'})

    # Compute medians
    df = df.groupby(['dataset', 'index', 'config']).median().reset_index()

    # Replace datasets
    dataset_dict = {
        "books_200M_uint64": "books",
        "fb_200M_uint64": "fb",
        "osm_cellids_200M_uint64": "osmc",
        "wiki_ts_200M_uint64": "wiki"
    }
    df.replace({**dataset_dict}, inplace=True)
    index_dict = {
        'RMI-ours': 'RMI (ours)',
        'RMI-ref': 'RMI (ref)',
        'ALEX': 'ALEX',
        'PGM-index': 'PGM-index',
        'RadixSpline': 'RadixSpline',
        'Compact Hist-Tree': 'Hist-Tree',
        'B-tree': 'B-tree',
        'ART': 'ART'
    }

    # Compute metrics
    df['size_in_MiB'] = df['size_in_bytes'] / (1024 * 1024)
    df['build_in_s'] = df['build_time'] / 1000000000
    df['eval_in_ns'] = df['eval_time'] / df['n_samples']
    df['lookup_in_ns'] = df['lookup_time'] / df['n_samples']
    df['search_in_ns'] = df['lookup_in_ns'] - df['eval_in_ns']

    # Define variable lists
    datasets = sorted(df['dataset'].unique())
    indexes = sorted(df['index'].unique())

    # Set colors
    cmap = cm.get_cmap('tab10')
    n_colors = 10
    colors = {}
    for i, index in enumerate(index_dict.keys()):
        colors[index] = cmap(i/n_colors)

    if args['paper']:
        # Plot lookup times against index size
        filename = 'index_comparison-lookup_time.pdf'
        print(f'Plotting lookup time results to \'{filename}\'...')
        plot_lookup(filename, 4, 2.7)

        # Plot build times against index size
        filename = 'index_comparison-build_time.pdf'
        print(f'Plotting build time results to \'{filename}\'...')
        plot_build(filename, 4, 2.7)

        # Plot share of eval time and search time in overall lookup time
        filename = 'index_comparison-lookup_shares.pdf'
        print(f'Plotting lookup time shares to \'{filename}\'...')
        plot_lookup_shares(filename, 2.1, 2)
    else:
        # Plot lookup times against index size
        filename = 'index_comparison-lookup_time.pdf'
        print(f'Plotting lookup time results to \'{filename}\'...')
        plot_lookup(filename)

        # Plot build times against index size
        filename = 'index_comparison-build_time.pdf'
        print(f'Plotting build time results to \'{filename}\'...')
        plot_build(filename)

        # Plot share of eval time and search time in overall lookup time
        filename = 'index_comparison-lookup_shares.pdf'
        print(f'Plotting lookup time shares to \'{filename}\'...')
        plot_lookup_shares(filename)
