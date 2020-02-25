/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Test TCP single flow single bottleneck
// 
// Variables:
//   * TCP type (Cubic, Reno, DCTCP, Prague)
//   * bandwidth of bottleneck
//   * base RTT of path
//   * flow time (unlimited file size)
//   * Initial window
//   * Pacing enabled
//   * Pacing threshold
//   * Queue type (FQ/CoDel, RED)
//   * ECN enabled on queue or not
//
// Traces:
//   * Throughput vs time
//   * Queue length vs time
//   * Congestion window vs time
//   * Estimated RTT vs time
//   * Segment transmit intervals (pacing) vs time
//
//  sender -> bottleneck -> receiver
//
// The ns-3 FQ-CoDel model uses ns-3 defaults:
// - 100ms interval
// - 5ms target
// - drop batch size of 64 packets
// - minbytes of 1500
//
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/dce-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpSingleFlowSingleBottleneck");

uint32_t g_bytesReceived = 0;
uint32_t g_marksObserved = 0;
uint32_t g_dropsObserved = 0;

uint32_t g_tcpSegmentSize = 1448;

Time lastTx = Seconds (0);

void
TracedValueCwnd (std::ofstream* ofStream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // TCP segment size is configured below to be 1448 bytes
  // report cwnd in segments
  *ofStream << Simulator::Now ().GetSeconds () << " " << static_cast<double> (newCwnd)/g_tcpSegmentSize << std::endl;
}

void
TracedValueRtt (std::ofstream* ofStream, Time oldRtt, Time newRtt)
{
  // output in units of ms
  *ofStream << Simulator::Now ().GetSeconds () << " " << newRtt.GetSeconds () * 1000 << std::endl;
}

void
TraceTx (std::ofstream* ofStream, Ptr<const Packet> packet)
{
  if (lastTx > Seconds (0))
    {
      *ofStream << Simulator::Now ().GetSeconds () << " " << std::fixed  << (Simulator::Now () - lastTx).GetSeconds () << std::endl;
    }
  lastTx = Simulator::Now ();
}

void
TraceRx (Ptr<const Packet> packet, const Address &address)
{
  // output in units of bytes
  g_bytesReceived += packet->GetSize ();
}

void
TraceQueueDepth (std::ofstream* ofStream, DataRate linkRate, uint32_t oldVal, uint32_t newVal)
{
  // output in units of ms
  *ofStream << Simulator::Now ().GetSeconds () << " " << std::fixed << static_cast<double> (newVal * 8) / (linkRate.GetBitRate () / 1000) << std::endl;
}

void
TraceMarks (std::ofstream* ofStream, Ptr<const QueueDiscItem> item, const char* reason)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << std::hex << item->Hash () << " " << reason << std::endl;
  g_marksObserved++;
}

void
TraceDrops (std::ofstream* ofStream, Ptr<const QueueDiscItem> item)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << std::hex << item->Hash () << std::endl;
  g_dropsObserved++;
}

void
TraceThroughput (std::ofstream* ofStream, Time throughputInterval)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << g_bytesReceived * 8 / throughputInterval.GetSeconds () / 1e6 << std::endl;
  g_bytesReceived = 0;
  Simulator::Schedule (throughputInterval, &TraceThroughput, ofStream, throughputInterval);
}

void
ScheduleTcpCwndTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&TracedValueCwnd, ofStream));
}

void
ScheduleTcpRttTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeBoundCallback (&TracedValueRtt, ofStream));
}

