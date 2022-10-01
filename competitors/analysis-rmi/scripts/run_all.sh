#!bash
# set -x
trap "exit" SIGINT

echo "Running RMI Segmentation (Section 5.1)..."
source scripts/run_rmi_segmentation.sh

echo "Running RMI Errors (Section 5.2)..."
source scripts/run_rmi_errors.sh

echo "Running RMI Intervals (Section 5.3)..."
source scripts/run_rmi_intervals.sh

echo "Running RMI Lookup (Section 6)..."
source scripts/run_rmi_lookup.sh

echo "Running RMI Build (Section 7)..."
source scripts/run_rmi_build.sh

echo "Running RMI Guideline (Section 8)..."
source scripts/run_rmi_guideline.sh

echo "Running Index Comparison (Section 9)..."
source scripts/run_index_comparison.sh
