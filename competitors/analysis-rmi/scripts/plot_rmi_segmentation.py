#!python3
import argparse
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import matplotlib.ticker as mtick
import os
import pandas as pd
import warnings

plt.style.use(os.path.join('scripts','matplotlibrc'))

# Ignore warnings
warnings.filterwarnings( "ignore")

# Argparse
parser = argparse.ArgumentParser()
parser.add_argument('-p', '--paper', help='produce paper plots', action='store_true')
args = vars(parser.parse_args())


def plot_frac_empty(filename='rmi_segmentation-frac_empty.pdf', width_fact=5, height_fact=4.2):
    n_cols = len(datasets)
    n_rows = 1

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, dataset in enumerate(datasets):
        ax = axs[col]
        for model in models:
            data = df[
                    (df['dataset']==dataset) &
                    (df['model']==model)
            ]
            if not data.empty:
                ax.plot(data['n_segments'], data['frac_empty'], label=model, c=colors[model])

        # Title
        ax.set_title(dataset)

        # Labels
        if col==0:
            ax.set_ylabel('Percentage of\nempty segments')
        ax.set_xlabel('# of segments')

        # Visuals
        ax.yaxis.set_major_formatter(mtick.PercentFormatter(1.0))
        ax.set_xscale('log', base=2)
        ax.set_xticks([2**12, 2**20])

        # Legend
        if col==0:
            fig.legend(ncol=len(models), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')


def plot_max_segment(filename='rmi_segmentation-max_segment.pdf', width_fact=5, height_fact=4.2):
    n_cols = len(datasets)
    n_rows = 1

    fig, axs = plt.subplots(n_rows, n_cols, figsize=(width_fact*n_cols, height_fact*n_rows), sharey=True, sharex=True)
    fig.tight_layout()

    for col, dataset in enumerate(datasets):
        ax = axs[col]
        for model in models:
            data = df[
                    (df['dataset']==dataset) &
                    (df['model']==model)
            ]
            if not data.empty:
                ax.plot(data['n_segments'], data['max'], label=model, c=colors[model])

        # Title
        ax.set_title(dataset)

        # Labels
        if col==0:
            ax.set_ylabel('Size of largest\nsegment')
        ax.set_xlabel('# of segments')

        # Visuals
        ax.set_yscale('log')
        ax.set_xscale('log', base=2)
        ax.set_yticks([10**2, 10**4, 10**6, 10**8])
        ax.set_xticks([2**12, 2**20])

        # Legend
        if col==0:
            fig.legend(ncol=len(models), bbox_to_anchor=(0.5, 1), loc='lower center', frameon=False)

    fig.savefig(os.path.join(path, filename), bbox_inches='tight')

if __name__ == "__main__":
    path = 'results'

    # Read csv file
    file = os.path.join(path, 'rmi_segmentation.csv')
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

    # Compute metrics
    df['frac_empty'] = df['n_empty'] / df['n_segments']

    # Define variable lists
    datasets = sorted(df['dataset'].unique())
    models = sorted(df['model'].unique())

    # Set colors
    cmap = cm.get_cmap('tab20b')
    n_colors = 5
    colors = {}
    for i, model in enumerate(models):
        colors[model] = cmap(i/n_colors+0.1)

    if args['paper']:
        # Plot empty segments
        filename = 'rmi_segmentation-frac_empty.pdf'
        print(f'Plotting empty segments to \'{filename}\'...')
        plot_frac_empty(filename, 2, 1.8)

        # Plot max segment
        filename = 'rmi_segmentation-max_segment.pdf'
        print(f'Plotting max segments to \'{filename}\'...')
        plot_max_segment(filename, 2, 1.8)
    else:
        # Plot empty segments
        filename = 'rmi_segmentation-frac_empty.pdf'
        print(f'Plotting empty segments to \'{filename}\'...')
        plot_frac_empty(filename)

        # Plot max segment
        filename = 'rmi_segmentation-max_segment.pdf'
        print(f'Plotting max segments to \'{filename}\'...')
        plot_max_segment(filename)
