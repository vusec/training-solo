#/bin/bash
# Figure 8
echo "Generating Figure 5..."


PARENT_FOLDER=$1
RESULTS_FOLDER=linux6.8.0-38-generic/results/out_its_all_snapshots/

OUT_FOLDER=./figures/

# sql_folder=`dirname -- "$0";`"/queries"
sql_folder=./queries
SQLITE="sqlite3 -header -csv -separator ;"

db=${PARENT_FOLDER}/${RESULTS_FOLDER}/gadgets.db
mkdir -p data ${OUT_FOLDER}

$SQLITE $db ".read $sql_folder/ind_registers_and_control_flow.sql" > ./data/ind_registers_and_control_flow.csv

python3 create_ecdf_regs_length.py  -df data/ind_registers_and_control_flow.csv -o ${OUT_FOLDER}/fig5_ecdf_victim_branches.pdf

$SQLITE $db ".read $sql_folder/ind_registers_and_control_flow_grouped_by_ncontrolled.sql" > ./data/ind_registers_and_control_flow_grouped_by_ncontrolled.csv

python3 scatter_plot.py  -df data/ind_registers_and_control_flow_grouped_by_ncontrolled.csv -o ${OUT_FOLDER}/fig5_scatter_victim_branches.pdf
