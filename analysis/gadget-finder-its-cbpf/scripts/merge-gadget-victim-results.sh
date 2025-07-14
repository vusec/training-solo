#!/bin/bash
set -e

OUT_FOLDER=out_its_cbpf

cd /results/$OUT_FOLDER

echo "[-] Combining gadgets with victims"
python3 /scripts/append_ind_control_info.py ../out_victims/all-tfps-reasoned.csv gadget_its_cbpf.csv gadget_its_cbpf_victim.csv | tee log_merging.txt
python3 /scripts/append_ind_control_info.py ../out_victims/all-tfps-reasoned.csv tfp_its_cbpf.csv tfp_its_cbpf_victim.csv | tee -a log_merging.txt


# Create sqlite3 database
echo "[-] Creating database"
/scripts/build-db.sh

# Execute some queries for manual analysis later
/analysis/get_summary.sh | tee -a log_merging.txt

cd /results

tar -czvf results_its_cbpf_${LINUX_VERSION}_${LINUX_FLAVOUR}_`date +"%Y_%m_%d"`.tar.gz $OUT_FOLDER/gadgets.db $OUT_FOLDER/asm $OUT_FOLDER/lists out_victims/asm/ out_victims/lists out_victims/all-tfps-reasoned.csv  out_victims/log_victim_scanning.txt > /dev/null
