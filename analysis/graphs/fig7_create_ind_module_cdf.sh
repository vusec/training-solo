#/bin/bash
# Figure 7
echo "Generating Figure 7..."

PARENT_FOLDER=$1
RESULTS_FOLDER=linux6.8.0-38-generic-fineibt/results/out_module_ind_targets/

OUT_FOLDER=./figures/

# sql_folder=`dirname -- "$0";`"/queries"
sql_folder=./queries
SQLITE="sqlite3 -header -csv -separator ;"

db=${PARENT_FOLDER}/${RESULTS_FOLDER}/gadgets.db
mkdir -p data ${OUT_FOLDER}

$SQLITE $db ".read $sql_folder/kernel_ind_only_cache_gadgets.sql" > ./data/ind_module_cache_gadgets.csv
$SQLITE $db ".read $sql_folder/kernel_ind_only_cache_sdb.sql" > ./data/ind_module_cache_sdb.csv
$SQLITE $db ".read $sql_folder/kernel_ind_only_cache_dispatch.sql" > ./data/ind_module_cache_dispatch.csv
$SQLITE $db ".read $sql_folder/kernel_ind_only_cache_all.sql" > ./data/ind_module_cache_all.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/ind_module_cache_all.csv "All" \
-df data/ind_module_cache_gadgets.csv "Data cache" \
-df data/ind_module_cache_sdb.csv "BTB" \
-df data/ind_module_cache_dispatch.csv "Dispatch" \
-o ${OUT_FOLDER}/fig7_ind_module_cdf_ \
-l right


db=${PARENT_FOLDER}/${RESULTS_FOLDER}/gadgets_w_victim_no_page_mask.db

$SQLITE $db ".read $sql_folder/kernel_its_module_w_victim_cache.sql" > ./data/kernel_its_module_w_victim_cache.csv
$SQLITE $db ".read $sql_folder/kernel_its_module_w_victim_sdb.sql" > ./data/kernel_its_module_w_victim_sdb.csv
$SQLITE $db ".read $sql_folder/kernel_its_module_w_victim_dispatch.sql" > ./data/kernel_its_module_w_victim_dispatch.csv
$SQLITE $db ".read $sql_folder/kernel_its_module_w_victim_all.sql" > ./data/kernel_its_module_w_victim_all.csv

python3 create_ecdf_collisions_its_cbpf.py \
-df data/kernel_its_module_w_victim_all.csv "All" \
-df data/kernel_its_module_w_victim_cache.csv "Data cache" \
-df data/kernel_its_module_w_victim_sdb.csv "BTB" \
-df data/kernel_its_module_w_victim_dispatch.csv "Dispatch" \
-o ${OUT_FOLDER}/fig7_ind_module_w_victim_cdf_ \
-l left

