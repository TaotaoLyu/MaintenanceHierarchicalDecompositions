<h1 align="center">Maintenance Hierarchical Decompositions</h1>

This project maintains hierarchical graph decompositions under dynamic edge updates. It is implemented in C++14 and uses OpenMP for parallel execution.

## Build requirements

- CMake 3.16 or later
- A C++14-compatible compiler, such as GCC or Clang
- OpenMP development support

On Debian or Ubuntu, the required build tools can be installed with:

```bash
sudo apt update
sudo apt install build-essential cmake
```

When building with Clang, install its OpenMP runtime if it is not already available:

```bash
sudo apt install libomp-dev
```

## Build

Run the following commands from the project root:

```bash
cmake -S . -B build
cmake --build build -j
```

Optional native CPU optimizations can be enabled with:

```bash
cmake -S . -B build -DMHD_NATIVE_OPTIMIZATION=ON
cmake --build build -j
```

## Run

The executable accepts one graph file. Each line in the input file contains two vertex IDs representing an undirected edge.

Run from the project root:

```bash
./build/mhd <graph_file>
```

Run from the `build` directory:

```bash
./mhd <graph_file>
```

Example from the project root:

```bash
./build/mhd Data/pokec.txt
```

The command-line usage and a corresponding example are also defined in `app/src/main.cpp`. Running the executable without a graph argument prints them automatically.

The main workflow builds the decomposition tree, removes the selected edges, validates the result, inserts the edges again, and performs a final validation.
