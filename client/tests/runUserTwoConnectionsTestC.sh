#!/bin/sh
cd ../
make clean
make
./app user group3 127.0.0.1 7003
cd tests