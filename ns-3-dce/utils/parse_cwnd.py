import os
import sys
import numpy as np

start_node = int (sys.argv[1])
end_node = int (sys.argv[2])

'''
Function to get socket information
'''
def get_sockets(in_filepath):
  all_sockets = []
  curr_socket = []
  first_socket = 1
  with open(in_filepath) as in_file:
    for line in in_file:
      if "ss -a -e -i" in line:
        if first_socket:
          first_socket = 0
        else:
          all_sockets.append(curr_socket)
          curr_socket = []
      else:
        if not first_socket:
          curr_socket.append(line)
		
  all_sockets.append(curr_socket)
  return all_sockets

sender_node_id = range (start_node, end_node+1)
receiver_node_id = range (43, 64)

'''
Checks for the directory to store cwnd data
'''
if (not os.path.isdir("../results/gfc-dctcp/cwnd_data")):
  os.system("mkdir ../results/gfc-dctcp/cwnd_data")

'''
Collect the data from files folder corresponding 
to each sender node and stores in a file
'''
for i in sender_node_id:
  os.system ("cat ../files-"+str(i)+"/var/log/*/* > "+"../results/gfc-dctcp/cwnd_data/"+str(i))

'''
For each sender parse the cwnd and time value from 
the file containg all the data and store it in plotme file
'''
for i in sender_node_id:
  in_filepath = "../results/gfc-dctcp/cwnd_data/"+str(i)
  all_sockets = get_sockets(in_filepath)
  write_mode = 'w'
  for socket in all_sockets:
    time_found = 0
    cwnd_found = 0
    port_found = 0
    time = None
    cwnd = None
    for info in socket:
      if "NS3" in info:
        time = info.split("NS3 Time:")[1].split(',')[0].strip().split('(')[1].strip().strip(')').strip().strip('+').strip('ns')
        time_format = float(time)/1000000000
        time_found = 1
      if ":50000" or ":50001" or ":50002" or ":50003" or ":50004" or ":50005" or ":50006" or ":50007" or ":50008" or ":50009" in info:
        port_found = 1
			
      if "cwnd" in info:
        cwnd = info.split("cwnd:")[1].split()[0]
        if port_found:
          if (i >= 3 and i <= 12):
            with open("../results/gfc-dctcp/cwnd_data/S1"+ str(i-3) + "-linux.txt", write_mode) as out_file:
              out_file.write(str(time_format) + " " + str(cwnd) + "\n")
            write_mode = 'a'
            port_found = 0
          elif (i >= 13 and i <= 32):
            with open("../results/gfc-dctcp/cwnd_data/S2"+ str(i-13) + "-linux.txt", write_mode) as out_file:
              out_file.write(str(time_format) + " " + str(cwnd) + "\n")
            write_mode = 'a'
            port_found = 0
          elif (i >= 33 and i <= 42):
            with open("../results/gfc-dctcp/cwnd_data/S3"+ str(i-33) + "-linux.txt", write_mode) as out_file:
              out_file.write(str(time_format) + " " + str(cwnd) + "\n")
            write_mode = 'a'
            port_found = 0

  if (i >= 3 and i <= 12):
    f = open("../results/gfc-dctcp/cwnd_data/S1"+ str(i-3) + "-linux.txt", 'r')
  elif (i >= 13 and i <= 32):
    f = open("../results/gfc-dctcp/cwnd_data/S2"+ str(i-13) + "-linux.txt", 'r')
  elif (i >= 33 and i <= 42):
    f = open("../results/gfc-dctcp/cwnd_data/S3"+ str(i-33) + "-linux.txt", 'r')
  data = f.readlines()
  arr=[]
  for lines in data:
    line=lines.split(" ")
    arr1=[];
    arr1.append(float(line[0]))
    arr1.append(line[1])
    arr.append(arr1)
  arr2=sorted(arr, key=lambda x: x[0])
  if (i >= 3 and i <= 12):
    f = open("../results/gfc-dctcp/cwnd_data/S1"+ str(i-3) + "-linux.plotme", 'w')
  elif (i >= 13 and i <= 32):
    f = open("../results/gfc-dctcp/cwnd_data/S2"+ str(i-13) + "-linux.plotme", 'w')
  elif (i >= 33 and i <= 42):
    f = open("../results/gfc-dctcp/cwnd_data/S3"+ str(i-33) + "-linux.plotme", 'w')
  for line in arr2:
    f.write(str(line[0])+" "+str(line[1]))
  f.close()

  if (i >= 3 and i <= 12):
    os.remove("../results/gfc-dctcp/cwnd_data/S1"+ str(i-3) + "-linux.txt")
  elif (i >= 13 and i <= 32):
    os.remove("../results/gfc-dctcp/cwnd_data/S2"+ str(i-13) + "-linux.txt")
  elif (i >= 33 and i <= 42):
    os.remove("../results/gfc-dctcp/cwnd_data/S3"+ str(i-33) + "-linux.txt")
