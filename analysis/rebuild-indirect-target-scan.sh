#!/bin/bash
set -e

# Stop running container if any
docker stop its_container || echo ""
docker rm its_container || echo ""


# Build image.
docker build . --network=host --tag btc-eval

# Start container.
docker run -it --name its_container -d -v $(pwd)/../$1/results:/results -v $(pwd)/../$1/work-dir:/work-dir --privileged btc-eval bash

echo "RUNNING ${LINUX_VERSION} ${FLAVOUR}"
# Run eval in container.
sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/run-gadget-scanning-ind-targets.sh
