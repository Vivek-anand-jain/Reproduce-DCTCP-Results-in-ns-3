/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Apoorva Bhargava <apoorvabhargava13@gmail.com>
 *          Shikha Bakshi <shikhabakshi912@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

// The network topology used in this example is based on the Fig. 17 described in
// Mohammad Alizadeh, Albert Greenberg, David A. Maltz, Jitendra Padhye,
// Parveen Patel, Balaji Prabhakar, Sudipta Sengupta, and Murari Sridharan.
// "Data Center TCP (DCTCP)." In ACM SIGCOMM Computer Communication Review,
// Vol. 40, No. 4, pp. 63-74. ACM, 2010.

#include <iostream>
#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/dce-module.h"

using namespace ns3;

std::vector<std::stringstream> filePlotQueue;
Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
std::string dir = "results/gfc-dctcp/";
double stopTime = 10;
NodeContainer senderNodes;

// Checks the queue size for router T1 after every 0.001 seconds
void
CheckQueueSize (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();

  // Check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueueSize, queue);

  std::ofstream fPlotQueue (std::stringstream (dir + "queue-0.plotme").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

// Checks queue size for router Scorpion after every 0.001 seconds
void
CheckQueueSize1 (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();

  // Check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueueSize1, queue);

  std::ofstream fPlotQueue (std::stringstream (dir + "queue-1.plotme").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

// Checks queue size for router T2 after every 0.001 seconds
void
CheckQueueSize2 (Ptr<QueueDisc> queue)
{
  uint32_t qSize = queue->GetCurrentSize ().GetValue ();

  // Check queue size every 1/100 of a second
  Simulator::Schedule (Seconds (0.001), &CheckQueueSize2, queue);

  std::ofstream fPlotQueue (std::stringstream (dir + "queue-2.plotme").str ().c_str (), std::ios::out | std::ios::app);
  fPlotQueue << Simulator::Now ().GetSeconds () << " " << qSize << std::endl;
  fPlotQueue.close ();
}

// Function to trace change in cwnd for all the sender with ns-3 stack
static void
CwndChange (Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " " << newCwnd / 1446.0 << std::endl;
}

// Function to calculate drops in a particular Queue
static void
DropAtQueue (Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " 1" << std::endl;
}

// Function to calculate marks in a particular Queue
static void
MarkAtQueue (Ptr<OutputStreamWrapper> stream, Ptr<const QueueDiscItem> item, const char* reason)
{
  *stream->GetStream () << Simulator::Now ().GetSeconds () << " 1" << std::endl;
}

// Trace Function for cwnd
void
TraceCwnd (uint32_t node, uint32_t cwndWindow,
           Callback <void, uint32_t, uint32_t> CwndTrace)
{
  Config::ConnectWithoutContext ("/NodeList/" + std::to_string (node) + "/$ns3::TcpL4Protocol/SocketList/" + std::to_string (cwndWindow) + "/CongestionWindow", CwndTrace);
}

// Install Sink application on a particular node
void InstallPacketSink (Ptr<Node> node, uint16_t port, std::string sock_factory, std::string stack)
{
  PacketSinkHelper sink (sock_factory,
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (node);
  if (stack == "linux")
    {
      sinkApps.Start (Seconds (10.0));
    }
  else if (stack == "ns3")
    {
      sinkApps.Start (Seconds (0.0));
    }
  sinkApps.Stop (Seconds (stopTime));
}

// Install Bulk Send Application on the particular node and starts the application randomly between 10 to 11 seconds for Linux stack
void InstallBulkSend (Ptr<Node> node, Ipv4Address address, uint16_t port, std::string sock_factory)
{
  BulkSendHelper source (sock_factory,
                         InetSocketAddress (address, port));

  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (node);
  Time timeToStart = Seconds (uv->GetValue (10, 11));
  sourceApps.Start (timeToStart);
  sourceApps.Stop (Seconds (stopTime));
}

// Install Bulk Send Application on the particular node and starts the application randomly between 0 to 1 seconds for ns-3 stack
void InstallBulkSend (Ptr<Node> node, Ipv4Address address, uint16_t port, std::string sock_factory,
                      uint32_t nodeId, uint32_t cwndWindow,
                      Callback <void, uint32_t, uint32_t> CwndTrace)
{

  BulkSendHelper source (sock_factory,
                         InetSocketAddress (address, port));

  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (node);
  Time timeToStart = Seconds (uv->GetValue (0, 1));
  sourceApps.Start (timeToStart);
  Simulator::Schedule (timeToStart + Seconds (0.001), &TraceCwnd, nodeId, cwndWindow, CwndTrace);
  sourceApps.Stop (Seconds (stopTime));
}

// Runs "ss" command on a particular node with Linux stack to get the socket stats of that node.
static void GetSSStats (Ptr<Node> node, Time at)
{
  DceApplicationHelper process;
  ApplicationContainer apps;
  process.SetBinary ("ss");
  process.SetStackSize (1 << 20);
  process.AddArgument ("-a");
  process.AddArgument ("-e");
  process.AddArgument ("-i");
  apps.Add (process.Install (node));
  apps.Start (at);
}

int main (int argc, char *argv[])
{
  uint32_t stream = 1;
  std::string stack = "linux";
  std::string transport_prot = "TcpReno";
  std::string linux_prot = "reno";
  std::string queue_disc_type = "FifoQueueDisc";
  std::string sock_factory = "ns3::TcpSocketFactory";
  bool useEcn = true;
  uint32_t dataSize = 1446;
  uint32_t delAckCount = 2;

  // Enable checksum for comunication between node with linux stack and ns3 stack
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,sizeof(buffer),"%d-%m-%Y-%I-%M-%S",timeinfo);
  std::string currentTime (buffer);

  CommandLine cmd;
  cmd.AddValue ("stream", "Seed value for random variable", stream);
  cmd.AddValue ("stack", "Set TCP/IP stack as ns3 or linux", stack);
  cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat, "
                "TcpLp", transport_prot);
  cmd.AddValue ("linux_prot", "Linux network protocol to use: reno, "
                "hybla, highspeed, htcp, vegas, scalable, veno, "
                "bic, yeah, illinois, westwood, lp", linux_prot);
  cmd.AddValue ("queue_disc_type", "Queue disc type for gateway (e.g. ns3::CoDelQueueDisc)", queue_disc_type);
  cmd.AddValue ("useEcn", "Use ECN", useEcn);
  cmd.AddValue ("dataSize", "Data packet size", dataSize);
  cmd.AddValue ("delAckCount", "Delayed ack count", delAckCount);
  cmd.AddValue ("stopTime", "Stop time for applications / simulation time will be stopTime", stopTime);
  cmd.Parse (argc,argv);

  uv->SetStream (stream);
  queue_disc_type = std::string ("ns3::") + queue_disc_type;

  if (stack == "ns3")
    {
      transport_prot = std::string ("ns3::") + transport_prot;
    }

  TypeId qdTid;
  NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (queue_disc_type, &qdTid), "TypeId " << queue_disc_type << " not found");

  // Enables PRR and sets TCP variant in ns-3
  if (stack == "ns3")
    {
      Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TypeId::LookupByName ("ns3::TcpPrrRecovery")));
      if (transport_prot.compare ("ns3::TcpWestwoodPlus") == 0)
        {
          // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
          Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
          // the default protocol type in ns3::TcpWestwood is WESTWOOD
          Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
        }
      else
        {
          TypeId tcpTid;
          NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
          Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));
        }
    }

  NodeContainer S1, S2, S3, R1, R2, T, Scorp;
  // Create router nodes
  T.Create (2);
  Scorp.Create (1);

  // Create sender nodes
  S1.Create (10);
  S2.Create (20);
  S3.Create (10);

  // Create receiver nodes
  R2.Create (20);
  R1.Create (1);

  senderNodes.Add (S1);
  senderNodes.Add (S2);
  senderNodes.Add (S3);

  // Create a point-to-point channel for senders and receiver to router and configure its attributes
  PointToPointHelper pointToPointSR;
  pointToPointSR.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  pointToPointSR.SetChannelAttribute ("Delay", StringValue ("0.05ms"));

  // Create a point-to-point channel for routers and configure its attributes
  PointToPointHelper pointToPointT;
  pointToPointT.SetDeviceAttribute ("DataRate", StringValue ("10000Mbps"));
  pointToPointT.SetChannelAttribute ("Delay", StringValue ("0.05ms"));

  PointToPointHelper pointToPointT1;
  pointToPointT1.SetDeviceAttribute ("DataRate", StringValue ("10000Mbps"));
  pointToPointT1.SetChannelAttribute ("Delay", StringValue ("0.05ms"));

  // Installing point-to-point channel between router T1 and Scorp and router T2 and Scorp
  NetDeviceContainer T1ScorpDev, T2ScorpDev, S1T1Dev, S2T1Dev, S3T2Dev, R1T2Dev, R2T2Dev;
  T1ScorpDev = pointToPointT.Install (T.Get (0), Scorp.Get (0));
  T2ScorpDev = pointToPointT1.Install (Scorp.Get (0), T.Get (1));

  // Installing point-to-point channel between receiver R1 and router T2
  R1T2Dev = pointToPointSR.Install (R1.Get (0), T.Get (1));

  // Installing point-to-point channel between S1 senders and T1 and S3 senders and T2
  for (uint32_t i = 0; i < S1.GetN (); i++)
    {
      S1T1Dev.Add (pointToPointSR.Install (S1.Get (i), T.Get (0)));
      S3T2Dev.Add (pointToPointSR.Install (S3.Get (i), T.Get (1)));
    }

  // Installing point-to-point channel between S2 sender and router T1
  for (uint32_t i = 0; i < S2.GetN (); i++)
    {
      S2T1Dev.Add (pointToPointSR.Install (S2.Get (i), T.Get (0)));
    }

  // Installing point-to-point channel between R2 receiver and router T2
  for (uint32_t i = 0; i < R2.GetN (); i++)
    {
      R2T2Dev.Add (pointToPointSR.Install (R2.Get (i), T.Get (1)));
    }

  DceManagerHelper dceManager;
  LinuxStackHelper linuxStack;
  InternetStackHelper internetStack;

  if (stack == "linux")
    {
      sock_factory = "ns3::LinuxTcpSocketFactory";
      dceManager.SetTaskManagerAttribute ("FiberManagerType", StringValue ("UcontextFiberManager"));
      dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory", "Library", StringValue ("liblinux.so"));

      // Install Linux stack on all the senders and receivers and ns-3 stack on all the routers
      linuxStack.Install (S1);
      linuxStack.Install (S2);
      linuxStack.Install (S3);
      linuxStack.Install (R1);
      linuxStack.Install (R2);
      internetStack.Install (T);
      internetStack.Install (Scorp);
    }
  else if (stack == "ns3")
    {
      // Install ns-3 stack on all the nodes
      internetStack.InstallAll ();
    }

  // Assign IP address to all the net devices
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer S1Int, S2Int, S3Int, R1Int, R2Int, T1Int,T2Int;

  address.SetBase ("10.1.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < S1T1Dev.GetN (); i = i + 2)
    {
      address.NewNetwork ();
      NetDeviceContainer S1iT1i;
      S1iT1i.Add (S1T1Dev.Get (i));
      S1iT1i.Add (S1T1Dev.Get (i + 1));
      S1Int.Add (address.Assign (S1iT1i));
    }
  address.SetBase ("10.2.0.0", "255.255.255.0");


  for (uint32_t i = 0; i < S2T1Dev.GetN (); i = i + 2)
    {
      address.NewNetwork ();
      NetDeviceContainer S2iT1i;
      S2iT1i.Add (S2T1Dev.Get (i));
      S2iT1i.Add (S2T1Dev.Get (i + 1));
      S2Int.Add (address.Assign (S2iT1i));
    }

  address.SetBase ("10.3.0.0", "255.255.255.0");

  for (uint32_t i = 0; i < S3T2Dev.GetN (); i = i + 2)
    {
      address.NewNetwork ();
      NetDeviceContainer S3iT2i;
      S3iT2i.Add (S3T2Dev.Get (i));
      S3iT2i.Add (S3T2Dev.Get (i + 1));
      S3Int.Add (address.Assign (S3iT2i));
    }

  address.SetBase ("10.4.0.0", "255.255.255.0");
  R1Int = address.Assign (R1T2Dev);
  address.SetBase ("10.5.0.0", "255.255.255.0");


  for (uint32_t i = 0; i < R2T2Dev.GetN (); i = i + 2)
    {
      address.NewNetwork ();
      NetDeviceContainer R2iT2i;
      R2iT2i.Add (R2T2Dev.Get (i));
      R2iT2i.Add (R2T2Dev.Get (i + 1));
      R2Int.Add (address.Assign (R2iT2i));
    }

  address.SetBase ("10.6.0.0", "255.255.255.0");
  T1Int = address.Assign (T1ScorpDev);

  address.SetBase ("10.7.0.0", "255.255.255.0");
  T2Int = address.Assign (T2ScorpDev);

  // Install DCE Manager on all the nodes
  dceManager.Install (S1);
  dceManager.Install (S2);
  dceManager.Install (S3);
  dceManager.Install (R1);
  dceManager.Install (R2);
  dceManager.Install (T);
  dceManager.Install (Scorp);

  if (stack == "linux")
    {
      // Activate IP forwarding in linux stack of all senders and receivers
      linuxStack.SysctlSet (S1, ".net.ipv4.conf.default.forwarding", "1");
      linuxStack.SysctlSet (S2, ".net.ipv4.conf.default.forwarding", "1");
      linuxStack.SysctlSet (S3, ".net.ipv4.conf.default.forwarding", "1");
      linuxStack.SysctlSet (R1, ".net.ipv4.conf.default.forwarding", "1");
      linuxStack.SysctlSet (R2, ".net.ipv4.conf.default.forwarding", "1");

      // Set TCP congestion control algorithm in linux stack of all senders and receivers
      linuxStack.SysctlSet (S1, ".net.ipv4.tcp_congestion_control", linux_prot);
      linuxStack.SysctlSet (S2, ".net.ipv4.tcp_congestion_control", linux_prot);
      linuxStack.SysctlSet (S3, ".net.ipv4.tcp_congestion_control", linux_prot);
      linuxStack.SysctlSet (R1, ".net.ipv4.tcp_congestion_control", linux_prot);
      linuxStack.SysctlSet (R2, ".net.ipv4.tcp_congestion_control", linux_prot);

      // Eanble ECN in linux stack of all senders and receivers
      linuxStack.SysctlSet (S1, ".net.ipv4.tcp_ecn", "1");
      linuxStack.SysctlSet (S2, ".net.ipv4.tcp_ecn", "1");
      linuxStack.SysctlSet (S3, ".net.ipv4.tcp_ecn", "1");
      linuxStack.SysctlSet (R1, ".net.ipv4.tcp_ecn", "1");
      linuxStack.SysctlSet (R2, ".net.ipv4.tcp_ecn", "1");

      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      LinuxStackHelper::PopulateRoutingTables ();
    }
  else if (stack == "ns3")
    {
      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

      // Sets default sender and receiver buffer size as 1MB
      Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 20));
      Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 20));
      // Sets default initial congestion window as 10 segments
      Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
      // Sets default delayed ack count to a specified value
      Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (delAckCount));
      // Sets default segment size of TCP packet to a specified value
      Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (dataSize));
      // Sets ECN mode to Classic ECN if useEcn is true
      if (useEcn)
        {
          Config::SetDefault ("ns3::TcpSocketBase::UseEcn", StringValue ("On"));
        }
    }

  // Creates directories to store plotme files
  dir += (currentTime + "/");
  std::string dirToSave = "mkdir -p " + dir;
  system (dirToSave.c_str ());
  system ((dirToSave + "/pcap/").c_str ());
  system ((dirToSave + "/markTraces/").c_str ());
  system ((dirToSave + "/queueTraces/").c_str ());
  if (stack == "ns3")
    {
      system ((dirToSave + "/cwndTraces/").c_str ());
      system (("cp -R PlotScripts-gfc-dctcp/ns3-* " + dir + "/pcap/").c_str ());
    }
  else if (stack == "linux")
    {
      system (("cp -R PlotScripts-gfc-dctcp/linux-* " + dir + "/pcap/").c_str ());
    }


  // Set default parameters for RED queue disc
  Config::SetDefault ("ns3::RedQueueDisc::UseEcn", BooleanValue (useEcn));
  Config::SetDefault ("ns3::RedQueueDisc::ARED", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::Gentle", BooleanValue (true));
  Config::SetDefault ("ns3::RedQueueDisc::UseHardDrop", BooleanValue (false));
  Config::SetDefault ("ns3::RedQueueDisc::MeanPktSize", UintegerValue (1500));
  Config::SetDefault ("ns3::RedQueueDisc::MinTh", DoubleValue (65));
  Config::SetDefault ("ns3::RedQueueDisc::MaxTh", DoubleValue (65));
  Config::SetDefault ("ns3::RedQueueDisc::QW", DoubleValue (1));
  Config::SetDefault (queue_disc_type + "::MaxSize", QueueSizeValue (QueueSize ("666p")));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> streamWrapper;

  TrafficControlHelper tch;
  tch.SetRootQueueDisc (queue_disc_type);

  QueueDiscContainer qd, qd1, qd2;

  // Install RED queue disc on outgoing link of T1 and get drop and mark packets stats
  tch.Uninstall (T1ScorpDev);
  qd = tch.Install (T1ScorpDev);
  Simulator::ScheduleNow (&CheckQueueSize, qd.Get (0));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/drop-0.plotme");
  qd.Get (0)->TraceConnectWithoutContext ("Drop", MakeBoundCallback (&DropAtQueue, streamWrapper));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/mark-0.plotme");
  qd.Get (0)->TraceConnectWithoutContext ("Mark", MakeBoundCallback (&MarkAtQueue, streamWrapper));

  // Install RED queue disc on outgoing link of Scorp and get drop and mark packets stats
  tch.Uninstall (T2ScorpDev);
  qd1 = tch.Install (T2ScorpDev);
  Simulator::ScheduleNow (&CheckQueueSize1, qd1.Get (0));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/drop-1.plotme");
  qd1.Get (0)->TraceConnectWithoutContext ("Drop", MakeBoundCallback (&DropAtQueue, streamWrapper));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/mark-1.plotme");
  qd1.Get (0)->TraceConnectWithoutContext ("Mark", MakeBoundCallback (&MarkAtQueue, streamWrapper));

  // Install RED queue disc on outgoing link of T2 and get drop and mark packets stats
  tch.Uninstall (R1T2Dev.Get (1));
  qd2 = tch.Install (R1T2Dev.Get (1));
  Simulator::ScheduleNow (&CheckQueueSize2, qd2.Get (0));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/drop-2.plotme");
  qd2.Get (0)->TraceConnectWithoutContext ("Drop", MakeBoundCallback (&DropAtQueue, streamWrapper));
  streamWrapper = asciiTraceHelper.CreateFileStream (dir + "/queueTraces/mark-2.plotme");
  qd2.Get (0)->TraceConnectWithoutContext ("Mark", MakeBoundCallback (&MarkAtQueue, streamWrapper));

  // Set queue size for outgoing link of router T1
  Config::Set ("/$ns3::NodeListPriv/NodeList/0/$ns3::Node/$ns3::TrafficControlLayer/RootQueueDiscList/0/$" + queue_disc_type + "/MaxSize", QueueSizeValue (QueueSize ("666p")));
  // Set queue size for outgoing link of router T2
  Config::Set ("/$ns3::NodeListPriv/NodeList/1/$ns3::Node/$ns3::TrafficControlLayer/RootQueueDiscList/1/$" + queue_disc_type + "/MaxSize", QueueSizeValue (QueueSize ("666p")));
  // Set queue size for outgoing link of router Scorp
  Config::Set ("/$ns3::NodeListPriv/NodeList/2/$ns3::Node/$ns3::TrafficControlLayer/RootQueueDiscList/1/$" + queue_disc_type + "/MaxSize", QueueSizeValue (QueueSize ("666p")));

  uint16_t port = 50000;
  // Install sink applications on R1
  for (int i = 0; i < S1.GetN () + S3.GetN (); i++)
    {
      InstallPacketSink (R1.Get (0), port + i, sock_factory, stack);
    }

  // Install Sink applications on R2
  for (uint32_t i = 0; i < R2.GetN (); i++)
    {
      InstallPacketSink (R2.Get (i), port, sock_factory, stack);
    }

  if (stack == "linux")
    {
      // Install BulkSend application on S1
      for (int i = 0; i < S1.GetN (); i++)
        {
          InstallBulkSend (S1.Get (i), R1Int.GetAddress (0), port + i, sock_factory);
        }

      // Install BulkSend application on S3
      for (int i = 0; i < S3.GetN (); i++)
        {
          InstallBulkSend (S3.Get (i), R1Int.GetAddress (0), port + i + 10, sock_factory);
        }

      // Install BulkSend application on S2
      for (int i = 0; i < S2.GetN (); i++)
        {
          InstallBulkSend (S2.Get (i), R2Int.GetAddress (i * 2), port, sock_factory);
        }
    }
  else if (stack == "ns3")
    {
      // Install BulkSend applications on S1 and S3
      for (int i = 0; i < S1.GetN (); i++)
        {
          Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + "cwndTraces/S1" + std::to_string (i) + "-ns3.plotme");
          InstallBulkSend (S1.Get (i), R1Int.GetAddress (0), port + i, sock_factory, 3 + i, 0, MakeBoundCallback (&CwndChange, stream));
        }

      for (int i = 0; i < S3.GetN (); i++)
        {
          Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + "cwndTraces/S3" + std::to_string (i) + "-ns3.plotme");
          InstallBulkSend (S3.Get (i), R1Int.GetAddress (0), port + 10 + i, sock_factory, 33 + i, 0, MakeBoundCallback (&CwndChange, stream));
        }

      // Install BulkSend applications on S2
      for (int i = 0; i < S2.GetN (); i++)
        {
          Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (dir + "cwndTraces/S2" + std::to_string (i) + "-ns3.plotme");
          InstallBulkSend (S2.Get (i), R2Int.GetAddress (2 * i), port, sock_factory, 13 + i, 0, MakeBoundCallback (&CwndChange, stream));
        }
    }

  pointToPointSR.EnablePcapAll (dir + "pcap/N", true);

  // Calls GetSSStats function on sender nodes after every 0.1 seconds
  if (stack == "linux")
    {
      for (int j = 0; j < 40; j++)
        {
          for (float i = 10.0; i < stopTime; i = i + 0.1)
            {
              GetSSStats (senderNodes.Get (j), Seconds (i));
            }
        }
    }

  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();

  // Store queue statistics in queueStats.txt file
  std::ofstream myfile;
  myfile.open (dir + "queueStats.txt", std::fstream::in | std::fstream::out | std::fstream::app);
  myfile << std::endl;
  myfile << "Stat for Queue 0";
  myfile << qd.Get (0)->GetStats ();
  myfile << "Stat for Queue 1";
  myfile << qd1.Get (0)->GetStats ();
  myfile << "Stat for Queue 2";
  myfile << qd2.Get (0)->GetStats ();
  myfile.close ();

  // Store configuration of the simulator in config.txt file
  myfile.open (dir + "config.txt", std::fstream::in | std::fstream::out | std::fstream::app);
  myfile << "useEcn " << useEcn << "\n";
  myfile << "queue_disc_type " << queue_disc_type << "\n";
  myfile << "stream  " << stream << "\n";
  myfile << "transport_prot " << transport_prot << "\n";
  myfile << "stopTime " << stopTime << "\n";
  myfile.close ();

  Simulator::Destroy ();
  return 0;
}
