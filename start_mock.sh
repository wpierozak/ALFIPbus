#!/bin/bash
log_filename="log/logs_mock"
current_time=$(date +%Y%m%d%H%M%S);
log_filename="${log_filename}_${current_time}.txt"
bin/AlfIPbus \
            -n ALF_MOCK \
            -l 127.0.0.1:50001 \
            -t 1000 \
#            -f $log_filename
