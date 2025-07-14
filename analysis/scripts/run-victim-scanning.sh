#!/bin/bash
export DIR_WORK=/work-dir
set -e

# -----------------------------------------------------------------------------
# 1: Retreive the linux images
# -----------------------------------------------------------------------------

/scripts/shared/get-linux-images.sh

# -----------------------------------------------------------------------------
# 2: Create memory dump of kernel image
# -----------------------------------------------------------------------------

/scripts/shared/create-memory-dump.sh

# -----------------------------------------------------------------------------
# 3: Create entry point list
# -----------------------------------------------------------------------------

mkdir -p $DIR_WORK/entry-points

if [ ! -f $DIR_WORK/entry-points/syscall_x64_entry_targets.csv ]; then
    cd $DIR_WORK/entry-points

    echo "address,name" > syscall_x64_entry_targets.csv
    nm ../images/vmlinux-dbg | grep -E " __x64_sys_*" | awk '{print "0x"$1 "," $3}' >> syscall_x64_entry_targets.csv
else
    echo "[+] [CACHED] Resuing entry-point lists from work directory"
fi

cd $DIR_WORK/entry-points

echo "address,length" > instructions_to_ignore.txt
objdump -D  ../images/dump_vmlinux --start-address=0xffffffff81000000 | grep "stac" | /entry-points/get_address_instruction_length.py >> instructions_to_ignore.txt
objdump -D  ../images/dump_vmlinux --start-address=0xffffffff81000000 | grep "clac" | /entry-points/get_address_instruction_length.py >> instructions_to_ignore.txt


# -----------------------------------------------------------------------------
# 4: Start scanner
# -----------------------------------------------------------------------------

OUT_FOLDER=out_victims

cd /results

rm -f fail.txt unsupported.txt
if [ -d $OUT_FOLDER ]; then
    mv $OUT_FOLDER ${OUT_FOLDER}_`date +"%Y%m%d-%H%M"`
fi
mkdir $OUT_FOLDER

echo "[+] Running scanner on all syscalls (Total: `cat ${DIR_WORK}/entry-points/syscall_x64_entry_targets.csv | wc -l`)"
# Start the analyzer with 20 parallel jobs.
python3 /scanner/run-parallel.py /inspectre-gadget/inspectre $DIR_WORK/images/dump_vmlinux $DIR_WORK/entry-points/syscall_x64_entry_targets.csv -c /scanner/config_victims.yaml -o $OUT_FOLDER -t2700 -j28 -s $DIR_WORK/images/vmlinux-dbg --ignore-instructions-list $DIR_WORK/entry-points/instructions_to_ignore.txt --indirect-branch-targets $DIR_WORK/indirect-recorder/indirect_branch_targets.json
# Move all to output folder
mv fail.txt $OUT_FOLDER
mv unsupported.txt $OUT_FOLDER || true
# Merge all results.
cd $OUT_FOLDER
python3 /analysis/merge_gadgets.py

# -----------------------------------------------------------------------------
# 5: Analyze results
# -----------------------------------------------------------------------------

cd /results/${OUT_FOLDER}
# Run the reasoner
/inspectre-gadget/inspectre reason all-tfps.csv all-tfps-reasoned.csv | tee log_victim_scanning.txt

# Copy used entry points
mkdir -p lists
cp /work-dir/entry-points/syscall_x64_entry_targets.csv lists/

