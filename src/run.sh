#!/bin/sh

g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi

for size in $(seq 600 400 3000); do
    ./matrixproduct 1 "$size" "output/test-cpp-1-${size}.csv"
done

for size in $(seq 600 400 3000) $(seq 4096 2048 10240); do
    ./matrixproduct 2 "$size" "output/test-cpp-2-${size}.csv"
done

for size in $(seq 4096 2048 10240); do
    for block_size in "128" "256" "512"; do
        ./matrixproduct 3 "$size" "output/test-cpp-3-${size}-${block_size}.csv" "$block_size"
    done
done

for size in $(seq 600 400 3000); do
    luajit matrixproduct.lua 1 "$size" "output/test-lua-1-${size}x${size}.csv"
done

for size in $(seq 600 400 3000); do
    luajit matrixproduct.lua 2 "$size" "output/test-lua-2-${size}x${size}.csv"
done
