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

def plot_lang_comp(cpp_df: pd.DataFrame, lua_df: pd.DataFrame) -> None:
    naive_cpp_df = cpp_df[cpp_df['OPERATION_MODE'] == 1].groupby('SIZE').median().reset_index()
    naive_lua_df = lua_df[lua_df['OPERATION_MODE'] == 1].groupby('SIZE').median().reset_index()
    line_cpp_df = cpp_df[(cpp_df['OPERATION_MODE'] == 2) & (cpp_df['SIZE'] <= 3000)].groupby('SIZE').median().reset_index()
    line_lua_df = lua_df[lua_df['OPERATION_MODE'] == 2].groupby('SIZE').median().reset_index()

    with plot_theme:
        plt.figure()

        plt.plot(naive_cpp_df['SIZE'], naive_cpp_df['TIME'], label='Naive C++', marker='.')
        plt.plot(naive_lua_df['SIZE'], naive_lua_df['TIME'], label='Naive Lua', marker='.')
        plt.plot(line_cpp_df['SIZE'], line_cpp_df['TIME'], label='Line C++', marker='.')
        plt.plot(line_lua_df['SIZE'], line_lua_df['TIME'], label='Line Lua', marker='.')

        plt.xticks(range(600, 3001, 400))

        plt.title('Naive vs Line by Language')
        plt.xlabel('Size')
        plt.ylabel('Time (s)')
        plt.legend()

        output_path = os.path.join(output_folder, 'naive-line.pdf')
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
    plot_lang_comp(cpp_df, lua_df)

