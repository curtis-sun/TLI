#!bash
# set -x
trap "exit" SIGINT

DIR_DATA="data"

# Set download urls
declare -A urls
urls["books_200M_uint64"]="https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/A6HDNT"
urls["fb_200M_uint64"]="https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/EATHF7"
urls["osm_cellids_200M_uint64"]="https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/8FX9BV"
urls["wiki_ts_200M_uint64"]="https://dataverse.harvard.edu/api/access/datafile/:persistentId?persistentId=doi:10.7910/DVN/JGVF9A/SVN8PI"

# Set md5 for compressed files
declare -A md5zst
md5zst["books_200M_uint64"]="cd1f8bcb0dfd36f9ab08d160b887bf8a"
md5zst["fb_200M_uint64"]="fec241e8b021b198b0849fbd5564c05f"
md5zst["osm_cellids_200M_uint64"]="42575cb58f24bb7ea0a623d422d4c9a6"
md5zst["wiki_ts_200M_uint64"]="6a2b17020959084ce2640177ee4afd5e"

# Set md5 for decompressed files
declare -A md5bin
md5bin["books_200M_uint64"]="aeedc7be338399ced89d0bb82287e024"
md5bin["fb_200M_uint64"]="3b0f820caa0d62150e87ce94ec989978"
md5bin["osm_cellids_200M_uint64"]="a7f6b8d2df09fcda5d9cfbc87d765979"
md5bin["wiki_ts_200M_uint64"]="4f1402b1c476d67f77d2da4955432f7d"

check_md5() {
    FILE=$1
    MD5_EXPECTED=$2
    echo "Checking '${FILE}'..."
    MD5_ACTUAL=$(md5sum -b ${FILE} | cut -d ' ' -f 1)
    [ ${MD5_EXPECTED} == ${MD5_ACTUAL} ]
}

download() {
    DATASET=$1
    FILE="${DIR_DATA}/${DATASET}.zst"
    URL=${urls[${DATASET}]}
    echo "Downloading '${DATASET}'..."
    wget -q --show-progress -O ${FILE} ${URL}
    return $?
}

decompress() {
    FILE=$1
    echo "Decompressing '${FILE}'..."
    zstd -f -d ${FILE}
    return $?
}

# Create data directory
if [ ! -d "${DIR_DATA}" ];
then
    mkdir -p "${DIR_DATA}";
fi

# Download datasets
for dataset in ${!urls[@]};
do
    FILE_BIN=${DIR_DATA}/${dataset}
    if [ -f ${FILE_BIN} ];
    then
        echo "File '${FILE_BIN}' already exists."
        check_md5 ${FILE_BIN} ${md5bin[${dataset}]} && continue
    fi

    FILE_ZST=${DIR_DATA}/${dataset}.zst
    if [ -f ${FILE_ZST} ];
    then
        echo "File '${FILE_ZST}' already exists."
        check_md5 ${FILE_ZST} ${md5zst[${dataset}]} && decompress ${FILE_ZST} && check_md5 ${FILE_BIN} ${md5bin[${dataset}]} && continue
    fi

    download ${dataset} && check_md5 ${FILE_ZST} ${md5zst[${dataset}]} && decompress ${FILE_ZST} && check_md5 ${FILE_BIN} ${md5bin[${dataset}]} && continue
    echo "Download failed. Please try again."
done
