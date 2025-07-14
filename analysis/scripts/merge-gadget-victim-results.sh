#!/bin/bash
set -e

OUT_FOLDER=out_its_all_snapshots
cd /results/$OUT_FOLDER

echo "[-] Combining gadgets with victims"
python3 -u /analysis/append_ind_control_info.py ../out_victims/all-tfps-reasoned.csv gadget_collisions.csv gadget-ind-victim.csv | tee log_merging.txt
python3 -u /analysis/append_ind_control_info.py ../out_victims/all-tfps-reasoned.csv tfp_collisions.csv tfp-ind-victim.csv | tee -a log_merging.txt


# Create sqlite3 database
echo "[-] Creating database"
/analysis/build-db.sh

# Execute some queries for manual analysis later
/analysis/get_summary.sh | tee -a log_merging.txt

cd /results

tar -czvf results_${OUT_FOLDER}_${LINUX_VERSION}_`date +"%Y_%m_%d"`.tar.gz ${OUT_FOLDER}/gadgets.db ${OUT_FOLDER}/asm ${OUT_FOLDER}/lists out_victims/asm/ out_victims/lists out_victims/all-tfps-reasoned.csv  out_victims/log_victim_scanning.txt > /dev/null

# tar -czvf results_${OUT_FOLDER}_${LINUX_VERSION}_`date +"%Y_%m_%d"`.tar.gz ${OUT_FOLDER}/gadgets.db ${OUT_FOLDER}/gadgets_victim.db  ${OUT_FOLDER}/asm out_victims/asm/ > /dev/null