void
SchedulePacketSinkConnection (void)
{
  Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&TraceRx));
}

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  bool enablePacing = true;
  bool paceInitialWindow = false;
  bool enablePcap = false;
  bool enableEcn = true;
  std::string tcpTypeId = "TcpDctcp";
  Time simulationStopTime = Seconds (30);
  Time baseRtt = MilliSeconds (50);
  Time throughputSamplingInterval = MilliSeconds (200);
  DataRate bottleneckRate ("50Mbps");
  Time minThTarget = MilliSeconds (5);
  std::string rttTraceFile = "tcp-single-flow-single-bottleneck-rtt.dat";
  std::string cwndTraceFile = "tcp-single-flow-single-bottleneck-cwnd.dat";
  std::string throughputTraceFile = "tcp-single-flow-single-bottleneck-throughput.dat";
  std::string queueDepthTraceFile = "tcp-single-flow-single-bottleneck-queue-depth.dat";
  std::string transmitIntervalTraceFile = "tcp-single-flow-single-bottleneck-transmit-interval.dat";
  std::string marksTraceFile = "tcp-single-flow-single-bottleneck-marks.dat";
  std::string dropsTraceFile = "tcp-single-flow-single-bottleneck-drops.dat";
  std::string txTraceFile = "tcp-single-flow-single-bottleneck-tx.dat";

  ////////////////////////////////////////////////////////////
  // Override ns-3 defaults                                 //
  ////////////////////////////////////////////////////////////
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (g_tcpSegmentSize));
  // Increase default buffer sizes to improve throughput over long delay paths
  Config::SetDefault ("ns3::TcpSocket::SndBufSize",UintegerValue (8192000));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize",UintegerValue (8192000));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));

  ////////////////////////////////////////////////////////////
  // command-line argument parsing                          //
  ////////////////////////////////////////////////////////////
  CommandLine cmd;
  cmd.AddValue ("enablePacing", "enable pacing", enablePacing);
  cmd.AddValue ("paceInitialWindow", "pace initial window", paceInitialWindow);
  cmd.AddValue ("enablePcap", "enable Pcap", enablePcap);
  cmd.AddValue ("enableEcn", "enable ECN", enableEcn);
  cmd.AddValue ("baseRtt", "base RTT (ms)", baseRtt);
  cmd.AddValue ("bottleneckRate", "data rate of bottleneck", bottleneckRate);
  cmd.Parse (argc, argv);

  Time oneWayDelay = baseRtt / 2;

  uint64_t bitRate = bottleneckRate.GetBitRate ();

  uint32_t minThInPackets = (bitRate * minThTarget.GetSeconds ())/(1500 * 8);

  ////////////////////////////////////////////////////////////
  // Override ns-3 defaults based on command-line arguments //
  ////////////////////////////////////////////////////////////
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::" + tcpTypeId));
  Config::SetDefault ("ns3::TcpSocketState::EnablePacing", BooleanValue (enablePacing));
  Config::SetDefault ("ns3::TcpSocketState::PaceInitialWindow", BooleanValue (paceInitialWindow));

  // Set default parameters for RED queue disc
  Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (enableEcn));
  Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::UseHardDrop", BooleanValue (false));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (1500));
  // DCTCP tracks instantaneous queue length only; so set QW = 1
  Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (1));
  // Triumph and Scorpion switches used in SIGCOMM paper have 4 MB of buffer
  // If every packet is 1500 bytes, 2666 packets can be stored in 4 MB
  Config::SetDefault ("ns3::RedQueueDisc::MaxSize", QueueSizeValue (QueueSize ("2666p")));
  // MinTh = MaxTh = 17% of QueueLimit as recommended in ACM SIGCOMM 2010 DCTCP Paper
