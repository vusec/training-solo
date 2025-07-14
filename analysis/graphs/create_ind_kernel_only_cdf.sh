#/bin/bash

# sql_folder=`dirname -- "$0";`"/queries"
sql_folder=./queries
SQLITE="sqlite3 -header -csv -separator ;"

db=./databases/gadgets_ind_col_kernel_only.db

# Modified max instructions to 90

# $SQLITE $db ".read $sql_folder/kernel_ind_only_cache_gadgets.sql" > ./data/ind_kernel_only_gadgets.csv
# $SQLITE $db ".read $sql_folder/kernel_ind_only_cache_sdb.sql" > ./data/ind_kernel_only_sdb.csv
# $SQLITE $db ".read $sql_folder/kernel_ind_only_cache_dispatch.sql" > ./data/ind_kernel_only_dispatch.csv
# $SQLITE $db ".read $sql_folder/kernel_ind_only_cache_all.sql" > ./data/ind_kernel_only_all.csv


python3 create_ecdf_collisions.py \
-df data/ind_kernel_only_all.csv "All" \
-df data/ind_kernel_only_gadgets.csv "Data cache" \
-df data/ind_kernel_only_sdb.csv "BTB" \
-df data/ind_kernel_only_dispatch.csv "Dispatch" \
-o  figures/ind_kernel_only_cdf_ \
# -o ~/papers/intra-mode-paper/figs/ind_module_cdf_
# -o  figures/ind_module_cdf_ \

