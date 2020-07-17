#!/bin/bash
python /home/nch/Tianlai_dedispersion_pipe/sys_trig.py
hashpipe -p TL_hashpipe -I 0 -o BINDHOST="p1p1" -c 0 TL_net_thread -c 2 TL_gpu_thread -c 4 TL_output_thread &
hashpipe -p TL_hashpipe -I 1 -o BINDHOST="p1p2" -c 6 TL_net_thread -c 8 TL_gpu_thread -c 10 TL_output_thread &
hashpipe -p TL_hashpipe -I 2 -o BINDHOST="p4p1" -c 1 TL_net_thread -c 3 TL_gpu_thread -c 5 TL_output_thread &
hashpipe -p TL_hashpipe -I 3 -o BINDHOST="p4p2" -c 7 TL_net_thread -c 9 TL_gpu_thread -c 11 TL_output_thread 

