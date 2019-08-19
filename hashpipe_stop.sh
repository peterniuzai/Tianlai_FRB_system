#!/bin/bash

pdsh -a "pkill -9 hashpipe"
sleep 1
pdsh -a "hashpipe_clean_shmem -I 0"
pdsh -a "hashpipe_clean_shmem -I 1"
pdsh -a "hashpipe_clean_shmem -I 2"
pdsh -a "hashpipe_clean_shmem -I 3"
sleep 2
pdsh -a "pkill -9 hashpipe"
