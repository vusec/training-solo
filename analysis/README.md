# Training Solo Analysis

This folder contains all scripts to run both the gadget and collision analysis performed for the Training Solo paper.

## Setup

Please initialize inspectre-gadget repo and apply the custom patch. This patch adds the secret-dependent branch covert channel and enhanced victim scanning features.

```sh
cd analysis
git init inspectre-gadget
cd inspectre-gadget
git apply ../inspectre_gadget_training_solo.patch
```

## Run the experiments

The analysis is divided in multiple experiments. Please see `run.sh` for a small description + experiment-figure mapping for each experiment.

To run the experiments with Linux kernel version 6.8.0-38.


```sh
cd analysis
./run.sh 6.8.0-38 generic
```

Please edit `run.sh` to run with a different version.

## Results

The results and working directory are outputted to `training-solo/linux6.8.0-38-generic`. The working directory caches experiment intermediate results to speed up the analysis at re-run.

See the folder `analysis/graphs` to generate the figures from the paper based on the results.

The raw results and used images are included as a release in the repository.
