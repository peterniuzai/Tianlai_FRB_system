#!/bin/bash

pdsh -a "pkill -9 heimdall"

pdsh -a "pkill -9 python"
#pdsh -a "pkill -INT | cat "
#pdsh -a "/home/nch/Tianlai_dedispersion_pipe/kill_python_scripts.sh"

