#!/bin/bash
set -e

# Stop running container if any
docker stop ind_container || echo ""
docker rm ind_container || echo ""


# Build image.
docker build . -f ./indirect-rec/Dockerfile --network=host --tag dir-rec

# Start container.
docker run -it --name ind_container -d -v $(pwd)/../$1/results:/results -v $(pwd)/../$1/work-dir:/work-dir --privileged dir-rec bash

echo "RUNNING ${LINUX_VERSION} ${FLAVOUR}"
# Run eval in container.
sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_MODULES_URL=$LINUX_MODULES_URL -e LINUX_VERSION=$LINUX_VERSION -e LINUX_FLAVOUR=$LINUX_FLAVOUR -it ind_container /scripts/do-indirect-record.sh

