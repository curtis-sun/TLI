#!bash
# set -x
trap "exit" SIGINT

echo "Plotting RMI Segmentation (Section 5.1)..."
python3 scripts/plot_rmi_segmentation.py

echo "Plotting RMI Errors (Section 5.2)..."
python3 scripts/plot_rmi_errors.py

echo "Plotting RMI Intervals (Section 5.3)..."
python3 scripts/plot_rmi_intervals.py

echo "Plotting RMI Lookup (Section 6)..."
python3 scripts/plot_rmi_lookup.py

echo "Plotting RMI Build (Section 7)..."
python3 scripts/plot_rmi_build.py

echo "Plotting RMI Guideline (Section 8)..."
python3 scripts/plot_rmi_guideline.py

echo "Plotting Index Comparison (Section 9)..."
python3 scripts/plot_index_comparison.py
