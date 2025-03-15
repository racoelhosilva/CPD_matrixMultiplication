#!/bin/sh

g++ -O2 matrixproduct.cpp -o matrixproduct -lpapi -fopenmp -DL3

for size in $(seq 4096 2048 10240); do
    ./matrixproduct "output/test-cpp-2-${size}-l3.csv" 2 "$size" "$size" "$size"
done

for size in $(seq 4096 2048 10240); do
    for block_size in "128" "256" "512"; do
        ./matrixproduct "output/test-cpp-3-${size}-${block_size}-l3.csv" 3 "$size" "$size" "$size" "$block_size"
    done
done