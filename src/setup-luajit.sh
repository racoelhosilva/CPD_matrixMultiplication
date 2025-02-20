#!/bin/sh

git clone https://luajit.org/git/luajit.git ~/luajit
cd ~/luajit
mkdir /tmp/luajit
make PREFIX=/tmp/luajit
make install PREFIX=/tmp/luajit
echo "export PATH=$PATH:/tmp/luajit/src/bin"
source ~/.bashrc
