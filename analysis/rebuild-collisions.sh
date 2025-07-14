#!/bin/bash
set -e

# Stop running container if any
docker stop col_container || echo ""
docker rm col_container || echo ""


# Build image.
docker build . -f ./collisions-calculator/Dockerfile --network=host --tag col-cal

# Start container.
docker run -it --name col_container -d -v $(pwd)/../$1/results:/results -v $(pwd)/../$1/work-dir:/work-dir --privileged col-cal bash

echo "RUNNING ${LINUX_VERSION} ${FLAVOUR}"
# Run eval in container.
sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -e LINUX_FLAVOUR=$LINUX_FLAVOUR  -it col_container /scripts/do-collisions-calculator.sh
