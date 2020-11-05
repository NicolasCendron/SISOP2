#!/bin/sh
make -f MakeReplica clean
make -f MakeReplica
gdb ./database/replica