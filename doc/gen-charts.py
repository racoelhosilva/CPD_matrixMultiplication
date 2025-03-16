#!/usr/bin/env python3

import os
from typing import Tuple
import pandas as pd
import matplotlib.pyplot as plt
from aquarel import load_theme

plot_theme = load_theme('ambivalent').set_font(sans_serif='Lato')

def import_csv_files(folder_path: str) -> Tuple[pd.DataFrame, pd.DataFrame]:
    cpp_dfs = []
    for filename in os.listdir(folder_path):
        if not filename.startswith('test-cpp') or not filename.endswith('.csv'):
            continue

        filepath = os.path.join(folder_path, filename)
        df = pd.read_csv(filepath)
        cpp_dfs.append(df)

    cpp_df = pd.concat(cpp_dfs, ignore_index=True)

    lua_dfs = []
    for filename in os.listdir(folder_path):
        if not filename.startswith('test-lua') or not filename.endswith('.csv'):
            continue

        filepath = os.path.join(folder_path, filename)
        df = pd.read_csv(filepath)
        lua_dfs.append(df)

    lua_df = pd.concat(lua_dfs, ignore_index=True)

    return (cpp_df, lua_df)

def plot_naive_line_time(cpp_df: pd.DataFrame, lua_df: pd.DataFrame) -> None:
    naive_cpp_df = cpp_df[cpp_df['OPERATION_MODE'] == 1].groupby('M').median().reset_index()
    naive_lua_df = lua_df[lua_df['OPERATION_MODE'] == 1].groupby('M').median().reset_index()
    line_cpp_df = cpp_df[(cpp_df['OPERATION_MODE'] == 2) & (cpp_df['M'] <= 3000)].groupby('M').median().reset_index()
    line_lua_df = lua_df[lua_df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(naive_cpp_df['M'], naive_cpp_df['TIME'], label='Naive C++', marker='.')
        plt.plot(naive_lua_df['M'], naive_lua_df['TIME'], label='Naive Lua', marker='.')
        plt.plot(line_cpp_df['M'], line_cpp_df['TIME'], label='Line C++', marker='.')
        plt.plot(line_lua_df['M'], line_lua_df['TIME'], label='Line Lua', marker='.')

        plt.xticks(range(600, 3001, 400))
        plt.ylim(bottom=0)

        plt.title('Execution Time of Naive vs Line by Language')
        plt.xlabel('Size')
        plt.ylabel('Time (s)')
        plt.legend()

        output_path = os.path.join(output_folder, 'naive-line-time.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_naive_line_flops(cpp_df: pd.DataFrame, lua_df: pd.DataFrame) -> None:
    naive_cpp_df = cpp_df[cpp_df['OPERATION_MODE'] == 1].groupby('M').median().reset_index()
    naive_lua_df = lua_df[lua_df['OPERATION_MODE'] == 1].groupby('M').median().reset_index()
    line_cpp_df = cpp_df[(cpp_df['OPERATION_MODE'] == 2) & (cpp_df['M'] <= 3000)].groupby('M').median().reset_index()
    line_lua_df = lua_df[lua_df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(naive_cpp_df['M'], naive_cpp_df['MFLOPS'], label='Naive C++', marker='.', )
        plt.plot(naive_lua_df['M'], naive_lua_df['MFLOPS'], label='Naive Lua', marker='.')
        plt.plot(line_cpp_df['M'], line_cpp_df['MFLOPS'], label='Line C++', marker='.')
        plt.plot(line_lua_df['M'], line_lua_df['MFLOPS'], label='Line Lua', marker='.')

        plt.xticks(range(600, 3001, 400))
        plt.ylim(bottom=0)

        plt.title('Mflops of Naive vs Line by Language')
        plt.xlabel('Size')
        plt.ylabel('Mflops')
        plt.legend()

        output_path = os.path.join(output_folder, 'naive-line-flops.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_line_block_time(df: pd.DataFrame) -> None:
    line_df = df[(df['OPERATION_MODE'] == 2) & (df['M'] >= 4096)].groupby('M').median().reset_index()
    block_128_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 128)].groupby('M').median().reset_index()
    block_256_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 256)].groupby('M').median().reset_index()
    block_512_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 512)].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(line_df['M'], line_df['TIME'], label='Line', marker='.')
        plt.plot(block_128_df['M'], block_128_df['TIME'], label='Block-128', marker='.')
        plt.plot(block_256_df['M'], block_256_df['TIME'], label='Block-256', marker='.')
        plt.plot(block_512_df['M'], block_512_df['TIME'], label='Block-512', marker='.')

        plt.xticks(range(4096, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Execution Time of Line and Block per Block Size')
        plt.xlabel('Size')
        plt.ylabel('Time (s)')
        plt.legend()

        output_path = os.path.join(output_folder, 'line-block-time.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_line_block_flops(df: pd.DataFrame) -> None:
    line_df = df[(df['OPERATION_MODE'] == 2) & (df['M'] >= 4096)].groupby('M').median().reset_index()
    block_128_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 128)].groupby('M').median().reset_index()
    block_256_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 256)].groupby('M').median().reset_index()
    block_512_df = df[(df['OPERATION_MODE'] == 3) & (df['BLOCK_SIZE'] == 512)].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(line_df['M'], line_df['MFLOPS'], label='Line', marker='.')
        plt.plot(block_128_df['M'], block_128_df['MFLOPS'], label='Block-128', marker='.')
        plt.plot(block_256_df['M'], block_256_df['MFLOPS'], label='Block-256', marker='.')
        plt.plot(block_512_df['M'], block_512_df['MFLOPS'], label='Block-512', marker='.')

        plt.xticks(range(4096, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Mflops of Line and Block per Block Size')
        plt.xlabel('Size')
        plt.ylabel('Mflops')
        plt.legend()

        output_path = os.path.join(output_folder, 'line-block-flops.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_parallel_time(df: pd.DataFrame) -> None:
    line_df = df[df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()
    parallel_1_df = df[df['OPERATION_MODE'] == 4].groupby('M').median().reset_index()
    parallel_2_df = df[df['OPERATION_MODE'] == 5].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(line_df['M'], line_df['TIME'], label='Line', marker='.')
        plt.plot(parallel_1_df['M'], parallel_1_df['TIME'], label='Parallel V.1', marker='.')
        plt.plot(parallel_2_df['M'], parallel_2_df['TIME'], label='Parallel V.2', marker='.')

        plt.xticks(range(0, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Execution Time of Sequential vs Parallel Line')
        plt.xlabel('Size')
        plt.ylabel('Time (s)')
        plt.legend()

        output_path = os.path.join(output_folder, 'parallel-time.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_parallel_flops(df: pd.DataFrame) -> None:
    line_df = df[df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()
    parallel_1_df = df[df['OPERATION_MODE'] == 4].groupby('M').median().reset_index()
    parallel_2_df = df[df['OPERATION_MODE'] == 5].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(line_df['M'], line_df['MFLOPS'], label='Line', marker='.')
        plt.plot(parallel_1_df['M'], parallel_1_df['MFLOPS'], label='Parallel V.1', marker='.')
        plt.plot(parallel_2_df['M'], parallel_2_df['MFLOPS'], label='Parallel V.2', marker='.')

        plt.xticks(range(0, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Mflops of Sequential vs Parallel Line')
        plt.xlabel('Size')
        plt.ylabel('Mflops')
        plt.legend()

        output_path = os.path.join(output_folder, 'parallel-flops.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_parallel_speedup(df: pd.DataFrame) -> None:
    line_df = df[df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()
    parallel_1_df = df[df['OPERATION_MODE'] == 4].groupby('M').median().reset_index()
    parallel_2_df = df[df['OPERATION_MODE'] == 5].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(parallel_1_df['M'], line_df['TIME'] / parallel_1_df['TIME'], label='Parallel V.1', marker='.')
        plt.plot(parallel_2_df['M'], line_df['TIME'] / parallel_2_df['TIME'], label='Parallel V.2', marker='.')

        plt.xticks(range(0, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Speedup of Parallel Line')
        plt.xlabel('Size')
        plt.ylabel('Speedup')
        plt.legend()

        output_path = os.path.join(output_folder, 'parallel-speedup.pdf')
        plt.savefig(output_path)
        plt.close()

def plot_parallel_efficiency(df: pd.DataFrame) -> None:
    line_df = df[df['OPERATION_MODE'] == 2].groupby('M').median().reset_index()
    parallel_1_df = df[df['OPERATION_MODE'] == 4].groupby('M').median().reset_index()
    parallel_2_df = df[df['OPERATION_MODE'] == 5].groupby('M').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(parallel_1_df['M'], line_df['TIME'] / parallel_1_df['TIME'] / 8, label='Parallel V.1', marker='.')
        plt.plot(parallel_2_df['M'], line_df['TIME'] / parallel_2_df['TIME'] / 8, label='Parallel V.2', marker='.')

        plt.xticks(range(0, 10241, 2048))
        plt.ylim(bottom=0)

        plt.title('Efficiency of Parallel Line')
        plt.xlabel('Size')
        plt.ylabel('Efficiency')
        plt.legend()

        output_path = os.path.join(output_folder, 'parallel-efficiency.pdf')
        plt.savefig(output_path)
        plt.close()

if __name__ == '__main__':
    folder_path = 'csv/'
    if not os.path.exists(folder_path):
        print(f'[fail] \'{folder_path}\' does not exist')
        exit(1)
    if not os.path.isdir(folder_path):
        print(f'[fail] \'{folder_path}\' is not a directory')
        exit(1)

    output_folder = 'latex/pdf/'
    os.makedirs(output_folder, exist_ok=True)

    (cpp_df, lua_df) = import_csv_files(folder_path)

    plot_naive_line_time(cpp_df, lua_df)
    plot_naive_line_flops(cpp_df, lua_df)
    plot_line_block_time(cpp_df)
    plot_line_block_flops(cpp_df)
    plot_parallel_time(cpp_df)
    plot_parallel_flops(cpp_df)
    plot_parallel_speedup(cpp_df)
    plot_parallel_efficiency(cpp_df)

