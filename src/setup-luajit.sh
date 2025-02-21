#!/bin/sh

mkdir ~/luajit
cd ~/luajit
git clone https://luajit.org/git/luajit.git ./repo
mkdir ./build
cd ./repo
make PREFIX=~/luajit/build
make install PREFIX=~/luajit/build
echo 'export PATH=$PATH:$HOME/luajit/build/bin' >> ~/.bashrc
