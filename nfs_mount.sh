#!/bin/bash
#mount -t nfs 100.0.0.5:/data0 /data0/FRB_search -o proto=tcp -o nolock
#echo "NFS mount successfully!"

mount -t nfs 100.0.0.5:/home/nch/Tianlai_dedispersion_pipe /home/nch/Tianlai_dedispersion_pipe  -o nolock #-o proto=tcp

echo "NFS mount successfully!"
