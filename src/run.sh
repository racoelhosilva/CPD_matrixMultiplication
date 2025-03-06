#!/bin/sh

g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi -fopenmp

for size in $(seq 600 400 3000); do
    ./matrixproduct "output/test-cpp-1-${size}.csv" 1 "$size"
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    ./matrixproduct "output/test-cpp-2-${size}.csv" 2 "$size"
done

for size in $(seq 4096 2048 10240); do
    for block_size in "128" "256" "512"; do
        ./matrixproduct "output/test-cpp-3-${size}-${block_size}.csv" 3 "$size" "$block_size"
    done
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    ./matrixproduct "output/test-cpp-4-${size}.csv" 4 "$size"
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    ./matrixproduct "output/test-cpp-5-${size}.csv" 5 "$size"
done

for size in $(seq 600 400 3000); do
    luajit matrixproduct.lua 1 "$size" "output/test-lua-1-${size}x${size}.csv"
done

for size in $(seq 600 400 3000); do
    luajit matrixproduct.lua 2 "$size" "output/test-lua-2-${size}x${size}.csv"
done