#!/usr/bin/env python2
import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt
import sys
import numpy as np

# Estimate delay is argv1
title = "TCP throughput"
filename = 'tcp-throughput.pdf'
f = open(sys.argv[1])
t = []
t2 = []
throughput = []
for line in f:
    columns = line.split()
    t.append(float(columns[0]))
    throughput.append (float(columns[1]))

f.close()


tstart = np.nonzero(throughput)[0][0]
n = np.arange(1,np.size(t)+1)
cumavetput = np.cumsum(throughput) / (np.maximum(1, n - tstart))


#df= pd.DataFrame({'x1': enqueue, 'x2' : dequeue, 'y': cum_bytes})
plt.xlabel('Time (s)')
plt.ylabel('Throughput (Mb/s)')
plt.title(title, fontdict = {'fontsize' : 20})
plt.plot(t, throughput, marker='', color='black', label='200ms average')
plt.plot(t, cumavetput, marker='', color='orange', label='running average')
plt.legend(loc='lower right')
plt.ticklabel_format(useOffset=False)
#plt.show()
plt.savefig(filename, format='pdf')
plt.close()
