#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="rmi segmentation"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/rmi_segmentation.csv"

BIN="build/bin/rmi_segmentation"

run() {
    DATASET=$1
    MODEL=$2
    N_SEGMENTS=$3
    DATA_FILE="${DIR_DATA}/${DATASET}"
    ${BIN} ${DATA_FILE} ${MODEL} ${N_SEGMENTS} >> ${FILE_RESULTS}
}

# Create results directory
if [ ! -d "${DIR_RESULTS}" ];
then
    mkdir -p "${DIR_RESULTS}";
fi

# Check data downloaded
if [ ! -d "${DIR_DATA}" ];
then
    >&2 echo "Please download datasets first."
    return 1
fi

DATASETS="books_200M_uint64 fb_200M_uint64 osm_cellids_200M_uint64 wiki_ts_200M_uint64"
MODELS="linear_spline cubic_spline linear_regression radix"

# Run experiments
echo "dataset,n_keys,model,n_segments,mean,stdev,median,min,max,n_empty" > ${FILE_RESULTS} # Write csv header
for dataset in ${DATASETS};
do
    echo "Performing ${EXPERIMENT} on '${dataset}'..."
    for model in ${MODELS};
    do
        for ((i=6; i<=25; i += 1));
        do
            n_segments=$((2**$i))
            run ${dataset} ${model} ${n_segments}
        done
    done
done

