#!/bin/bash

cat files-0/var/log/messages | awk '{ print $2, $3, $4, $5 }' | grep cwnd | awk '{ print ($1 - 300)"."$2, $4 }' | uniq > tcp-single-flow-single-bottleneck-cwnd.dat
