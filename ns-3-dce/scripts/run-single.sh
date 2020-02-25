#!/bin/bash

./waf --run 'dctcp-single-flow --enableEcn=0 --baseRtt=1us'

bash cwnd.parse.dctcp.sh

python plot-tcp-cwnd.py tcp-single-flow-single-bottleneck-cwnd.dat 
python plot-tcp-rtt.py tcp-single-flow-single-bottleneck-rtt.dat 
python plot-tcp-throughput.py tcp-single-flow-single-bottleneck-throughput.dat 
python plot-queue-depth.py tcp-single-flow-single-bottleneck-queue-depth.dat
python plot-tcp-tx.py tcp-single-flow-single-bottleneck-tx.dat

/usr/bin/pdfjam tcp-cwnd.pdf tcp-rtt.pdf tcp-queue-depth.pdf tcp-throughput.pdf --nup 2x2 --landscape --outfile dce-noecn-0ms.pdf
