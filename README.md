# RISC-V Transient Execution Attacks

This repository holds all the work-in-progress code used to check if RISC-V implementations are susceptible to transient execution attacks (TEAs).

# Target configurations

```

T-Head TH1520 SoC (quad XuanTie C910 cores) of Sipeed Lichee Pi 4A SBC

```

# Implemented attacks

The following attacks are implemented within the repo.

## CVE-2017-5753: Spectre-BCB (Bounds Check Bypass) or Spectre-v1 [1]
   * Spectre-BCB.c
## CVE-2017-5715: Spectre-BTI (Branch Target Injection) or Spectre-v2 [1]
   * Spectre-BTI.c
## CVE-2018-15572: Spectre-RSB (Return Stack Buffer attack) or Spectre-v5 [2]
   * Spectre-RSB.c + swStackGadget.S
   * May not succeed on machines w/o RAS (Return Address Stack) or something similar.
## CVE-2018-3639: Spectre-SSB (Speculative Store Bypass) or Spectre-v4
   * Spectre-SSB.c
   * Will not succeed on machines w/o MDP (Memory Dependence Prediction) mechanisms.

# W. I. P. / Not completed attacks

The following attacks are in-progress and have not been prepared for use yet.

# Building the tests

To build you need to run `make`

# Running the Tests

Built "baremetal" binaries can directly run on targets that are specified above.
```
path/to/a_binary_name_above.riscv
```
Depending on implementations, you may also need Linux proxy kernel (pk).

# References

[1] P. Kocher, D. Genkin, D. Gruss, W. Haas, M. Hamburg, M. Lipp, S. Mangard, T. Prescher, M. Schwarz, and Y. Yarom, “Spectre attacks: Exploiting speculative execution,” ArXiv e-prints, Jan. 2018

[2] E. M. Koruyeh, K. N. Khasawneh, C. Song, N. Abu-Ghazaleh, “Spectre Returns! Speculation Attacks using the Return Stack Buffer,” 12th USENIX Workshop on Offensive Technologies, 2018
