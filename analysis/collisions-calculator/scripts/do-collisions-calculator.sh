#!/bin/bash
set -e

export DIR_WORK=/work-dir


# -----------------------------------------------------------------------------
# 1: Retrieve the linux images
# -----------------------------------------------------------------------------

/scripts/shared/get-linux-images.sh

# -----------------------------------------------------------------------------
# 2: Create memory dump of kernel image
# -----------------------------------------------------------------------------

/scripts/shared/create-memory-dump.sh

# -----------------------------------------------------------------------------
# 3: Create branches list kernel text
# -----------------------------------------------------------------------------

if [ ! -f $DIR_WORK/images/vmlinux_asm ] || [ ! -f $DIR_WORK/images/branches_kernel_text.csv ]; then
    echo "[+] Exporting branches for Kernel Text"
    cd $DIR_WORK/images

    START_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".text" | awk '{print "0x"$5'}`
    END_ADDRESS=`readelf -S $DIR_WORK/images/vmlinux | grep -w ".rodata" | awk '{print "0x"$5'}`

    objdump -D $DIR_WORK/images/dump_vmlinux --start-address=$START_ADDRESS --stop-address=$END_ADDRESS > vmlinux_asm

    # Extract branches from assembly output
    python3 /entry-points/extract_branches.py vmlinux_asm branches.csv

    # Add is_module column
    head -n 1 branches.csv | awk '{print $0";module_name;is_module"}' > tmp.csv
    tail -n +2 branches.csv | awk '{print $0";;0"}' >> tmp.csv
    mv tmp.csv branches_kernel_text.csv

    # TOOD: All branch entry points
    nm  $DIR_WORK/images/vmlinux-dbg | grep -e " t " -e " T " | grep -v "__pfx_" | awk '{print "0x"$1 "," $3}'  > kernel_text_function_addresses.csv
    python3 /entry-points/get_branch_entry_points.py $DIR_WORK/images/dump_vmlinux kernel_text_function_addresses.csv > kernel_text_branch_entry_points.csv

else
    echo "[+] [CACHED] Resuing kernel text vmlinux assembly from work directory"
fi


# -----------------------------------------------------------------------------
# Create memory dump with modules (multiple snapshots)
# -----------------------------------------------------------------------------
/scripts/shared/create-memory-dump-with-modules.sh


# -----------------------------------------------------------------------------
# Extract branches for all snapshots
# -----------------------------------------------------------------------------
cd $DIR_WORK/images

for SNAPSHOT_FOLDER in ./snapshot_*/     # list directories in the form "/tmp/dirname/"
do
    [ -d "$SNAPSHOT_FOLDER" ] || continue

    if [ ! -f $SNAPSHOT_FOLDER/all_branches.csv ] || [ ! -f $SNAPSHOT_FOLDER/all_branch_entry_points.csv ] ; then
        echo "[+] Exporting branches for $SNAPSHOT_FOLDER"

        cd $SNAPSHOT_FOLDER

        START_ADDRESS=`head -n1  text_region.txt | awk '{print "0x"$1'}`
        END_ADDRESS=`tail -n1  text_region.txt | awk '{print "0x"$1'}`

        rm -rf vmlinux_modules_asm
        rm -rf branches_modules_last.csv
        rm -rf branches_modules.csv
        rm -rf module_branch_entry_points_last.csv
        rm -rf module_branch_entry_points.csv
        rm -rf entry_points.pickle

        cat module_names.txt | while read module_name
        do
            section_start=`grep "module/${module_name}/sections/.text$" module_sections.txt | awk '{print $1}'`
            # First we check on clean_up_module
            section_end=`grep "\[${module_name}\]" kallsyms.txt | grep -w "cleanup_module" | awk '{print "0x"$1}'`

            if [ -z "${section_end}" ]; then
                section_end=`grep "\[${module_name}\]" kallsyms.txt | sort | grep -i " t " | grep -v "T init_module" | tail -n 1 | awk '{print "0x"$1}'`
            fi

            # echo $section_start "-" $section_end " " $module_name
            /scripts/shared/objdump_dump_segment.sh dump_vmlinux_modules $section_start $section_end > vmlinux_module_asm_${module_name}

            cat vmlinux_module_asm_${module_name} >> vmlinux_modules_asm

            python3 /entry-points/extract_branches.py vmlinux_module_asm_${module_name} branches_modules_last.csv

            # Insert the branch module name to the extracted branches
            head -n 1 branches_modules_last.csv | awk '{print $0";module_name"}' > tmp.csv
            tail -n +2 branches_modules_last.csv | awk -v var="$module_name" '{print $0";"var}' >> tmp.csv
            mv tmp.csv branches_modules_last.csv
            # Add to the global list, later we add the header
            tail -n +2 branches_modules_last.csv >> branches_modules.csv

            # Lion cove specific: we want to get the entry point of each branch

            grep "\[${module_name}\]" kallsyms.txt | grep -e " t " -e " T " | grep -v "__pfx_" | awk '{print "0x"$1 "," $3}'  > module_functions.txt
            python3 -u /entry-points/get_branch_entry_points.py dump_vmlinux_modules module_functions.txt --module-section > module_branch_entry_points_last.csv

            # Add to the global list, later we add the header
            tail -n +2 module_branch_entry_points_last.csv >> module_branch_entry_points.csv

            rm vmlinux_module_asm_${module_name}

        done

        # Add is_module column
        # We copy header from last output
        head -n 1 branches_modules_last.csv | awk '{print $0";is_module"}' > tmp.csv
        tail -n +2 branches_modules.csv | awk '{print $0";1"}' >> tmp.csv
        mv tmp.csv branches_modules.csv

        cp $DIR_WORK/images/branches_kernel_text.csv all_branches.csv
        tail -n +2 branches_modules.csv >> all_branches.csv

        head -n 1 module_branch_entry_points_last.csv > tmp.csv
        tail -n +2 module_branch_entry_points.csv >> tmp.csv
        mv tmp.csv module_branch_entry_points.csv

        cp $DIR_WORK/images/kernel_text_branch_entry_points.csv all_branch_entry_points.csv
        tail -n +2 module_branch_entry_points.csv >> all_branch_entry_points.csv



        cd $DIR_WORK/images
    else
        echo "[+] [CACHED] Resuing branch info for $SNAPSHOT_FOLDER"
    fi

done

echo "[+] Do collision analsyis for all snaphosts"

# -----------------------------------------------------------------------------
# INDIRECT: Output for all snapshots IND-IND collisions
# -----------------------------------------------------------------------------

for SNAPSHOT_FOLDER in ./snapshot_*/
do
    [ -d "$SNAPSHOT_FOLDER" ] || continue

    if [ ! -f $SNAPSHOT_FOLDER/collisions.csv ] || [ ! -f $SNAPSHOT_FOLDER/collisions_lion_cove_dir_ind.csv ]; then
        echo "[+] Doing collisions analysis for $SNAPSHOT_FOLDER"
        cd $SNAPSHOT_FOLDER

        START_ADDRESS=`head -n1  text_region.txt | awk '{print "0x"$1'}`

        python3 -u /scripts/output_indirect_collisions.py all_branches.csv collisions.csv --text-base=$START_ADDRESS | tee log_collisions.txt
        cp collisions.csv collisions_common.csv

        python3 -u /scripts/output_indirect_collisions_lion_cove.py all_branches.csv all_branch_entry_points.csv collisions_lion_cove.csv --text-base=$START_ADDRESS | tee -a log_collisions.txt
        tail -n +2 collisions_lion_cove.csv.compatible >> collisions.csv

        python3 -u /scripts/output_direct_collisions_lion_cove.py all_branches.csv all_branch_entry_points.csv  collisions_lion_cove_dir_ind.csv --text-base=$START_ADDRESS | tee log_collisions_lion_cove_dir_ind.txt


        # Add snapshot_id column
        SNAPSHOT_ID=`tr -d "/." <<< $SNAPSHOT_FOLDER`
        head -n 1 collisions.csv | awk '{print $0";snapshot_id"}' > tmp.csv
        tail -n +2 collisions.csv | awk -v var="$SNAPSHOT_ID" '{print $0";"var}' >> tmp.csv
        mv tmp.csv collisions.csv

        # Add snapshot_id column
        SNAPSHOT_ID=`tr -d "/." <<< $SNAPSHOT_FOLDER`
        head -n 1 collisions_common.csv | awk '{print $0";snapshot_id"}' > tmp.csv
        tail -n +2 collisions_common.csv | awk -v var="$SNAPSHOT_ID" '{print $0";"var}' >> tmp.csv
        mv tmp.csv collisions_common.csv


        # Add snapshot_id column
        SNAPSHOT_ID=`tr -d "/." <<< $SNAPSHOT_FOLDER`
        head -n 1 collisions_lion_cove.csv | awk '{print $0";snapshot_id"}' > tmp.csv
        tail -n +2 collisions_lion_cove.csv | awk -v var="$SNAPSHOT_ID" '{print $0";"var}' >> tmp.csv
        mv tmp.csv collisions_lion_cove.csv


        # Add snapshot_id column
        SNAPSHOT_ID=`tr -d "/." <<< $SNAPSHOT_FOLDER`
        head -n 1 collisions_lion_cove_dir_ind.csv | awk '{print $0";snapshot_id"}' > tmp.csv
        tail -n +2 collisions_lion_cove_dir_ind.csv | awk -v var="$SNAPSHOT_ID" '{print $0";"var}' >> tmp.csv
        mv tmp.csv collisions_lion_cove_dir_ind.csv

        cd $DIR_WORK/images
    else
        echo "[+] [CACHED] Reusing collision analysis for $SNAPSHOT_FOLDER"
    fi
done

# -----------------------------------------------------------------------------
# INDIRECT: Merging
# -----------------------------------------------------------------------------
mkdir -p $DIR_WORK/collisions

# MERGING

if [ ! -f $DIR_WORK/collisions/collisionsXXX.csv ] || [ ! -f $DIR_WORK/collisions/collisions.db ] || [ ! -f $DIR_WORK/collisions/collisions_lion_cove_dir_ind.db ]; then

    echo "[+] Merging collisions files"
    cd $DIR_WORK/images

    head -n 1 ./snapshot_000/collisions.csv > $DIR_WORK/collisions/collisions.csv
    head -n 1 ./snapshot_000/collisions_lion_cove.csv > $DIR_WORK/collisions/collisions_lion_cove.csv
    head -n 1 ./snapshot_000/collisions_lion_cove_dir_ind.csv > $DIR_WORK/collisions/collisions_lion_cove_dir_ind.csv

    for SNAPSHOT_FOLDER in ./snapshot_*/     # list directories in the form "/tmp/dirname/"
    do
        [ -d "$SNAPSHOT_FOLDER" ] || continue

        cd $SNAPSHOT_FOLDER

        tail -n +2 collisions.csv >> $DIR_WORK/collisions/collisions.csv
        tail -n +2 collisions_lion_cove.csv >> $DIR_WORK/collisions/collisions_lion_cove.csv
        tail -n +2 collisions_lion_cove_dir_ind.csv >> $DIR_WORK/collisions/collisions_lion_cove_dir_ind.csv

        cd $DIR_WORK/images

    done

    cd $DIR_WORK/collisions

    sqlite3 :memory: -header -csv -separator \; '.import collisions.csv collisions' '.save collisions.db'
    echo "[+] Saved collisions.db"

    sqlite3 :memory: -header -csv -separator \; '.import collisions_lion_cove.csv collisions' '.save collisions_lion_cove.db'
    echo "[+] Saved collisions_lion_cove.db"

    sqlite3 :memory: -header -csv -separator \; '.import collisions_lion_cove_dir_ind.csv collisions' '.save collisions_lion_cove_dir_ind.db'
    echo "[+] Saved collisions_lion_cove_dir_ind.db"

else
    echo "[+] [CACHED] Reusing merged collision file $DIR_WORK/collisions/collisions.csv"
fi

# -----------------------------------------------------------------------------
# ITS: Output for all snapshots DIR-IND collisions
# -----------------------------------------------------------------------------

echo "[+] Do ITS collision analsyis for all snapshots"

cd $DIR_WORK/images

for SNAPSHOT_FOLDER in ./snapshot_*/
do
    [ -d "$SNAPSHOT_FOLDER" ] || continue

    if [ ! -f $SNAPSHOT_FOLDER/collisions_its.csv ]; then
        echo "[+] Doing collisions analysis for $SNAPSHOT_FOLDER"
        cd $SNAPSHOT_FOLDER

        START_ADDRESS=`head -n1  text_region.txt | awk '{print "0x"$1'}`

        python3 -u /entry-points/output_collisions.py $DIR_WORK/images/branches_kernel_text.csv collisions_its.csv --text-base=$START_ADDRESS | tee log_collisions_its.txt

        # Add snapshot_id column
        SNAPSHOT_ID=`tr -d "/." <<< $SNAPSHOT_FOLDER`
        head -n 1 collisions_its.csv | awk '{print $0";snapshot_id"}' > tmp.csv
        tail -n +2 collisions_its.csv | awk -v var="$SNAPSHOT_ID" '{print $0";"var}' >> tmp.csv
        mv tmp.csv collisions_its.csv

        cd $DIR_WORK/images
    else
        echo "[+] [CACHED] Resuing ITS collision analysis for $SNAPSHOT_FOLDER"
    fi
done


# -----------------------------------------------------------------------------
# ITS: Merging
# -----------------------------------------------------------------------------

if [ ! -f $DIR_WORK/collisions/collisions_its.csv ] || [ ! -f $DIR_WORK/collisions/collisions_its.db ]; then

    echo "[+] Merging collisions files"
    cd $DIR_WORK/images


    head -n 1 ./snapshot_000/collisions_its.csv > $DIR_WORK/collisions/collisions_its.csv

    for SNAPSHOT_FOLDER in ./snapshot_*/     # list directories in the form "/tmp/dirname/"
    do
        [ -d "$SNAPSHOT_FOLDER" ] || continue

        cd $SNAPSHOT_FOLDER

        tail -n +2 collisions_its.csv >> $DIR_WORK/collisions/collisions_its.csv

        cd $DIR_WORK/images

    done

    cd $DIR_WORK/collisions

    sqlite3 :memory: -header -csv -separator \; '.import collisions_its.csv collisions' '.save collisions_its.db'

    echo "[+] Saved collisions_its.db"

else
    echo "[+] [CACHED] Reusing merged ITS collision file $DIR_WORK/collisions/collisions_its.csv"
fi

