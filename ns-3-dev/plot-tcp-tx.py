#!/usr/bin/env python2
import matplotlib
matplotlib.use("agg")
import matplotlib.pyplot as plt
import sys
import numpy as np

# Estimate delay is argv1
title = "ECDF of TX interval (s)"
filename = 'tcp-tx.pdf'
f = open(sys.argv[1])
a = np.array([])
throughput = []
for line in f:
    columns = line.split()
    a = np.append(a, float(columns[1]))
f.close()

x = np.sort(a)
y = np.arange(len(x))/float(len(x))

plt.xlabel('Interarrival (s)')
plt.ylabel('ECDF')
plt.title(title, fontdict = {'fontsize' : 20})
plt.plot(x, y, marker='', color='black', label='ecdf')
plt.legend(loc='lower right')
plt.ticklabel_format(useOffset=False)
#plt.show()
plt.savefig(filename, format='pdf')
plt.close()
