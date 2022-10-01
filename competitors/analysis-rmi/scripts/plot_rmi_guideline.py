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


def plot_guideline(filename='rmi_guideline.pdf', width_fact=5, height_fact=4.2):
    n_cols = len(datasets)
    n_rows = 1

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=False, sharex=True)
    fig.tight_layout()

    for col, dataset in enumerate(datasets):
        ax = axs[col]
        fast_lookups = list()
        fast_sizes = list()
        guide_lookups = list()
        guide_sizes = list()
        for budget in budgets:

            # Fastest configuration
            fast_confs = df[
                (df['dataset']==dataset) &
                (df['budget_in_bytes']==budget) &
                (df['is_guideline']==False)
            ]
            fast_lookup = fast_confs['lookup_in_ns'].min()
            fast_conf = fast_confs[fast_confs['lookup_in_ns']==fast_lookup]
            fast_size = fast_conf['size_in_bytes'].iloc[0]

            fast_lookups.append(fast_lookup)
            fast_sizes.append(fast_size)

            # Guideline configuration
            guide_conf = df[
                (df['dataset']==dataset) &
                (df['budget_in_bytes']==budget) &
                (df['is_guideline']==True)
            ]
            guide_lookup = guide_conf['lookup_in_ns'].iloc[0]
            guide_size = guide_conf['size_in_bytes'].iloc[0]

            guide_lookups.append(guide_lookup)
            guide_sizes.append(guide_size)

        # Plot lookup times
        ax.plot(fast_sizes, fast_lookups, marker='+', markersize=5, c=colors['fastest'], label='RMI (fastest)')
        ax.plot(guide_sizes, guide_lookups, c=colors['guideline'], linestyle='dotted', label='RMI (guideline)')

        # Title
        ax.set_title(f'{dataset}')

        # Labels
        if col==0:
            ax.set_ylabel('Lookup time [ns]')
        ax.set_xlabel('Index size [MiB]')

        # Visuals
        ax.set_xscale('log')
        if col==n_cols-1:
            ax.set_ylim(bottom=0)

        # Legend
        if col==0:
            fig.legend(ncol=2, bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_guideline.csv')
    df = pd.read_csv(file, delimiter=',', header=0, comment='#')

    # Compute median of lookup times
    df = df.groupby(['dataset','layer1','layer2','n_models','bounds','search','is_guideline']).median().reset_index()

    # Replace datasets
    dataset_dict = {
        "books_200M_uint64": "books",
        "fb_200M_uint64": "fb",
        "osm_cellids_200M_uint64": "osmc",
        "wiki_ts_200M_uint64": "wiki"
    }
    df.replace({**dataset_dict}, inplace=True)

    # Compute metrics
    df['size_in_MiB'] = df['size_in_bytes'] / (1024 * 1024)
    df['lookup_in_ns'] = df['lookup_time'] / df['n_samples']

    # Define varibale lists
    datasets = sorted(df['dataset'].unique())
    budgets = sorted(df['budget_in_bytes'].unique())

    # Set colors
    colors = {}
    cmap = cm.get_cmap('tab10')
    n_colors = 8
    for i, x in enumerate(['fastest', 'guideline']):
        colors[x] = cmap((i)/n_colors)

    if args['paper']:
        # Plot guideline
        filename = 'rmi_guideline.pdf'
        print(f'Plotting guideline to \'{filename}\'...')
        plot_guideline(filename, 2.7, 2)
    else:
        # Plot guideline
        filename = 'rmi_guideline.pdf'
        print(f'Plotting guideline to \'{filename}\'...')
        plot_guideline(filename)
