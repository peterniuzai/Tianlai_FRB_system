#!/bin/bash

./hashpipe_init.sh &
sleep 5
./coincidencer_init.sh &

sleep 2
./dedisperse_init.sh &
./result_handle.sh
