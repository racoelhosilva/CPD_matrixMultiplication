#!/bin/sh

function print_usage() {
    echo "Usage: $0"    
}

if [ $# -ne 0 ]; then
    print_usage
    exit 1
fi

mkdir ~/luajit
cd ~/luajit
git clone https://luajit.org/git/luajit.git ./repo
mkdir ./build
cd ./repo
make PREFIX=~/luajit/build
make install PREFIX=~/luajit/build
echo 'export PATH=$PATH:$HOME/luajit/build/bin' >> ~/.bashrc
