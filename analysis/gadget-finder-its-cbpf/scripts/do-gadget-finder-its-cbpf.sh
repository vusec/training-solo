#!/bin/bash
set -e

export DIR_WORK=/work-dir

# -----------------------------------------------------------------------------
# 1: Retreive the linux images
# -----------------------------------------------------------------------------

/scripts/shared/get-linux-images.sh

# -----------------------------------------------------------------------------
# 2: Create memory dump of kernel image
# -----------------------------------------------------------------------------

/scripts/shared/create-memory-dump-with-modules.sh


# -----------------------------------------------------------------------------
# 3: Create kprobe_events
# -----------------------------------------------------------------------------

if [ ! -f $DIR_WORK/images/vmlinux_asm ]; then
    echo "[+] Dumping assembly from memory dump"

    cd $DIR_WORK/images


    START_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".text" | awk '{print "0x"$5'}`
    END_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".rodata" | awk '{print "0x"$5'}`

    objdump -D $DIR_WORK/images/dump_vmlinux --start-address=$START_ADDRESS --stop-address=$END_ADDRESS > vmlinux_asm

else
    echo "[+] [CACHED] Resuing assembly dumps from work directory"
fi

mkdir -p $DIR_WORK/entry-points

if [ ! -f $DIR_WORK/entry-points/branches.csv ] || [ ! -f $DIR_WORK/entry-points/its_cbpf_targets_unique.csv ]; then
    cd $DIR_WORK/entry-points

    echo "[+] Creating entrypoints"

    # Extract branches from assembly output
    python3 /entry-points/extract_branches.py $DIR_WORK/images/vmlinux_asm branches.csv

    # cat $DIR_WORK/indirect-recorder/kprobe_hist | awk '{print "0x"$3}' | tr -d , | sort -u > reached_branch_addresses.csv
    # cat /results/out_victims/all-tfps-reasoned.csv  | awk -F";" '{print $5}' | sort -u >> reached_branch_addresses.csv
    sqlite3 :memory: -header -csv -separator \; -cmd '.import /results/out_victims/all-tfps-reasoned.csv tfps' -cmd '.separator ,' 'select DISTINCT pc as address from tfps where n_controlled_sufficiently > 0;' > reached_branch_addresses.csv

    head -n 1 branches.csv > reached_branches.csv
    cat branches.csv | grep "indirect" | grep -f reached_branch_addresses.csv >> reached_branches.csv

    python3 /scripts/output_indirect_near_targets.py reached_branches.csv its_cbpf_targets.csv

    # Export the unique targets to separate file
    sqlite3 :memory: -header -csv -separator \; -cmd '.import its_cbpf_targets.csv its_cbpf_targets' -cmd '.separator ,' 'select DISTINCT target as address, target as name from its_cbpf_targets;' > its_cbpf_targets_unique.csv
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

OUT_FOLDER=out_its_cbpf
TARGET_FILE=${DIR_WORK}/entry-points/its_cbpf_targets_unique.csv

cd /results

rm -f fail.txt unsupported.txt
if [ -d $OUT_FOLDER ]; then
    mv $OUT_FOLDER $OUT_FOLDER`date +"%Y%m%d-%H%M"`
fi
mkdir $OUT_FOLDER

echo "[+] Running scanner on all near ITS cBPF targets (Total: `cat ${TARGET_FILE}| wc -l`)"

# Start the analyzer with 20 parallel jobs.
python3 /scanner/run-parallel.py /inspectre-gadget/inspectre $DIR_WORK/images/dump_vmlinux ${TARGET_FILE} -c /scanner/config_its_cbpf.yaml -o $OUT_FOLDER -t60 -j32 -s $DIR_WORK/images/vmlinux-dbg
# Move all to output folder
mv fail.txt $OUT_FOLDER
mv unsupported.txt $OUT_FOLDER || true
# Merge all results.
cd $OUT_FOLDER
python3 /analysis/merge_gadgets.py

# -----------------------------------------------------------------------------
# 6: Analyze results
# -----------------------------------------------------------------------------

cd /results/$OUT_FOLDER
# Run the reasoner
/inspectre-gadget/inspectre reason all-gadgets.csv all-gadgets-reasoned.csv | tee log_gadget_scanning.txt
/inspectre-gadget/inspectre reason all-tfps.csv all-tfps-reasoned.csv | tee -a log_gadget_scanning.txt


# Copy used entry points
mkdir -p lists

cp $TARGET_FILE lists/its_cbpf_targets_unique.csv
cp /work-dir/entry-points/branches.csv lists/
cp /work-dir/entry-points/reached_branches.csv lists/
cp /work-dir/entry-points/its_cbpf_targets.csv lists/
cp /work-dir/entry-points/reachable_functions.csv lists/

# Create sqlite3 database
/analysis/join-gadget-its-cbpf.sh