#if 0
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (453));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (453));
#endif
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (minThInPackets));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (2*minThInPackets));

  std::ofstream tcpRttOfStream;
  tcpRttOfStream.open (rttTraceFile.c_str (), std::ofstream::out);
  std::ofstream tcpCwndOfStream;
  tcpCwndOfStream.open (cwndTraceFile.c_str (), std::ofstream::out);
  std::ofstream tcpThroughputOfStream;
  tcpThroughputOfStream.open (throughputTraceFile.c_str (), std::ofstream::out);
  std::ofstream queueDepthOfStream;
  queueDepthOfStream.open (queueDepthTraceFile.c_str (), std::ofstream::out);
  std::ofstream marksOfStream;
  marksOfStream.open (marksTraceFile.c_str (), std::ofstream::out);
  std::ofstream dropsOfStream;
  dropsOfStream.open (dropsTraceFile.c_str (), std::ofstream::out);
  std::ofstream txOfStream;
  txOfStream.open (txTraceFile.c_str (), std::ofstream::out);

  ////////////////////////////////////////////////////////////
  // scenario setup                                         //
  ////////////////////////////////////////////////////////////
  Ptr<Node> tcpClient = CreateObject<Node> ();
  Ptr<Node> router = CreateObject<Node> ();
  Ptr<Node> tcpServer = CreateObject<Node> ();

  // Device containers 
  NetDeviceContainer tcpClientDevices;
  NetDeviceContainer tcpServerDevices;

  PointToPointHelper p2p;
  // Set large queue size on DCE links because BQL is not in effect
  p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue (QueueSize ("1000p")));
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("1Gbps")));
  // Add delay only on the WAN links
  p2p.SetChannelAttribute ("Delay", TimeValue (oneWayDelay));
  tcpClientDevices = p2p.Install (router, tcpClient);
  p2p.SetChannelAttribute ("Delay", TimeValue (MicroSeconds (1)));
  tcpServerDevices = p2p.Install (router, tcpServer);

  // Limit the bandwidth on the router->tcpServer
  Ptr<PointToPointNetDevice> p = tcpServerDevices.Get (0)->GetObject<PointToPointNetDevice> ();
  p->SetAttribute ("DataRate", DataRateValue (bottleneckRate));

  DceManagerHelper dceManager;
  LinuxStackHelper linuxStack;
  dceManager.SetTaskManagerAttribute ("FiberManagerType", StringValue ("UcontextFiberManager"));
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));
  linuxStack.Install (tcpClient);
  linuxStack.Install (tcpServer);
  dceManager.Install (tcpClient);
  dceManager.Install (tcpServer);
  linuxStack.SysctlSet (tcpClient, ".net.ipv4.tcp_congestion_control", "dctcp");
  linuxStack.SysctlSet (tcpServer, ".net.ipv4.tcp_congestion_control", "dctcp");

  InternetStackHelper stackHelper;
  stackHelper.Install (router);

  TrafficControlHelper tch;
  tch.SetRootQueueDisc ("ns3::RedQueueDisc");
  tch.SetQueueLimits ("ns3::DynamicQueueLimits", "HoldTime", StringValue ("1ms"));
  tch.Install (tcpClientDevices.Get (0));
  tch.Install (tcpServerDevices.Get (0));

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer tcpClientIfaces = ipv4.Assign (tcpClientDevices);
  ipv4.SetBase ("192.168.2.0", "255.255.255.0");
  Ipv4InterfaceContainer tcpServerIfaces = ipv4.Assign (tcpServerDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  LinuxStackHelper::PopulateRoutingTables ();

  ////////////////////////////////////////////////////////////
  // application setup                                      //
  ////////////////////////////////////////////////////////////

  BulkSendHelper tcp ("ns3::LinuxTcpSocketFactory", Address ());
  // set to large value:  e.g. 1000 Mb/s for 60 seconds = 7500000000 bytes
  tcp.SetAttribute ("MaxBytes", UintegerValue (7500000000));
  uint16_t port = 5000;
  ApplicationContainer tcpApp;
  InetSocketAddress tcpDestAddress (tcpServerIfaces.GetAddress (1), port);
  tcp.SetAttribute ("Remote", AddressValue (tcpDestAddress));
  tcpApp = tcp.Install (tcpClient);
  tcpApp.Start (MilliSeconds (1));
  tcpApp.Stop (simulationStopTime - Seconds (1));

  Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper sinkHelper ("ns3::LinuxTcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp;
  sinkApp = sinkHelper.Install (tcpServer);
  sinkApp.Start (MilliSeconds (1));
  sinkApp.Stop (simulationStopTime - Seconds (0.5));

  // Setup traces that can be hooked now
  Ptr<TrafficControlLayer> tc;
  tc = router->GetObject<TrafficControlLayer> ();
  Ptr<RedQueueDisc> qd = tc->GetRootQueueDiscOnDevice (tcpServerDevices.Get (0))->GetObject<RedQueueDisc> ();
  qd->TraceConnectWithoutContext ("BytesInQueue", MakeBoundCallback (&TraceQueueDepth, &queueDepthOfStream, bottleneckRate));
  qd->TraceConnectWithoutContext ("Mark", MakeBoundCallback (&TraceMarks, &marksOfStream));
  qd->TraceConnectWithoutContext ("Drop", MakeBoundCallback (&TraceDrops, &dropsOfStream));

  Ptr<PointToPointNetDevice> ptx = tcpClientDevices.Get (1)->GetObject<PointToPointNetDevice> ();
  ptx->TraceConnectWithoutContext ("PhyTxBegin", MakeBoundCallback (&TraceTx, &txOfStream));

  // Setup scheduled traces
  Simulator::Schedule (MilliSeconds (1) + TimeStep (1), &ScheduleTcpRttTraceConnection, &tcpRttOfStream);
  Simulator::Schedule (MilliSeconds (1) + TimeStep (1), &ScheduleTcpCwndTraceConnection, &tcpCwndOfStream);
  Simulator::Schedule (MilliSeconds (1) + TimeStep (1), &SchedulePacketSinkConnection);
  Simulator::Schedule (throughputSamplingInterval, &TraceThroughput, &tcpThroughputOfStream, throughputSamplingInterval);

  if (enablePcap)
    {
      p2p.EnablePcapAll ("tcp-single-flow-single-bottleneck", false);
    }

  Simulator::Stop (simulationStopTime);
  Simulator::Run ();

  tcpRttOfStream.close ();
  tcpThroughputOfStream.close ();
  queueDepthOfStream.close ();
  marksOfStream.close ();
  dropsOfStream.close ();
  txOfStream.close ();
}

