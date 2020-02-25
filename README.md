# Reproduce DCTCP result in ns-3

Step 1: Download standard ns-3-dce:
```
git clone https://gitlab.com/nsnam/bake
export BAKE_HOME=`pwd`/bake
export PATH=$PATH:$BAKE_HOME
export PYTHONPATH=$PYTHONPATH:$BAKE_HOME

mkdir ~/dctcp-results
cd ~/dctcp-results
bake.py configure -e dce-linux-dev
bake.py download
```

Step 2: Replace downloaded ns-3-dce, ns-3-dev and net-next-nuse with this repository

Step 3: Build the ns-3
```
cd ~/dctcp-results
bake.py build
```

Step 4: To generate ns-3 validation results (Section 4.2)
```
cd ~/dctcp-results/source/ns-3-dev
sed -i -e 's/--enableEcn=0/--enableEcn=1/g' run-single.sh
sed -i -e 's/TcpNewReno/TcpDctcp/g' run-single.sh
./run-single.sh
```

Step 5: Similarly to generate Linux DCE validation results (Section 4.2)
```
cd ~/dctcp-results/source/ns-3-dce
./run-single.sh
```

Step 6: To generate the fallback to New reno results (Section 4.3)
```
cd ~/dctcp-results/source/ns-3-dev
sed -i -e 's/--enableEcn=1/--enableEcn=0/g' run-single.sh
sed -i -e 's/TcpDctcp/TcpNewReno/g' run-single.sh
./run-single.sh
# Retain all the results

# Repeat for New Reno
sed -i -e 's/--enableEcn=1/--enableEcn=0/g' run-single.sh
sed -i -e 's/TcpNewReno/TcpDctcp/g' run-single.sh
./run-single.sh
```

Step 7: To generate the evaluation results (Section 5)
```
cd  ~/dctcp-results/source/ns-3-dev
./waf --run dctcp-single-flow
```
