#!/usr/bin/env python2
import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt
import sys

# Estimate delay is argv1

filename = 'tcp-queue-depth.pdf'

f = open(sys.argv[1])
t = []
latency = []
for line in f:
    columns = line.split()
    t.append(float(columns[0]))
    latency.append (float(columns[1]))
f.close()

#df= pd.DataFrame({'x1': enqueue, 'x2' : dequeue, 'y': cum_bytes})
title = "queue occupancy (ms)"
plt.xlabel('Time (s)')
plt.ylabel('queue occupancy (ms)')
plt.title(title, fontdict = {'fontsize' : 20})
plt.plot(t, latency, marker='', color='black')
plt.ticklabel_format(useOffset=False)
#plt.show()
plt.savefig(filename, format='pdf')
plt.close()
