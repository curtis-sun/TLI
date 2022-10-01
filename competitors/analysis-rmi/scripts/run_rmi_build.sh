#!bash
# set -x
trap "exit" SIGINT

EXPERIMENT="rmi build"

DIR_DATA="data"
DIR_RESULTS="results"
FILE_RESULTS="${DIR_RESULTS}/rmi_build.csv"

BIN="build/bin/rmi_build"

# Set number of repetitions and samples
N_REPS="3"
PARAMS="--n_reps ${N_REPS}"
TIMEOUT="60s"

DATASETS="books_200M_uint64 fb_200M_uint64 osm_cellids_200M_uint64 wiki_ts_200M_uint64"
LAYER1="cubic_spline linear_spline linear_regression radix"
LAYER2="linear_spline linear_regression"
BOUNDS="none gabs gind labs lind"

run() {
    DATASET=$1
    L1=$2
    L2=$3
    N_MODELS=$4
    BOUND=$5
    DATA_FILE="${DIR_DATA}/${DATASET}"
    timeout ${TIMEOUT} ${BIN} ${DATA_FILE} ${L1} ${L2} ${N_MODELS} ${BOUND} ${PARAMS} >> ${FILE_RESULTS}
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
echo "dataset,n_keys,rmi,layer1,layer2,n_models,bounds,size_in_bytes,rep,build_time,checksum" > ${FILE_RESULTS} # Write csv header

# Run layer1 and layer 2 model type experiment
for dataset in ${DATASETS};
do
    echo "Performing ${EXPERIMENT} (ours) on '${dataset}'..."
    for ((i=6; i<=25; i += 1));
    do
        n_models=$((2**$i))
        for l1 in ${LAYER1};
        do
            for l2 in ${LAYER2};
            do
                for bound in ${BOUNDS};
                do
                    run ${dataset} ${l1} ${l2} ${n_models} ${bound}
                done
            done
        done
    done
done


# Prepare reference implementation experiment
CWD=$(pwd)
RMI_PATH="third_party/RMI"
TMP_PATH="${CWD}/${RMI_PATH}/tmp"
MANIFEST_FILE="${CWD}/${RMI_PATH}/Cargo.toml"
NAMESPACE="tmp"
RESULTS_FILE=${CWD}/${FILE_RESULTS}
mkdir -p ${TMP_PATH}
cd ${TMP_PATH}

declare -A l1models
l1models['linear_spline']="linear_spline"
l1models['cubic_spline']="cubic"
l1models['linear_regression']="linear"
l1models['radix']="radix"

declare -A l2models
l2models['linear_spline']="linear_spline"
l2models['linear_regression']="linear"

declare -A bounds
bounds['labs']=""
bounds['none']="--no-errors"

# Run reference implementation experiment
for dataset in ${DATASETS};
do
    DATA_FILE="${CWD}/${DIR_DATA}/${dataset}"
    echo "Performing ${EXPERIMENT} (ref) on '${dataset}'..."
    for ((i=6; i<=25; i += 1));
    do
        n_models=$((2**$i))
        for l1 in ${!l1models[@]};
        do
            for l2 in ${!l2models[@]};
            do
                for bound in ${!bounds[@]};
                do
                    for ((rep=0; rep<${N_REPS}; rep += 1));
                    do
                        # Build RMI.
                        cargo run --manifest-path ${MANIFEST_FILE} --release -- ${DATA_FILE} ${NAMESPACE} ${l1models[${l1}]},${l2models[${l2}]} ${n_models} ${bounds[${bound}]} > /dev/null

                        # Exract results.
                        size=$(cat ${TMP_PATH}/tmp.h | grep SIZE | sed 's/.*=//' | tr -d -c 0-9)
                        build_time=$(cat ${TMP_PATH}/tmp.h | grep BUILD | sed 's/.*=//' | tr -d -c 0-9)

                        # Append results to csv.
                        echo "${dataset},200000000,ref,${l1},${l2},${n_models},${bound},${size},${rep},${build_time},0" >> ${RESULTS_FILE}
                    done
                done
            done
        done
    done
done
cd $CWD
