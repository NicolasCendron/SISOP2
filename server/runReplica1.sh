#!/bin/sh
make -f MakeReplica clean
make -f MakeReplica
./database/replica 7001
