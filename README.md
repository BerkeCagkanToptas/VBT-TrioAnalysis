# VARIANT BENCHMARKING TOOLS (VBT)

**Author:** Berke Cagkan Toptas  
**Project Language:** C++11

VBT provides set of tools that is used for aligner/variant calling benchmarking. Currently there are three tools implemented:

    1. VARIANT COMPARISON
    2. MENDELIAN VIOLATION DETECTOR
    3. GRAPH VCF COMPARISON

### Quick Install:

```
git clone https://gitlab.sbgdinc.com/Variants/vbt.git
cd vbt
make
```

**Note:** If you are unable to compile program or getting runtime error, it is mostly because the project is still under heavy development. Please shoot me an email **(berke.toptas@sbgdinc.com)** for any problems, suggestions etc.

### Parameter format:

```
./vbt varComp [PARAMETERS]
./vbt mendelian [PARAMETERS]
./vbt graphComp [PARAMETERS]
```
For additional help, please use the following commands:

```
./vbt --help
./vbt [module_name] --help
```
