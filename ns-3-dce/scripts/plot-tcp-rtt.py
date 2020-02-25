#!/usr/bin/env python2
import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt
import sys

# Estimate delay is argv1
title = "TCP RTT latency"
filename = 'tcp-rtt.pdf'
f = open(sys.argv[1])
t = []
t2 = []
latency = []
for line in f:
    columns = line.split()
    t.append(float(columns[0]))
    latency.append (float(columns[1]))
f.close()

#df= pd.DataFrame({'x1': enqueue, 'x2' : dequeue, 'y': cum_bytes})
plt.xlabel('Time (s)')
plt.ylabel('RTT (ms)')
plt.ylim([0,100])
plt.title(title, fontdict = {'fontsize' : 20})
plt.plot(t, latency, marker='', color='black')
plt.ticklabel_format(useOffset=False)
#plt.show()
plt.savefig(filename, format='pdf')
plt.close()
