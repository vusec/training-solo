# Training Solo

This repository contains the code and results of the paper
["Training Solo: On the Limitations of Domain Isolation Against Spectre-v2 Attacks"](https://download.vusec.net/papers/trainingsolo_sp25.pdf),
published at IEEE S&P'25.

For more information about the Training Solo project, self-training attacks, or
our test-suite, please see our [project page](https://www.vusec.net/projects/training-solo/) or the paper.

```txt
training-solo
├─ analysis/           # Analysis code
├─ pocs/               # POCs and exploits
│  ├─ history-exploit/ # History-based end-to-end exploit   (section 7)
│  ├─ its-exploit/     # ITS end-to-end exploit user-kernel (section 9)
│  ├─ its-vmm/         # ITS VMM Experiments                (section 10)
├─ test-suite/         # Training Solo test suite           (section 5)
├─ README.md           # You are here
```
