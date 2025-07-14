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

# if [ ! -f $DIR_WORK/entry-points/vmlinux_asm ] || [ ! -f $DIR_WORK/entry-points/collision_targets.csv ] || [ ! -f $DIR_WORK/entry-points/collisions.csv ]; then
#     cd $DIR_WORK/entry-points

#     START_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".text" | awk '{print "0x"$5'}`
#     END_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".rodata" | awk '{print "0x"$5'}`

#     objdump -D $DIR_WORK/images/dump_vmlinux --start-address=$START_ADDRESS --stop-address=$END_ADDRESS > vmlinux_asm

#     # Extract branches from assembly output
#     python3 /entry-points/extract_branches.py vmlinux_asm branches.csv

#     # Add is_module column
#     head -n 1 branches.csv | awk '{print $0";is_module"}' > tmp.csv
#     tail -n +2 branches.csv | awk '{print $0";0"}' >> tmp.csv
#     mv tmp.csv branches_kernel_text.csv

#     # Find the collisions
#     python3 /entry-points/output_collisions.py branches_kernel_text.csv collisions.csv
#     # Export the unique targets to separate file
#     sqlite3 :memory: -header -csv -separator \; -cmd '.import collisions.csv collisions' -cmd '.separator ,' 'select DISTINCT target as address, target as name from collisions;' > collision_targets.csv

# else
#     echo "[+] [CACHED] Resuing entry-point lists from work directory"
# fi

if [ ! -f $DIR_WORK/entry-points/module_ind_targets.csv ]; then
    cd $DIR_WORK/entry-points

    cat module_ind_targets_info.csv | awk -F',' {'print $1","$2'} > module_ind_targets.csv

else
    echo "[+] [CACHED] Resuing entry-point lists from work directory"
fi

# -----------------------------------------------------------------------------
# 4: Create reachable list
# -----------------------------------------------------------------------------

if [ ! -f $DIR_WORK/entry-points/reachable_functions.csv ]; then
    cd $DIR_WORK/entry-points

    echo "name" > reachable_functions.csv

    # Linux 6.6.0-rc2 (ce9ecca0238b)
    # https://syzkaller.appspot.com/text?tag=KernelConfig&x=710dc49bece494df
    wget https://storage.googleapis.com/syzbot-assets/458b85159265/ci-qemu-upstream-ce9ecca0.html -O ci-qemu-upstream-6.6-rc2.html
    /scripts/shared/get-reached-functions.sh ci-qemu-upstream-6.6-rc2.html > reachable_functions_6.6-rc2.txt


    # Linux 6.10-rc1 (91613e604df0c)
    # https://syzkaller.appspot.com/text?tag=KernelConfig&x=238430243a58f702
    wget https://storage.googleapis.com/syzbot-assets/a088abc3741b/ci-qemu-upstream-1613e604.html -O ci-qemu-upstream-6.10-rc1.html
    /scripts/shared/get-reached-functions.sh ci-qemu-upstream-6.10-rc1.html > reachable_functions_6.10-rc1.txt

    # Latest finished?
    wget https://storage.googleapis.com/syzkaller/cover/ci-qemu-upstream.html -O ci-qemu-upstream.html
    /scripts/shared/get-reached-functions.sh ci-qemu-upstream.html > reachable_functions_latest.txt

    cat reachable_functions_6.6-rc2.txt reachable_functions_6.10-rc1.txt reachable_functions_latest.txt | sort -u >> reachable_functions.csv

else
    echo "[+] [CACHED] Resuing reachable lists from work directory"
fi


# -----------------------------------------------------------------------------
# 5: Start scanner
# -----------------------------------------------------------------------------

OUT_FOLDER=out_module_ind_targets
TARGET_FILE=${DIR_WORK}/entry-points/module_ind_targets.csv

cd /results

rm -f fail.txt unsupported.txt
if [ -d $OUT_FOLDER ]; then
    mv $OUT_FOLDER ${OUT_FOLDER}_`date +"%Y%m%d-%H%M"`
fi
mkdir $OUT_FOLDER

echo "[+] Running scanner on targets (Total: `cat ${TARGET_FILE}| wc -l`)"
# Start the analyzer with 20 parallel jobs.
python3 /scanner/run-parallel.py /inspectre-gadget/inspectre $DIR_WORK/images/dump_vmlinux $TARGET_FILE -c /scanner/config_all.yaml -o ${OUT_FOLDER} -t360 -j24 -s $DIR_WORK/images/vmlinux-dbg
# Move all to output folder
mv fail.txt ${OUT_FOLDER}
mv unsupported.txt ${OUT_FOLDER} || true
# Merge all results.
cd ${OUT_FOLDER}
python3 /analysis/merge_gadgets.py

# -----------------------------------------------------------------------------
# 6: Analyze results
# -----------------------------------------------------------------------------

cd /results/${OUT_FOLDER}
# Run the reasoner
/inspectre-gadget/inspectre reason all-gadgets.csv all-gadgets-reasoned.csv | tee log_gadget_scanning.txt
/inspectre-gadget/inspectre reason all-tfps.csv all-tfps-reasoned.csv | tee -a log_gadget_scanning.txt


# Copy used entry points
mkdir -p lists
cp /work-dir/images/branches_kernel_text.csv lists/
cp $TARGET_FILE lists/
cp /work-dir/entry-points/reachable_functions.csv lists/

# Create sqlite3 database
# /analysis/join-gadget-collisions.sh

