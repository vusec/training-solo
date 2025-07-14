#!/bin/bash
set -e


echo "RUNNING ${LINUX_VERSION} ${FLAVOUR}"

# Run victim scanning (results/out_victims)
# Figure 5. Used for gadget analysis later
sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/run-victim-scanning.sh

# Analyzes all gadget collisions within kernel text (results/out)
# Not used in the end. Can be used to create collisions analysis for one snapshot, and run gadget analysis for it
# sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/run-gadget-scanning.sh

# Analyzes all collisions targets of ITS (dir-ind) (results/out_its_all_snapshots)
# - Results db (manually renamed): gadgets_its_native.db
# - Paper figure 10
sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/run-gadget-scanning-its-all-snapshots.sh

# Merge gadget and victim results into one database
sudo docker exec -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/merge-gadget-victim-results.sh
