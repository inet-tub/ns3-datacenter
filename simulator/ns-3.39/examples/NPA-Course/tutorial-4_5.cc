/*
 Vamsi
 Created: 14 Nov 17:22
*/

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <fstream>
#include <iomanip>
#include <map>
#include <ctime>
#include <set>
#include <unordered_map>

#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/ipv4-global-routing-helper.h"

# define PACKET_SIZE 1400
# define GIGA 1000000000

/*Congestion Control Algorithms*/
# define RENO 0
# define CUBIC 1
# define VEGAS 2
# define BBR 3


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NPA_TUTORIAL_4_5");

Ptr<OutputStreamWrapper> windowOutput[2];
AsciiTraceHelper asciiTraceHelper;

static void
CwndTrace(uint32_t nodeId, uint32_t oldcwnd, uint32_t newcwnd)
{
    *windowOutput[nodeId]->GetStream () << Simulator::Now().GetSeconds() << "," << newcwnd << std::endl;
}

void scheduleConnect(uint32_t node){
	Config::ConnectWithoutContext("/NodeList/" + std::to_string(node) +
                                      "/$ns3::TcpL4Protocol/SocketList/" +
                                      std::to_string(0) + "/CongestionWindow",
                                  MakeBoundCallback(&CwndTrace, node));
}

