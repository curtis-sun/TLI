#!bash
# set -x
trap "exit" SIGINT

echo "Plotting RMI Segmentation (Section 5.1)..."
python3 scripts/plot_rmi_segmentation.py --paper

echo "Plotting RMI Errors (Section 5.2)..."
python3 scripts/plot_rmi_errors.py --paper

echo "Plotting RMI Intervals (Section 5.3)..."
python3 scripts/plot_rmi_intervals.py --paper

echo "Plotting RMI Lookup (Section 6)..."
python3 scripts/plot_rmi_lookup.py --paper

echo "Plotting RMI Build (Section 7)..."
python3 scripts/plot_rmi_build.py --paper

echo "Plotting RMI Guideline (Section 8)..."
python3 scripts/plot_rmi_guideline.py --paper

echo "Plotting Index Comparison (Section 9)..."
python3 scripts/plot_index_comparison.py --paper
