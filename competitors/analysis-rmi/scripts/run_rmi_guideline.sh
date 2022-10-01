#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="rmi guideline"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/rmi_guideline.csv"

BIN="build/bin/rmi_guideline"

# Set number of repetitions and samples
N_REPS="3"
N_SAMPLES="20000000"
PARAMS="--n_reps ${N_REPS} --n_samples ${N_SAMPLES}"

run() {
    DATASET=$1
    BUDGET=$2
    DATA_FILE="${DIR_DATA}/${DATASET}"
    ${BIN} ${DATA_FILE} ${BUDGET} ${PARAMS} >> ${FILE_RESULTS}
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

DATASETS="books_200M_uint64 osm_cellids_200M_uint64 wiki_ts_200M_uint64"

# Run experiments
echo "dataset,n_keys,layer1,layer2,n_models,bounds,search,size_in_bytes,rep,n_samples,budget_in_bytes,is_guideline,lookup_time,lookup_accu" > ${FILE_RESULTS} # Write csv header
for dataset in ${DATASETS};
do
    echo "Performing ${EXPERIMENT} on '${dataset}'..."
    for ((i=1; i<=20; i += 1));
    do
        budget=$((2**$i * 1024))
        run ${dataset} ${budget}
    done
done
