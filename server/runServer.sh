#!/bin/sh
make -f MakeServer clean
make -f MakeServer
./server "5001"