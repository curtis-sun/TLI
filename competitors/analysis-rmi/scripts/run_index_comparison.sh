#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="index comparison"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/index_comparison.csv"

BIN="build/bin/index_comparison"

# Set number of repetitions and samples
N_REPS="3"
N_SAMPLES="20000000"
PARAMS="--n_reps ${N_REPS} --n_samples ${N_SAMPLES}"

# Set which indexes to run on datasets
declare -A flags
flags['books_200M_uint64']="--rmi --alex --pgm --rs --cht --art --tlx --ref --bin"
flags['fb_200M_uint64']="--rmi --alex --pgm --rs --cht --art --tlx --ref --bin"
flags['osm_cellids_200M_uint64']="--rmi --alex --pgm --rs --cht --art --tlx --ref --bin"
flags['wiki_ts_200M_uint64']="--rmi --alex --pgm --rs --tlx --ref --bin" # ART and CHT do not support duplicates

run() {
    DATASET=$1
    DATA_FILE="${DIR_DATA}/${DATASET}"
    ${BIN} ${PARAMS} ${flags[${DATASET}]} ${DATA_FILE} >> ${FILE_RESULTS}
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

# Run experiments
echo "dataset,n_keys,index,config,size_in_bytes,rep,n_samples,build_time,eval_time,lookup_time,eval_accu,lookup_accu" > ${FILE_RESULTS} # Write csv header
for dataset in ${!flags[@]};
do
    echo "Performing ${EXPERIMENT} on '${dataset}'..."
    run $dataset
done
