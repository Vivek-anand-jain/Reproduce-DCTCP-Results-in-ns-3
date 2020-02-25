#!/usr/bin/env python2
import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt
import sys

# Estimate delay is argv1
title = "TCP congestion window (segments)"
filename = 'tcp-cwnd.pdf'
f = open(sys.argv[1])
t = []
t2 = []
cwnd = []
for line in f:
    columns = line.split()
    t.append(float(columns[0]))
    cwnd.append (float(columns[1]))
f.close()

#df= pd.DataFrame({'x1': enqueue, 'x2' : dequeue, 'y': cum_bytes})
plt.xlabel('Time (s)')
plt.ylabel('cwnd (segments)')
#plt.xlim([5,10])
plt.title(title, fontdict = {'fontsize' : 20})
plt.plot(t, cwnd, marker='', color='black')
plt.ticklabel_format(useOffset=False)
#plt.show()
plt.savefig(filename, format='pdf')
plt.close()
