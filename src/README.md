# Description

Source code for the first assignment of the Parallel and Distributed Computing course. The program calculates the product of two matrices with the given sizes, in both C++ and Lua.

# Setup

1. Install PAPI and OpenMP
2. Install LuaJIT through your package manager, or run the following commands if you're using a FEUP PC:

```bash
./setup-luajit.sh
. ~/.bashrc        # Sync terminal with changes, to have `luajit` in PATH
```

# Usage

1. To run a matrix product, run one of the following commands:

```bash
# In C++
g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi -fopenmp     # Compile
# Use the flag -DL3 to measure L3 cache misses
./matrixproduct <output-file> <op> <m> <n> <p> [<block-size>]

# In Lua
luajit matrixproduct.lua <output-file> <op> <m> <n> <p>
```

2. To run the program in interactive mode, run one of the following commands:

```bash
# In C++
g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi -fopenmp     # Compile
./matrixproduct <output-file>

# In Lua
luajit matrixproduct.lua <output-file>
```

3. To run the pile of tests, run the following command:

```bash
./run.sh  # Use run-l3.sh for running the relevant tests with L3 cache misses
```
