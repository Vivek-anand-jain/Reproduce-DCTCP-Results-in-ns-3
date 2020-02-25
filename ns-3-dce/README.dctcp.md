# Validation-of-DCTCP

## Steps to reproduce results

1. Install DCE in your system.

2. Copy dce-dctcp-17.cc in ns-3-dce/examples folder and update the wscript.

3. Copy parse_cwnd.py in ns-3-dce/utils folder.

4. Copy the PlotScripts-gfc-dctcp folder in ns-3-dce directory and give execute permission to ns3-plotshellTp.sh and linux-plotshellTp.sh files inside ns-3-dce/PlotScripts-gfc-dctcp folder.

5. Instructions for running the example:

     1. Run the below command after successful compilation.\
        For ns-3 stack:\
        ```./waf --run "dce-dctcp-17 --stack=ns3 --stopTime=10 --transport_prot=TcpDctcp --queue_disc_type=RedQueueDisc --useEcn=true"```
        
        For Linux stack:\
        ```./waf --run "dce-dctcp-17 --stack=linux --stopTime=20 --linux_prot=dctcp --queue_disc_type=RedQueueDisc --useEcn=true"```
        
        Run the below command for a list of all the commandline arguments and their description.\
          ```./waf --run "dce-dctcp-17 --help"```
        
     2. A 'result' folder will be created in the main ns-3-dce directory. This folder contains a folder named gfc-dctcp. This folder contains all the results named according to the start date and time of the simulation.
     
     3. Go to the required folder. In this folder, various folders will be created containing different kinds of statistics.
     
     4. Go to the pcap folder and run the file ns3-plotshellTp.sh or linux-plotshellTp.sh (One of the files will be present depending for which stack simulation was run.)
     
     5. At the end of the execution of the above shell script, a folder named Graphs will contain all the graphs. 
