#!/bin/bash

pdsh -a "rm -rf /FRBTMPFS/Beam*"
sleep 1
pdsh -w ssh:gpu1 "rm -rf /home/nch/Tianlai_dedispersion_pipe/Coincidencer/*.cand"
sleep 1
pdsh -w ssh:gpu1 "rm -rf /home/nch/Tianlai_dedispersion_pipe/Flags/*.flag"
sleep 1
pdsh -a "rm -rf /FRBTMPFS/Beam*"
sleep 1
pdsh -w ssh:gpu1 "rm -rf /home/nch/Tianlai_dedispersion_pipe/Coincidencer/*.cand"
sleep 1
pdsh -w ssh:gpu1 "rm -rf /home/nch/Tianlai_dedispersion_pipe/Flags/*.flag"
