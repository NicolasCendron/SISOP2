#!/bin/sh
cd ../
make clean
make
./app user group2 127.0.0.1 7002
cd tests