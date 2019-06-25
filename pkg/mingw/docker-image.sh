#!/bin/bash
cd "$(dirname "$0")"
mkdir tmp
sudo docker build -t maxrd2/arch-mingw -f Dockerfile "$PWD/tmp"
rmdir tmp
