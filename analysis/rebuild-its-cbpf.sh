#!/bin/bash
set -e

# Stop running container if any
docker stop its_cbpf_container || echo ""
docker rm its_cbpf_container || echo ""


# Build image.
docker build . -f ./gadget-finder-its-cbpf/Dockerfile --network=host --tag its-cbpf

# Start container.
docker run -it --name its_cbpf_container -d -v $(pwd)/../$1/results:/results -v $(pwd)/../$1/work-dir:/work-dir --privileged its-cbpf bash

echo "RUNNING ${LINUX_VERSION} ${FLAVOUR}"
# Run eval in container.
sudo docker exec -e LINUX_VERSION=$LINUX_VERSION -e LINUX_FLAVOUR=$LINUX_FLAVOUR -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -it its_cbpf_container /scripts/do-gadget-finder-its-cbpf.sh

if [ ! -f $(pwd)/../$1/results/out_victims/all-tfps-reasoned.csv  ]; then
    echo "Run victim scanning"
    sudo docker exec -e LINUX_DEB_URL=$LINUX_DEB_URL -e LINUX_DDEB_URL=$LINUX_DDEB_URL -e LINUX_VERSION=$LINUX_VERSION -it its_container /scripts/run-victim-scanning.sh
else
	echo "./linux${LINUX_VERSION}-${FLAVOUR}/results/out_victims/all-tfps-reasoned.csv file is found, caching victim scanning!"
fi


sudo docker exec -e LINUX_VERSION=$LINUX_VERSION -e LINUX_FLAVOUR=$LINUX_FLAVOUR -it its_cbpf_container /scripts/merge-gadget-victim-results.sh
