#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="rmi intervals"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/rmi_intervals.csv"

BIN="build/bin/rmi_intervals"

run() {
    DATASET=$1
    LAYER1=$2
    LAYER2=$3
    N_MODELS=$4
    BOUND=$5
    DATA_FILE="${DIR_DATA}/${DATASET}"
    ${BIN} ${DATA_FILE} ${LAYER1} ${LAYER2} ${N_MODELS} ${BOUND} >> ${FILE_RESULTS}
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
LAYERS1="linear_spline cubic_spline linear_regression radix"
LAYERS2="linear_spline linear_regression"
BOUNDS="gabs gind labs lind"

# Run experiments
echo "dataset,n_keys,layer1,layer2,n_models,bounds,size_in_bytes,mean_interval,median_interval,stdev_interval,min_interval,max_interval" > ${FILE_RESULTS} # Write csv header
for dataset in ${DATASETS};
do
    echo "Performing ${EXPERIMENT} on '${dataset}'..."
    for ((i=6; i<=25; i += 1));
    do
        n_models=$((2**$i))
        for l1 in ${LAYERS1};
        do
            for l2 in ${LAYERS2};
            do
                for bound in ${BOUNDS};
                do
                    run ${dataset} ${l1} ${l2} ${n_models} ${bound}
                done
            done
        done
    done
done

