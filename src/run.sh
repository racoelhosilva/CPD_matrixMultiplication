#!/bin/sh

g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi

for size in $(seq 600 400 3000); do
    # TODO(Process-ing): Are the increments the same for both matrices?
    ./matrixproduct 1 "$size" "$size" "output/test-cpp-1-${size}x${size}.csv"
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    ./matrixproduct 2 "$size" "$size" "output/test-cpp-2-${size}x${size}.csv"
done

for size in $(seq 4096 2048 10240); do
    for block_size in "128" "256" "512"; do
        ./matrixproduct 3 "$size" "$size" "output/test-cpp-3-${size}x${size}-${block_size}.csv" "$block_size"
    done
done

for size in $(seq 600 400 3000); do
    # TODO(Process-ing): Are the increments the same for both matrices?
    luajit matrixproduct.lua 1 "$size" "$size" "output/test-lua-1-${size}x${size}.csv"
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    luajit matrixproduct.lua 2 "$size" "$size" "output/test-lua-2-${size}x${size}.csv"
done
