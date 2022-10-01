#!bash
# set -x
trap "exit" SIGINT

DIR_DATA="data"
RMI_PATH="third_party/RMI"
CONFIG_PATH="scripts/rmi_ref"

DATASETS="books_200M_uint64 fb_200M_uint64 osm_cellids_200M_uint64 wiki_ts_200M_uint64"

gen_config_json() {
    DATASET=$1
    CWD=$(pwd)
    DATA_FILE="${CWD}/${DIR_DATA}/${DATASET}"
    CONFIG_FILE="${CWD}/${CONFIG_PATH}/${DATASET}.json"
    MANIFEST_FILE="${CWD}/${RMI_PATH}/Cargo.toml"

    echo "Generating reference RMI config json for ${DATASET}..."
    cargo run --manifest-path "${MANIFEST_FILE}" --release -- ${DATA_FILE} --optimize "${CONFIG_FILE}"
}

train_rmi () {
    DATASET=$1
    CWD=$(pwd)
    DATA_FILE="${CWD}/${DIR_DATA}/${DATASET}"
    CONFIG_FILE="${CWD}/${CONFIG_PATH}/${DATASET}.json"
    MANIFEST_FILE="${CWD}/${RMI_PATH}/Cargo.toml"

    # Create include dir
    INCLUDE_PATH="${CWD}/${RMI_PATH}/include/rmi_ref"
    mkdir -p "${INCLUDE_PATH}"
    cd "${INCLUDE_PATH}"

    echo "Training reference RMIs on ${DATASET}..."
    cargo run --manifest-path "${MANIFEST_FILE}" --release -- ${DATA_FILE} --param-grid "${CONFIG_FILE}" --disable-parallel-training

    cd ${CWD}
}

for dataset in ${DATASETS};
do
    # Generate RMI configurations
    # gen_config_json "$dataset" # configs are pre-generated

    # Train RMIs
    train_rmi "$dataset"
done
