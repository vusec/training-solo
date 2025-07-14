#/bin/bash
# Figure 10
echo "Generating Figure 10..."

PARENT_FOLDER=$1
RESULTS_FOLDER=linux6.8.0-38-generic/results/out_its_all_snapshots/

OUT_FOLDER=./figures/

# sql_folder=`dirname -- "$0";`"/queries"
sql_folder=./queries
SQLITE="sqlite3 -header -csv -separator ;"

db=${PARENT_FOLDER}/${RESULTS_FOLDER}/gadgets.db
mkdir -p data ${OUT_FOLDER}

#  Exploitable gadgets for Comet Lake
$SQLITE $db ".read $sql_folder/native_its_all_comet.sql" > ./data/native_its_all_comet.csv
$SQLITE $db ".read $sql_folder/native_its_cache_comet.sql" > ./data/native_its_cache_comet.csv
$SQLITE $db ".read $sql_folder/native_its_sdb_comet.sql" > ./data/native_its_sdb_comet.csv
$SQLITE $db ".read $sql_folder/native_its_dispatch_comet.sql" > ./data/native_its_dispatch_comet.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/native_its_all_comet.csv "All" \
-df data/native_its_cache_comet.csv "Data cache" \
-df data/native_its_sdb_comet.csv "BTB" \
-df data/native_its_dispatch_comet.csv "Dispatch" \
-o ${OUT_FOLDER}/fig10_its_native_comet_lake_cdf_ \
-l none

#  Exploitable gadgets for Sunny Cove
$SQLITE $db ".read $sql_folder/native_its_all_sunny.sql" > ./data/native_its_all_sunny.csv
$SQLITE $db ".read $sql_folder/native_its_cache_sunny.sql" > ./data/native_its_cache_sunny.csv
$SQLITE $db ".read $sql_folder/native_its_sdb_sunny.sql" > ./data/native_its_sdb_sunny.csv
$SQLITE $db ".read $sql_folder/native_its_dispatch_sunny.sql" > ./data/native_its_dispatch_sunny.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/native_its_all_sunny.csv "All" \
-df data/native_its_cache_sunny.csv "Data cache" \
-df data/native_its_sdb_sunny.csv "BTB" \
-df data/native_its_dispatch_sunny.csv "Dispatch" \
-o ${OUT_FOLDER}/fig10_its_native_sunny_cove_cdf_ \
-l right

# #  Exploitable gadgets with a matching victim
$SQLITE $db ".read $sql_folder/native_its_all_w_victim.sql" > ./data/native_its_all_w_victim.csv
$SQLITE $db ".read $sql_folder/native_its_cache_w_victim.sql" > ./data/native_its_cache_w_victim.csv
$SQLITE $db ".read $sql_folder/native_its_sdb_w_victim.sql" > ./data/native_its_sdb_w_victim.csv
$SQLITE $db ".read $sql_folder/native_its_dispatch_w_victim.sql" > ./data/native_its_dispatch_w_victim.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/native_its_all_w_victim.csv "All" \
-df data/native_its_cache_w_victim.csv "Data cache" \
-df data/native_its_sdb_w_victim.csv "BTB" \
-df data/native_its_dispatch_w_victim.csv "Dispatch" \
-o ${OUT_FOLDER}/fig10_its_native_w_victim_cdf_ \
-l left

# #  Exploitable gadgets with a matching victim and reachable direct branch
$SQLITE $db ".read $sql_folder/native_its_all_w_victim_w_dir.sql" > ./data/native_its_all_w_victim_w_dir.csv
$SQLITE $db ".read $sql_folder/native_its_cache_w_victim_w_dir.sql" > ./data/native_its_cache_w_victim_w_dir.csv
$SQLITE $db ".read $sql_folder/native_its_sdb_w_victim_w_dir.sql" > ./data/native_its_sdb_w_victim_w_dir.csv
$SQLITE $db ".read $sql_folder/native_its_dispatch_w_victim_w_dir.sql" > ./data/native_its_dispatch_w_victim_w_dir.csv


python3 create_ecdf_collisions_its_cbpf.py \
-df data/native_its_all_w_victim_w_dir.csv "All" \
-df data/native_its_cache_w_victim_w_dir.csv "Data cache" \
-df data/native_its_sdb_w_victim_w_dir.csv "BTB" \
-df data/native_its_dispatch_w_victim_w_dir.csv "Dispatch" \
-o ${OUT_FOLDER}/fig10_its_native_w_victim_w_dir_cdf_ \
-l none
