# Training Solo Test Suite

This folder contains the Training Solo test suite, it allows a researcher to
quickly test a variety of BPU related experiments. The test suite is
organized in test templates (`user/templates`) which can be run
individually. The test suite is designed to quickly test different
parameters, such as:

- Test different Branch types
- Randomize branch addresses
- Randomize branch history in different matters
- Train/test in user or kernel domain with one switch
- Do BTB or iBTB eviction

## Setup

Install the kernel modules

```sh
cd test-suite
sudo ./setup.sh
```

## Execute tests

To run one test, set the current architecture with `ARCH=` and the test
name (=header file name) with `TEST=` and run `./run.sh`.

```sh
sudo TEST=ind_ind_history_collision ARCH=GOLDEN_COVE ./run.sh
```

To run a complete folder, for example, the BTB properties experiments:

```sh
sudo ARCH=GOLDEN_COVE ./run_all.sh table3_btb_properties
```

To run all:

```sh
sudo ARCH=GOLDEN_COVE ./run_all.sh
```

## Results

See folder `refs/` for the tests output on the tested microarchitectures.

## Randomization tests

Templates for randomization experiments are included in `templates/randomization`.
You may want to adjust the template for you needs (e.g., only test for
async branch type collisions) or aggregate the results afterwards to search for
unexpected collisions.
You run the template as any other template, for example:

```sh
sudo TEST=randomize_set_bits ARCH=GOLDEN_COVE ./run.sh
```

## Adding a custom template

You can create your own test template by creating a new header file
in the folder `user/templates` and including the file in `user/templates.h`.