int
main (int argc, char *argv[])
{
	CommandLine cmd;

	double START_TIME = 0;
	double END_TIME = 20;

	uint64_t serverCapacity = 10*GIGA;
	uint64_t linkCapacity = 1*GIGA;
	uint32_t linkLatency = 1000; // in microseconds

	cmd.AddValue("linklatency","propagation delay on each link", linkLatency); // For example: ./waf --run "tutorial-4_5 --linklatency=1000"

	uint32_t TcpProt = RENO; // For example: ./waf --run "tutorial-4_5 --TcpProt=0" # here RENO = 0, CUBIC = 1, see at the top of this top.
	cmd.AddValue("TcpProt", "Tcp protocol", TcpProt);

	uint32_t initialCwnd = 2;
	cmd.AddValue("initialCwnd", "the initial congestion window used by TCP", initialCwnd);

	uint32_t BufferSize = 100 * PACKET_SIZE; // For example:./waf --run "tutorial-4_5 --TcpProt=0 --BufferSize=1000000"
	cmd.AddValue ("BufferSize", "BufferSize in Bytes", BufferSize);

	uint32_t rto = 100 * 1000; // in MicroSeconds, 10 milliseconds. // For example: ./waf --run "tutorial-4_5 --TcpProt=0 --BufferSize=1000000 --rto=10000"
	cmd.AddValue ("rto", "min Retransmission timeout value in MicroSeconds", rto);

	std::string scenario = "oneflow"; // DONT pass this argument for now. We only use oneflow scenario for tutorial 4_5
	cmd.AddValue("scenario", "oneflow or twoflow", scenario);

	std::string cwndOutput[2];

	/*Parse CMD*/
	cmd.Parse (argc, argv);


	uint32_t ssThresh = 5*linkCapacity*(6*linkLatency*1e-6)/8.0; // roughly 10 times BDP

	if (scenario != "oneflow" && scenario != "twoflow"){
		std::cout << "unknown scenario passed in the command line arguments. Aborting!";
		return 0;
	}
	else if (TcpProt != RENO && TcpProt != CUBIC && TcpProt != VEGAS && TcpProt != BBR){
		std::cout << "unknown TCP passed in the command line arguments. Aborting!";
		return 0;	
	}


	cwndOutput[0] = "./examples/NPA-Course/tutorial-4_cwnd_"+scenario+"_0.csv";
	cwndOutput[1] = "./examples/NPA-Course/tutorial-4_cwnd_"+scenario+"_1.csv";

	windowOutput[0] = asciiTraceHelper.CreateFileStream (cwndOutput[0]);
	windowOutput[1] = asciiTraceHelper.CreateFileStream (cwndOutput[1]);

	*windowOutput[0]->GetStream ()
		<< "time" << ","
		<< "cwnd" << std::endl;

	*windowOutput[1]->GetStream ()
		<< "time" << ","
		<< "cwnd" << std::endl;

	Config::SetDefault ("ns3::FifoQueueDisc::MaxSize", QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, BufferSize)));

	TrafficControlHelper tc;

	/*General TCP Socket settings. Mostly used by various congestion control algorithms in common*/
	Config::SetDefault ("ns3::TcpSocket::ConnTimeout", TimeValue (MilliSeconds (100))); // syn retry interval
	Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MicroSeconds (rto)) );  //(MilliSeconds (5))
	Config::SetDefault ("ns3::TcpSocketBase::ClockGranularity", TimeValue (NanoSeconds (10))); //(MicroSeconds (100))
	Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (ssThresh));
	Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1073725440)); //1073725440
	Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1073725440));
	Config::SetDefault ("ns3::TcpSocket::ConnCount", UintegerValue (6));  // Syn retry count
	Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue (true));
	Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (PACKET_SIZE));
	Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
	Config::SetDefault ("ns3::TcpSocket::PersistTimeout", TimeValue (Seconds (20)));
	Config::SetDefault ("ns3::TcpSocketState::EnablePacing", BooleanValue (true));


	Config::SetDefault ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
	tc.SetRootQueueDisc ("ns3::FifoQueueDisc");
	/*CC Configuration*/
	switch (TcpProt) {
	case RENO:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpNewReno::GetTypeId()));
		break;
	case CUBIC:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpCubic::GetTypeId()));
		break;
	case VEGAS:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpVegas::GetTypeId()));
		break;
	case BBR:
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpBbr::GetTypeId()));
		break;
	default:
		std::cout << "Error in CC configuration" << std::endl;
		return 0;
	}

	NodeContainer servers;
	servers.Create (4);
	NodeContainer routers;
	routers.Create (2);

	InternetStackHelper internet;
	Ipv4GlobalRoutingHelper globalRoutingHelper;
	internet.SetRoutingHelper (globalRoutingHelper);

	internet.Install(servers);
	internet.Install(routers);

	PointToPointHelper p2p;
	Ipv4AddressHelper ipv4;

	Time LINK_LATENCY = MicroSeconds(linkLatency);
	ipv4.SetBase ("10.1.0.0", "255.255.252.0");
	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (serverCapacity)));
	p2p.SetChannelAttribute ("Delay", TimeValue(LINK_LATENCY));
	p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("1p"));

	/* Node 0 to Router 0.*/
	ipv4.NewNetwork();
	NodeContainer nodeContainer = NodeContainer(servers.Get(0), routers.Get(0));
	NetDeviceContainer netDeviceContainer = p2p.Install(nodeContainer);
	QueueDiscContainer queueDiscs;
	queueDiscs = tc.Install(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(0));

	/* Node 1 to Router 0.*/
	ipv4.NewNetwork();
	nodeContainer = NodeContainer(servers.Get(1), routers.Get(0));
	netDeviceContainer = p2p.Install(nodeContainer);
	queueDiscs = tc.Install(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(0));

	/* Node 2 to Router 1.*/
	ipv4.NewNetwork();
	nodeContainer = NodeContainer(servers.Get(2), routers.Get(1));
	netDeviceContainer = p2p.Install(nodeContainer);
	queueDiscs = tc.Install(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(0));

	/* Node 3 to Router 1.*/
	ipv4.NewNetwork();
	nodeContainer = NodeContainer(servers.Get(3), routers.Get(1));
	netDeviceContainer = p2p.Install(nodeContainer);
	queueDiscs = tc.Install(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(0));

	/* Router 0 to Router 1.*/
	p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (linkCapacity)));
	ipv4.NewNetwork();
	nodeContainer = NodeContainer(routers.Get(0), routers.Get(1));
	netDeviceContainer = p2p.Install(nodeContainer);
	queueDiscs = tc.Install(netDeviceContainer);
	ipv4.Assign(netDeviceContainer.Get(1));
	ipv4.Assign(netDeviceContainer.Get(0));


	uint32_t port = 4444;
	uint64_t flowSize = 20*GIGA/8.0;
	double startTime = START_TIME+0.1;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Node 0 to Node 2 */
	Ptr<Node> rxNode = servers.Get (2);
	Ptr<Ipv4> ipv4Ptr = rxNode->GetObject<Ipv4> ();
	Ipv4InterfaceAddress rxInterface = ipv4Ptr->GetAddress (1, 0);
	Ipv4Address rxAddress = rxInterface.GetLocal ();

	InetSocketAddress ad (rxAddress, port);
	Address sinkAddress(ad);
	Ptr<BulkSendApplication> bulksend = CreateObject<BulkSendApplication>();
	bulksend->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
	bulksend->SetAttribute ("SendSize", UintegerValue (PACKET_SIZE));
	bulksend->SetAttribute ("MaxBytes", UintegerValue(flowSize));
	bulksend->SetAttribute("Remote", AddressValue(sinkAddress));
	bulksend->SetAttribute("InitialCwnd", UintegerValue (initialCwnd));
	bulksend->SetStartTime (Seconds(startTime));
	bulksend->SetStopTime (Seconds (END_TIME));
	servers.Get (0)->AddApplication(bulksend);

	Simulator::Schedule(Seconds(startTime)+NanoSeconds(10), scheduleConnect, 0);

	PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
	ApplicationContainer sinkApp = sink.Install (servers.Get(2));
	sinkApp.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
	sinkApp.Start (Seconds(startTime));
	sinkApp.Stop (Seconds (END_TIME));
	/////////////////////////////////////////////////////////////////////////////////////////////////
	/* Node 1 to Node 3 */
	if (scenario == "twoflow"){
		startTime = 2;
		flowSize = 2*GIGA/8.0;
		
		Ptr<Node> rxNode1 = servers.Get (3);
		Ptr<Ipv4> ipv4Ptr1 = rxNode1->GetObject<Ipv4> ();
		Ipv4InterfaceAddress rxInterface1 = ipv4Ptr1->GetAddress (1, 0);
		Ipv4Address rxAddress1 = rxInterface1.GetLocal ();

		InetSocketAddress ad1 (rxAddress1, port+1);
		Address sinkAddress1(ad1);
		Ptr<BulkSendApplication> bulksend1 = CreateObject<BulkSendApplication>();
		bulksend1->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
		bulksend1->SetAttribute ("SendSize", UintegerValue (PACKET_SIZE));
		bulksend1->SetAttribute ("MaxBytes", UintegerValue(flowSize));
		bulksend1->SetAttribute("Remote", AddressValue(sinkAddress1));
		bulksend1->SetAttribute("InitialCwnd", UintegerValue (initialCwnd));
		bulksend1->SetStartTime (Seconds(startTime));
		bulksend1->SetStopTime (Seconds (END_TIME));
		servers.Get (1)->AddApplication(bulksend1);

		Simulator::Schedule(Seconds(startTime)+NanoSeconds(10), scheduleConnect, 1);

		PacketSinkHelper sink1 ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port+1));
		ApplicationContainer sinkApp1 = sink1.Install (servers.Get(3));
		sinkApp1.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
		sinkApp1.Start (Seconds(startTime));
		sinkApp1.Stop (Seconds (END_TIME));
	}

// AsciiTraceHelper ascii;
//    p2p.EnableAsciiAll (ascii.CreateFileStream ("eval.tr"));
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	// NS_LOG_UNCOND("Running the Simulation...!");
	std::cout << "Running the Simulation...!" << std::endl;
	Simulator::Stop (Seconds (END_TIME));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
