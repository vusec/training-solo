#/bin/bash
# Figure 8
echo "Generating Figure 8..."

PARENT_FOLDER=$1
RESULTS_FOLDER=linux6.8.0-38-generic/results/out_its_cbpf/

OUT_FOLDER=./figures/

# sql_folder=`dirname -- "$0";`"/queries"
sql_folder=./queries
SQLITE="sqlite3 -header -csv -separator ;"

db=${PARENT_FOLDER}/${RESULTS_FOLDER}/gadgets.db
mkdir -p data ${OUT_FOLDER}

$SQLITE $db ".read $sql_folder/kernel_its_cbpf_cache.sql" > ./data/ind_its_cbpf_cache.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_sdb.sql" > ./data/ind_its_cbpf_sdb.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_dispatch.sql" > ./data/ind_its_cbpf_dispatch.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_all.sql" > ./data/ind_its_cbpf_all.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/ind_its_cbpf_all.csv "All" \
-df data/ind_its_cbpf_cache.csv "Data cache" \
-df data/ind_its_cbpf_sdb.csv "BTB" \
-df data/ind_its_cbpf_dispatch.csv "Dispatch" \
-o ${OUT_FOLDER}/fig8_its_cbpf_cdf_ \
-l 'right'
# -o  figures/ind_module_cdf_ \

$SQLITE $db ".read $sql_folder/kernel_its_cbpf_cache_victim.sql" > ./data/ind_its_cbpf_cache_victim.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_sdb_victim.sql" > ./data/ind_its_cbpf_sdb_victim.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_dispatch_victim.sql" > ./data/ind_its_cbpf_dispatch_victim.csv
$SQLITE $db ".read $sql_folder/kernel_its_cbpf_all_victim.sql" > ./data/ind_its_cbpf_all_victim.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/ind_its_cbpf_all_victim.csv "All" \
-df data/ind_its_cbpf_cache_victim.csv "Data cache" \
-df data/ind_its_cbpf_sdb_victim.csv "BTB" \
-df data/ind_its_cbpf_dispatch_victim.csv "Dispatch" \
-o ${OUT_FOLDER}/fig8_its_cbpf_cdf_victim_ \
-l 'left'

# $SQLITE $db ".read $sql_folder/kernel_its_cbpf_all_msb9.sql" > ./data/ind_its_cbpf_all_msb9.csv


# python3 create_ecdf_collisions.py \
# -df data/ind_its_cbpf_all.csv "Comet Lake" \
# -df data/ind_its_cbpf_all_msb9.csv "Sunny Cove" \
# -o ~/papers/intra-mode-paper/figs/its_cbpf_msb_cdf_
# # -o  figures/ind_module_cdf_ \
