#!/bin/bash
log_filename="log/logs"
current_time=$(date +%Y%m%d%H%M%S);
log_filename="${log_filename}_${current_time}.txt"
bin/AlfIPbus \
            -n ALF_FTM \
            -l 172.20.75.175:50001 \
            -t 1000 \
            -f $log_filename \
#            -v