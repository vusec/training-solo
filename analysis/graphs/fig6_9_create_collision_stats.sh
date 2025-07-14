#/bin/bash
# Figure 6 + 9
PARENT_FOLDER=$1
RESULTS_FOLDER=linux6.8.0-38-generic/work-dir/collisions/

OUT_FOLDER=./figures/

mkdir -p ${OUT_FOLDER}

echo "Generating Figure 6..."
python3 summarize_collision_stats.py ${PARENT_FOLDER}/${RESULTS_FOLDER}/collisions.csv ${OUT_FOLDER}

echo "Generating Figure 9..."
python3 summarize_collision_stats.py ${PARENT_FOLDER}/${RESULTS_FOLDER}/collisions_its.csv ${OUT_FOLDER}
