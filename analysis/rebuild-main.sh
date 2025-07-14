#!/bin/bash
set -e

# Stop running container if any
docker stop its_container || echo ""
docker rm its_container || echo ""

# Build image.
docker build . --network=host --tag btc-eval

# Start container.
docker run -it --name its_container -d -v $(pwd)/../$1/results:/results -v $(pwd)/../$1/work-dir:/work-dir --privileged btc-eval bash

./run-main.sh $1
