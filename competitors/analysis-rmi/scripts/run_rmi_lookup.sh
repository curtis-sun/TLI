#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="rmi lookup"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/rmi_lookup.csv"

BIN="build/bin/rmi_lookup"

# Set number of repetitions and samples
N_REPS="3"
N_SAMPLES="20000000"
PARAMS="--n_reps ${N_REPS} --n_samples ${N_SAMPLES}"
TIMEOUT="90s"

DATASETS="books_200M_uint64 fb_200M_uint64 osm_cellids_200M_uint64 wiki_ts_200M_uint64"
LAYER1="cubic_spline linear_spline linear_regression radix"
LAYER2="linear_spline linear_regression"

run() {
    DATASET=$1
    L1=$2
    L2=$3
    N_MODELS=$4
    BOUND=$5
    SEARCH=$6
    DATA_FILE="${DIR_DATA}/${DATASET}"
    timeout ${TIMEOUT} ${BIN} ${DATA_FILE} ${L1} ${L2} ${N_MODELS} ${BOUND} ${SEARCH} ${PARAMS} >> ${FILE_RESULTS}
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

# Write csv header
echo "dataset,n_keys,layer1,layer2,n_models,bounds,search,size_in_bytes,rep,n_samples,lookup_time,lookup_accu" > ${FILE_RESULTS} # Write csv header

# Run model type experiment
for dataset in ${DATASETS};
do
    echo "Performing ${EXPERIMENT} on '${dataset}'..."
    for l1 in ${LAYER1};
    do
        for l2 in ${LAYER2};
        do
            for ((i=6; i<=25; i += 1));
            do
                n_models=$((2**$i))
                run ${dataset} ${l1} ${l2} ${n_models} none model_biased_linear
                run ${dataset} ${l1} ${l2} ${n_models} none model_biased_exponential

                run ${dataset} ${l1} ${l2} ${n_models} gabs binary

                run ${dataset} ${l1} ${l2} ${n_models} gind model_biased_binary
                run ${dataset} ${l1} ${l2} ${n_models} gind binary

                run ${dataset} ${l1} ${l2} ${n_models} labs binary

                run ${dataset} ${l1} ${l2} ${n_models} lind model_biased_binary
                run ${dataset} ${l1} ${l2} ${n_models} lind binary
            done
        done
    done
done
