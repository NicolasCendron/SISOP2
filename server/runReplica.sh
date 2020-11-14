#!/bin/sh
make -f MakeReplica clean
make -f MakeReplica
#./database/replica       # Rodar Normalmente a Replica
gdb -ex run ./database/replica  # Rodar MODO DEBUGGER a Replica. (CTRL-Z para sari no terminal)