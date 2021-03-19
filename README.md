# A multi-population multi-parent biased random key genetic algorithm for the home health care routing and scheduling problem

This repository contains an implementation of the BRKGA-MP-IPR metaheuristic for the daily home health care routing and scheduling problem, which is a generalization of the vehicle routing problem with time-windows. It also has additional constraints to model the home care problem's operations, such as multiple visits in some clients/patients following simultaneous attendances or precedence constraints. As the state-of-art exact solvers can not solve more than a few dozen patients, and we propose this genetic algorithm to find high-quality solutions heuristically using few minutes of processing. 

This repository contains the implementation of such a heuristic. It also contains a git submodule with a benchmark dataset for the problem. Please check [afkummer/gecco2020-brkga](https://github.com/afkummer/gecco2020-brkga). Our implementation is based in the library [brkga_mp_ipr](https://github.com/afkummer/brkga_mp_ipr_cpp), originally developed by [ceandrade](https://github.com/ceandrade/brkga_mp_ipr_cpp). Our fork only includes a small hotfix in one header of this library.

## Cloning this repository

Supposing you have the `git` command acessible in your shell, this simple command should be enough to clone the repository. 

```
$ git clone https://github.com/afkummer/brkga-mp-ipr-hhcrsp-2021.git
```

To initialize the submodules, you can issue the following command. But first you need to change to the recently cloned repository.

```
$ cd brkga-mp-ipr-hhcrsp-2021
$ git submodule update --init
```

## Software requirements

We have a few requirements to compile our code. You need a reasonably updated version of the GNU G++ compiler (we use the version 7.3.0), and you also need the CMake building system. Our code also depends upon Boost Program Options library for parsing the command line arguments. In Ubuntu 18.04, you can install all these dependencies in a single command.

```
$ sudo apt install g++ cmake libboost-program-options-dev
```

The `brkga_mp_ipr_cpp` uses OpenMP directives to enable parallel decoding of the individuals. Make sure if your system supports the OpenMP extensions, and make the necessary adjusts into the [CMakeLists.txt](CMakeLists.txt) to enable the support. Of course you can disable OpenMP, if you are willing to accept the performance impact. In our tests, the speedup achieved with multi-core processing is almost linear in the number of threads used.

## Building the binary

Once you have completed the previous steps, you are ready to compile the `brkga` binary. For that, you need to enter in the `build` directory and issue the following command to generate your `makefile`. The parameter `..` indicates to CMake to search for a `CMakeLists.txt` in the directory above. The parameter `-DCMAKE_BUILD_TYPE=Release` instructs the generation of a `makefile` with compilation commands optimized for performance.

```
$ cd build
$ cmake .. -DCMAKE_BUILD_TYPE=Release
```

Once completed, you can run the standard `$ make` utility to build the binary. After this step, you have generated the `brkga` binary. 

**Note:** The CMakeLists.txt contains instructions to generate a binary optimized for the native processor of the compilation machine. If you plan to run the code in other system, you need to either recompile the code into such machine, or change the [CMakeLists.txt](CMakeLists.txt) parameters to `-march=x86-64` and `-mtune=generic`. As a consequence, you may expect a small performance hit due to the target-agnostic binary.

## Running the meta-heuristic

To run the meta-heuristic, you only need to invoke the `brkga` command. By default, the program shows a quick summary of accepted parameters if you give none. For example, you can solve the instance `InstanzVNS_200_2.txt` using the following command line.

```
$ ./brkga --popsize 1000 --gens 1900 --pe 0.1 --pm 0.01 --mp 8 --ep 3 --bp constant --npop 3 --npairs 75 --mindist 0.001 --psel random --pperc 0.01 --ptype permutation --pperiod 180 --reset 700 --xelite 80 --immigrants 50 -i ../gecco2020-brkga/instances-HHCRSP/InstanzVNS_HCSRP_200_2.txt -s 13
```

All the progress of the search is logged out in the standard output. When the meta-heuristic finishes, the solution is then written to the text file indicated in the output.

## Automatic parameter setting through irace

The `brkga` command is highly parameterized, requiring a large human effort to manually set a good choice of values. Instead, we use the [irace](https://github.com/MLopez-Ibanez/irace) tool to automatically choose a effective parameter setting to the problem automatically. All the files you need to run your own automatic algorithm configuration experiment is inside the [aac-irace](aac-irace/) directory. We already run such experiment, and our output is available in the [aac-aac-irace/run-march14](aac-aac-irace/run-march14/) directory.

## Output logs of our experiment

We already extensively tested our meta-heuristic with the instance dataset proposed by [Mankowska et al. (2014)](https://link.springer.com/article/10.1007/s10729-013-9243-1). These files are available in the [logs](logs/) directory. In this experiments, we use the best configuration found by irace, according to [this logfile](aac-irace/run-march14/irace.log). We run the `brkga` using the four cores of a Intel i7-930 computer at 2.80 GHz, running Ubuntu 18.04. We also used the GNU G++ compiler version 7.3.0. Our system has 12 GB of memory--despite that the memory amount consumed by `brkga` is negligible.

