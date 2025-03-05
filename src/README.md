# Usage

1. Install LuaJIT through your package manager, or run the following commands if you're using a FEUP PC:

```bash
./setup-luajit.sh
. ~/.bashrc  # Sync terminal with changes, to have `luajit` in PATH
```

2. Execute one of the following sequence of commands:

```bash
# Run C++ program
g++ -std=c++17 -O2 matrixproduct.cpp -o matrixproduct -lpapi  # Make sure you have PAPI installed
./matrixproduct <op> <size> <output> [blockSize]

# Run Lua program
luajit matrixproduct.lua <op> <size> <output>

# Run pile of tests
./run.sh
```

<!-- TODO: Check if these are the commands we want to execute in the end -->
