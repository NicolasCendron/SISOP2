#!/bin/sh
cd ../
make clean
make
./app user group 127.0.0.1 7001
cd tests