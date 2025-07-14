#!/bin/bash
export DIR_WORK=/work-dir
set -e


cd /results/out_victims

sqlite3 :memory: -cmd '.mode csv' -cmd '.separator ;' -cmd ".import lists/branches.csv branches"  -cmd ".import all-tfps-reasoned.csv victims" \
 "select pc from branches where target_type = 'indirect' and address in (select DISTINCT PC from victims where n_controlled_sufficiently >= 1)" > $DIR_WORK/entry-points/cbpf_controlled_victims_pc.csv


