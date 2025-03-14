#!/usr/bin/env python3

import csv
import sys
import os
import statistics

def calculate_median(input_file):
    output_file = os.path.splitext(input_file)[0] + '-med.csv'

    with open(input_file, newline='') as csvfile:
        reader = csv.reader(csvfile)
        headers = next(reader)
        columns = list(zip(*reader))

    medians = [statistics.median(map(float, col)) for col in columns]

    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(headers)
        writer.writerow(medians)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: median.py <input_csv_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    calculate_median(input_file)