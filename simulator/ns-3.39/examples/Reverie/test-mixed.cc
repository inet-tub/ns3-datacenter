/*
 * test-mixed.cc
 *
 *  Created on: May 25, 2022
 *      Author: vamsi
 */
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
*/

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include "ns3/core-module.h"
#include "ns3/qbb-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/packet.h"
#include "ns3/error-model.h"
#include <ns3/rdma.h>
#include <ns3/rdma-client.h>
#include <ns3/rdma-client-helper.h>
#include <ns3/rdma-driver.h>
#include <ns3/switch-node.h>
#include <ns3/sim-setting.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <ctime>
#include <set>
#include <string>
#include <unordered_map>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

using namespace ns3;
using namespace std;

#define LOSSLESS 0
#define LOSSY 1
#define DUMMY 2

# define DT 101
# define FAB 102
# define CS 103
# define IB 104
# define ABM 110

NS_LOG_COMPONENT_DEFINE("GENERIC_SIMULATION");

extern "C"
{
#include "cdf.h"
}
#define LINK_CAPACITY_BASE    1000000000          // 1Gbps

std::string data_rate, link_delay, topology_file, flow_file;

Ptr<OutputStreamWrapper> fctOutput;
AsciiTraceHelper asciiTraceHelper;

Ptr<OutputStreamWrapper> torStats;
AsciiTraceHelper torTraceHelper;

uint32_t cc_mode = 1;
bool enable_qcn = true;
uint32_t packet_payload_size = 1000, l2_chunk_size = 0, l2_ack_interval = 0;
double pause_time = 5, simulator_stop_time = 3.01;

double alpha_resume_interval = 55, rp_timer, ewma_gain = 1 / 16;
double rate_decrease_interval = 4;
uint32_t fast_recovery_times = 5;
std::string rate_ai, rate_hai, min_rate = "100Mb/s";
std::string dctcp_rate_ai = "1000Mb/s";
bool clamp_target_rate = false, l2_back_to_zero = false;
double error_rate_per_link = 0.0;
uint32_t has_win = 1;
uint32_t global_t = 1;
uint32_t mi_thresh = 5;
bool var_win = false, fast_react = true;
bool multi_rate = true;
bool sample_feedback = false;
double pint_log_base = 1.05;
double pint_prob = 1.0;
double u_target = 0.95;
uint32_t int_multi = 1;
bool rate_bound = true;

uint32_t ack_high_prio = 0;

uint32_t buffer_size = 16;

uint32_t qlen_dump_interval = 100000000, qlen_mon_interval = 100;
uint64_t qlen_mon_start = 2000000000, qlen_mon_end = 2100000000;
string qlen_mon_file;

unordered_map<uint64_t, uint32_t> rate2kmax, rate2kmin;
unordered_map<uint64_t, double> rate2pmax;

double alpha_values[8]={1,1,1,1,1,1,1,1};

/************************************************
 * Runtime varibles
 ***********************************************/
std::ifstream topof, flowf, tracef;

NodeContainer n;
NetDeviceContainer switchToSwitchInterfaces;
std::map< uint32_t, std::map< uint32_t, std::vector<Ptr<QbbNetDevice>> > > switchToSwitch;

// vamsi
std::map<uint32_t, uint32_t> switchNumToId;
std::map<uint32_t, uint32_t> switchIdToNum;
std::map<uint32_t, NetDeviceContainer> switchUp;
std::map<uint32_t, NetDeviceContainer> switchDown;
//NetDeviceContainer switchUp[switch_num];
std::map<uint32_t, NetDeviceContainer> sourceNodes;

NodeContainer servers;
NodeContainer tors;

uint64_t nic_rate;

uint64_t maxRtt, maxBdp;

struct Interface {
    uint32_t idx;
    bool up;
    uint64_t delay;
    uint64_t bw;

    Interface() : idx(0), up(false) {}
};
map<Ptr<Node>, map<Ptr<Node>, Interface> > nbr2if;
// Mapping destination to next hop for each node: <node, <dest, <nexthop0, ...> > >
map<Ptr<Node>, map<Ptr<Node>, vector<Ptr<Node> > > > nextHop;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairDelay;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairTxDelay;
map<uint32_t, map<uint32_t, uint64_t> > pairBw;
map<Ptr<Node>, map<Ptr<Node>, uint64_t> > pairBdp;
map<uint32_t, map<uint32_t, uint64_t> > pairRtt;

std::vector<Ipv4Address> serverAddress;

// maintain port number for each host pair
std::unordered_map<uint32_t, unordered_map<uint32_t, uint16_t> > portNumder;
std::unordered_map<uint32_t, unordered_map<uint32_t, uint16_t> > DestportNumder;

struct FlowInput {
    uint64_t src, dst, pg, maxPacketCount, port, dport;
    double start_time;
    uint32_t idx;
};
FlowInput flow_input = {0};
uint32_t flow_num;

void ReadFlowInput() {
    if (flow_input.idx < flow_num) {
        flowf >> flow_input.src >> flow_input.dst >> flow_input.pg >> flow_input.dport >> flow_input.maxPacketCount >> flow_input.start_time;
        NS_ASSERT(n.Get(flow_input.src)->GetNodeType() == 0 && n.Get(flow_input.dst)->GetNodeType() == 0);
    }
}
void ScheduleFlowInputs() {
    while (flow_input.idx < flow_num && Seconds(flow_input.start_time) <= Simulator::Now()) {
        // std::cout << "Flow " << flow_input.src << " " << flow_input.dst << " " << flow_input.pg << " " << flow_input.dport << " " << flow_input.maxPacketCount << " " << flow_input.start_time << " " << Simulator::Now().GetSeconds() << std::endl;
        uint32_t port = portNumder[flow_input.src][flow_input.dst]++; // get a new port number
        RdmaClientHelper clientHelper(flow_input.pg, serverAddress[flow_input.src], serverAddress[flow_input.dst], port, flow_input.dport, flow_input.maxPacketCount, has_win ? (global_t == 1 ? maxBdp : pairBdp[n.Get(flow_input.src)][n.Get(flow_input.dst)]) : 0, global_t == 1 ? maxRtt : pairRtt[flow_input.src][flow_input.dst], Simulator::GetMaximumSimulationTime());
        ApplicationContainer appCon = clientHelper.Install(n.Get(flow_input.src));
//      appCon.Start(Seconds(flow_input.start_time));
        appCon.Start(Seconds(0)); // setting the correct time here conflicts with Sim time since there is already a schedule event that triggered this function at desired time.
        // get the next flow input
        flow_input.idx++;
        ReadFlowInput();
    }

    // schedule the next time to run this function
    if (flow_input.idx < flow_num) {
        Simulator::Schedule(Seconds(flow_input.start_time) - Simulator::Now(), ScheduleFlowInputs);
    } else { // no more flows, close the file
        flowf.close();
    }
}

Ipv4Address node_id_to_ip(uint32_t id) {
    return Ipv4Address(0x0b000001 + ((id / 256) * 0x00010000) + ((id % 256) * 0x00000100));
}

uint32_t ip_to_node_id(Ipv4Address ip) {
    return (ip.Get() >> 8) & 0xffff;
}


void TraceMsgFinish (Ptr<OutputStreamWrapper> stream, double size, double start, bool incast, uint32_t prior )
{
    double fct, standalone_fct, slowdown;
    fct = Simulator::Now().GetNanoSeconds() - start;
    standalone_fct = maxRtt + size * 8.0 / nic_rate;
    slowdown = fct / standalone_fct;

    *stream->GetStream ()
            << Simulator::Now().GetSeconds()
            << " " << size
            << " " << fct
            << " " << standalone_fct
            << " " << slowdown
            << " " << maxRtt / 1e3
            << " " << (start / 1e3 - Seconds(10).GetMicroSeconds())
            << " " << prior
            << " " << incast
            << std::endl;
}

void qp_finish(Ptr<OutputStreamWrapper> fout, Ptr<RdmaQueuePair> q) {
    uint32_t sid = ip_to_node_id(q->sip), did = ip_to_node_id(q->dip);
    uint64_t base_rtt = pairRtt[sid][did], b = pairBw[sid][did];
    uint32_t total_bytes = q->m_size + ((q->m_size - 1) / packet_payload_size + 1) * (CustomHeader::GetStaticWholeHeaderSize() - IntHeader::GetStaticSize()); // translate to the minimum bytes required (with header but no INT)
    uint64_t standalone_fct = base_rtt + total_bytes * 8 * 1e9 / b;
    *fout->GetStream () << "FCT " << (Simulator::Now() - q->startTime).GetNanoSeconds() << " size " << q->m_size << " baseFCT " << standalone_fct << std::endl;

    // remove rxQp from the receiver
    Ptr<Node> dstNode = n.Get(did);
    Ptr<RdmaDriver> rdma = dstNode->GetObject<RdmaDriver> ();
    rdma->m_rdma->DeleteRxQp(q->sip.Get(), q->m_pg, q->sport);
}


void get_pfc(FILE* fout, Ptr<QbbNetDevice> dev, uint32_t type) {
    fprintf(fout, "%lu %u %u %u %u\n", Simulator::Now().GetTimeStep(), dev->GetNode()->GetId(), dev->GetNode()->GetNodeType(), dev->GetIfIndex(), type);
}

void CalculateRoute(Ptr<Node> host) {
    // queue for the BFS.
    vector<Ptr<Node> > q;
    // Distance from the host to each node.
    map<Ptr<Node>, int> dis;
    map<Ptr<Node>, uint64_t> delay;
    map<Ptr<Node>, uint64_t> txDelay;
    map<Ptr<Node>, uint64_t> bw;
    // init BFS.
    q.push_back(host);
    dis[host] = 0;
    delay[host] = 0;
    txDelay[host] = 0;
    bw[host] = 0xfffffffffffffffflu;
    // BFS.
    for (int i = 0; i < (int)q.size(); i++) {
        Ptr<Node> now = q[i];
        int d = dis[now];
        for (auto it = nbr2if[now].begin(); it != nbr2if[now].end(); it++) {
            // skip down link
            if (!it->second.up)
                continue;
            Ptr<Node> next = it->first;
            // If 'next' have not been visited.
            if (dis.find(next) == dis.end()) {
                dis[next] = d + 1;
                delay[next] = delay[now] + it->second.delay;
                txDelay[next] = txDelay[now] + packet_payload_size * 1000000000lu * 8 / it->second.bw;
                bw[next] = std::min(bw[now], it->second.bw);
                // we only enqueue switch, because we do not want packets to go through host as middle point
                if (next->GetNodeType() == 1)
                    q.push_back(next);
            }
            // if 'now' is on the shortest path from 'next' to 'host'.
            if (d + 1 == dis[next]) {
                nextHop[next][host].push_back(now);
            }
        }
    }
    for (auto it : delay)
        pairDelay[it.first][host] = it.second;
    for (auto it : txDelay)
        pairTxDelay[it.first][host] = it.second;
    for (auto it : bw)
        pairBw[it.first->GetId()][host->GetId()] = it.second;
}

void CalculateRoutes(NodeContainer &n) {
    for (int i = 0; i < (int)n.GetN(); i++) {
        Ptr<Node> node = n.Get(i);
        if (node->GetNodeType() == 0)
            CalculateRoute(node);
    }
}

void SetRoutingEntries() {
    // For each node.
    for (auto i = nextHop.begin(); i != nextHop.end(); i++) {
        Ptr<Node> node = i->first;
        auto &table = i->second;
        for (auto j = table.begin(); j != table.end(); j++) {
            // The destination node.
            Ptr<Node> dst = j->first;
            // The IP address of the dst.
            Ipv4Address dstAddr = dst->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal();
            // The next hops towards the dst.
            vector<Ptr<Node> > nexts = j->second;
            for (int k = 0; k < (int)nexts.size(); k++) {
                Ptr<Node> next = nexts[k];
                uint32_t interface = nbr2if[node][next].idx;
                if (node->GetNodeType() == 1)
                    DynamicCast<SwitchNode>(node)->AddTableEntry(dstAddr, interface);
                else {
                    node->GetObject<RdmaDriver>()->m_rdma->AddTableEntry(dstAddr, interface);
                }
            }
        }
    }
}

// take down the link between a and b, and redo the routing
void TakeDownLink(NodeContainer n, Ptr<Node> a, Ptr<Node> b) {
    if (!nbr2if[a][b].up)
        return;
    // take down link between a and b
    nbr2if[a][b].up = nbr2if[b][a].up = false;
    nextHop.clear();
    CalculateRoutes(n);
    // clear routing tables
    for (uint32_t i = 0; i < n.GetN(); i++) {
        if (n.Get(i)->GetNodeType() == 1)
            DynamicCast<SwitchNode>(n.Get(i))->ClearTable();
        else
            n.Get(i)->GetObject<RdmaDriver>()->m_rdma->ClearTable();
    }
    DynamicCast<QbbNetDevice>(a->GetDevice(nbr2if[a][b].idx))->TakeDown();
    DynamicCast<QbbNetDevice>(b->GetDevice(nbr2if[b][a].idx))->TakeDown();
    // reset routing table
    SetRoutingEntries();

    // redistribute qp on each host
    for (uint32_t i = 0; i < n.GetN(); i++) {
        if (n.Get(i)->GetNodeType() == 0)
            n.Get(i)->GetObject<RdmaDriver>()->m_rdma->RedistributeQp();
    }
}

uint64_t get_nic_rate(NodeContainer &n) {
    for (uint32_t i = 0; i < n.GetN(); i++)
        if (n.Get(i)->GetNodeType() == 0)
            return DynamicCast<QbbNetDevice>(n.Get(i)->GetDevice(1))->GetDataRate().GetBitRate();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Applications */

double poission_gen_interval(double avg_rate)
{
    if (avg_rate > 0)
        return -logf(1.0 - (double)rand() / RAND_MAX) / avg_rate;
    else
        return 0;
}

template<typename T>
T rand_range (T min, T max)
{
    return min + ((double)max - min) * rand () / RAND_MAX;
}

uint32_t numPriorities;
uint32_t prioRand = 0;


#define QUERY_DATA 300000

int tar = 0;
int get_target_leaf(int leafCount) {
    tar += 1;
    if (tar == leafCount) {
        tar = 0;
        return tar;
    }
    return tar;
}

std::vector<uint32_t> maxBinbufferOccupancy(100, 0); // ugly initialization

uint32_t binBuffer = 0;
uint32_t maxBin = 1;

void printBuffer(Ptr<OutputStreamWrapper> fout, NodeContainer switches, double delay) {
    binBuffer++;
    for (uint32_t i = 0; i < switches.GetN(); i++) {
        if (switches.Get(i)->GetNodeType()) { // switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(switches.Get(i));
            *fout->GetStream() << "switch " << i
                               << " buffer " << sw->m_mmu->totalUsed
                               << " egressOccupancyLossless " << sw->m_mmu->egressPoolUsed[0]
                               << " egressOccupancyLossy " << sw->m_mmu->egressPoolUsed[1]
                               << " ingressPoolOccupancy " << sw->m_mmu->totalUsed - sw->m_mmu->xoffTotalUsed
                               << " headroomOccupancy " << sw->m_mmu->xoffTotalUsed
                               <<  " time " << Simulator::Now().GetSeconds() << std::endl;
        }
    }
    if (binBuffer == maxBin)
        binBuffer = 0;

    Simulator::Schedule(Seconds(delay), printBuffer, fout, switches, delay);
}

/******************************************************************************************************************************************************************************************************/

int main(int argc, char *argv[])
{
    clock_t begint, endt;
    begint = clock();
    std::ifstream conf;
    bool powertcp = false;
    bool thetapowertcp = false;


    uint32_t SERVER_COUNT = 32;
    uint32_t LEAF_COUNT = 2; // LEAF and SPINE correspond to a single pod. Leafs are ToR switches and Spine are AGG switches. Count is within a single pod.
    uint32_t SPINE_COUNT = 8;
    uint64_t LEAF_SERVER_CAPACITY = 10;
    uint64_t SPINE_LEAF_CAPACITY = 40;

    double START_TIME = 0.1;
    double END_TIME = 0.010;
    double FLOW_LAUNCH_END_TIME = 5;

    double load = 0.2;

    uint32_t requestSize = 1000000;
    double queryRequestRate = 1;
    uint32_t incast = 5;

    uint32_t algorithm = 1;
    uint32_t windowCheck = 0;

    uint32_t bufferalgIngress = DT;
    uint32_t bufferalgEgress = DT;


    std::string confFile = "/home/vamsi/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/examples/buffer-devel/config-workload.txt";
    std::string cdfFileName = "/home/vamsi/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/examples/buffer-devel/websearch.txt";

    unsigned randomSeed = 7;


    CommandLine cmd;
    cmd.AddValue("conf", "config file path", confFile);
    cmd.AddValue("powertcp", "enable powertcp", powertcp);
    cmd.AddValue("thetapowertcp", "enable theta-powertcp, delay version", thetapowertcp);
    cmd.AddValue ("randomSeed", "Random seed, 0 for random generated", randomSeed);

    cmd.AddValue ("SERVER_COUNT", "servers per tor. Please specify the correct value according to the topology file information", SERVER_COUNT);
    cmd.AddValue ("LEAF_COUNT", "number of ToRs that receive traffic", LEAF_COUNT);
    cmd.AddValue ("SPINE_COUNT", "number of ToRs that receive traffic", SPINE_COUNT);
    cmd.AddValue ("LEAF_SERVER_CAPACITY", "tor to server capacity", LEAF_SERVER_CAPACITY);
    cmd.AddValue ("SPINE_LEAF_CAPACITY", "tor to aggregation switch capacity", SPINE_LEAF_CAPACITY);

    cmd.AddValue ("START_TIME", "sim start time", START_TIME);
    cmd.AddValue ("END_TIME", "sim end time", END_TIME);
    cmd.AddValue ("FLOW_LAUNCH_END_TIME", "flow launch process end time", FLOW_LAUNCH_END_TIME);
    cmd.AddValue ("cdfFileName", "File name for flow distribution", cdfFileName);
    cmd.AddValue ("load", "Load on the links from ToR to Agg switches (This is the only place with Over-subscription and is the correct place to load, 0.0 - 1.0", load);

    cmd.AddValue ("request", "Query Size in Bytes", requestSize);
    cmd.AddValue("queryRequestRate", "Query request rate (poisson arrivals)", queryRequestRate);

    cmd.AddValue ("algorithm", "specify CC mode. This is added for my convinience since I prefer cmd rather than parsing files.", algorithm);
    cmd.AddValue("windowCheck", "windowCheck", windowCheck);

    cmd.AddValue ("bufferalgIngress", "specify buffer management algorithm to be used at the ingress", bufferalgIngress);
    cmd.AddValue ("bufferalgEgress", "specify buffer management algorithm to be used at the egress", bufferalgEgress);

    cmd.AddValue("incast", "incast", incast);

    std::string fctOutFile = "./fcts.txt";
    cmd.AddValue ("fctOutFile", "File path for FCTs", fctOutFile);

    std::string torOutFile = "examples/buffer-devel/losslesssfirstDT-newbuffer.txt";
    cmd.AddValue ("torOutFile", "File path for ToR statistic", torOutFile);

    std::string pfcOutFile = "./pfc.txt";
    cmd.AddValue ("pfcOutFile", "File path for pfc events", pfcOutFile);

    std::string alphasFile = "/home/vamsi/src/phd/codebase/ns3-datacenter/simulator/ns-3.35/examples/buffer-devel/alphas0.25"; // On lakewood
    cmd.AddValue ("alphasFile", "alpha values file (should be exactly nPrior lines)", alphasFile);

    std::string bufferModel = "sonic";
    cmd.AddValue("bufferModel", "the buffer model to be used in the switch MMU", bufferModel);

    cmd.Parse (argc, argv);

    fctOutput = asciiTraceHelper.CreateFileStream (fctOutFile);
    torStats = torTraceHelper.CreateFileStream (torOutFile);

    std::string line;
    std::fstream aFile;
    aFile.open(alphasFile);
    uint32_t p=0;
    while ( getline( aFile, line ) && p < 8 ) { // hard coded to read only 8 alpha values.
        std::istringstream iss( line );
        double a;
        iss >> a;
        alpha_values[p] = a;
        p++;
    }
    aFile.close();

    SPINE_LEAF_CAPACITY = SPINE_LEAF_CAPACITY * LINK_CAPACITY_BASE;
    LEAF_SERVER_CAPACITY = LEAF_SERVER_CAPACITY * LINK_CAPACITY_BASE;

    conf.open(confFile.c_str());
    while (!conf.eof())
    {
        std::string key;
        conf >> key;

        if (key.compare("ENABLE_QCN") == 0)
        {
            uint32_t v;
            conf >> v;
            enable_qcn = v;
        }
        else if (key.compare("CLAMP_TARGET_RATE") == 0)
        {
            uint32_t v;
            conf >> v;
            clamp_target_rate = v;
        }
        else if (key.compare("PAUSE_TIME") == 0)
        {
            double v;
            conf >> v;
            pause_time = v;
        }
        else if (key.compare("DATA_RATE") == 0)
        {
            std::string v;
            conf >> v;
            data_rate = v;
        }
        else if (key.compare("LINK_DELAY") == 0)
        {
            std::string v;
            conf >> v;
            link_delay = v;
        }
        else if (key.compare("PACKET_PAYLOAD_SIZE") == 0)
        {
            uint32_t v;
            conf >> v;
            packet_payload_size = v;
        }
        else if (key.compare("L2_CHUNK_SIZE") == 0)
        {
            uint32_t v;
            conf >> v;
            l2_chunk_size = v;
        }
        else if (key.compare("L2_ACK_INTERVAL") == 0)
        {
            uint32_t v;
            conf >> v;
            l2_ack_interval = v;
        }
        else if (key.compare("L2_BACK_TO_ZERO") == 0)
        {
            uint32_t v;
            conf >> v;
            l2_back_to_zero = v;
        }
        else if (key.compare("TOPOLOGY_FILE") == 0)
        {
            std::string v;
            conf >> v;
            topology_file = v;
        }
        else if (key.compare("FLOW_FILE") == 0)
        {
            std::string v;
            conf >> v;
            flow_file = v;
        }
        else if (key.compare("SIMULATOR_STOP_TIME") == 0)
        {
            double v;
            conf >> v;
            simulator_stop_time = v;
        }
        else if (key.compare("ALPHA_RESUME_INTERVAL") == 0)
        {
            double v;
            conf >> v;
            alpha_resume_interval = v;
        }
        else if (key.compare("RP_TIMER") == 0)
        {
            double v;
            conf >> v;
            rp_timer = v;
        }
        else if (key.compare("EWMA_GAIN") == 0)
        {
            double v;
            conf >> v;
            ewma_gain = v;
        }
        else if (key.compare("FAST_RECOVERY_TIMES") == 0)
        {
            uint32_t v;
            conf >> v;
            fast_recovery_times = v;
        }
        else if (key.compare("RATE_AI") == 0)
        {
            std::string v;
            conf >> v;
            rate_ai = v;
        }
        else if (key.compare("RATE_HAI") == 0)
        {
            std::string v;
            conf >> v;
            rate_hai = v;
        }
        else if (key.compare("ERROR_RATE_PER_LINK") == 0)
        {
            double v;
            conf >> v;
            error_rate_per_link = v;
        }
        else if (key.compare("CC_MODE") == 0) {
            conf >> cc_mode;
        } else if (key.compare("RATE_DECREASE_INTERVAL") == 0) {
            double v;
            conf >> v;
            rate_decrease_interval = v;
        } else if (key.compare("MIN_RATE") == 0) {
            conf >> min_rate;
        } else if (key.compare("HAS_WIN") == 0) {
            conf >> has_win;
        } else if (key.compare("GLOBAL_T") == 0) {
            conf >> global_t;
        } else if (key.compare("MI_THRESH") == 0) {
            conf >> mi_thresh;
        } else if (key.compare("VAR_WIN") == 0) {
            uint32_t v;
            conf >> v;
            var_win = v;
        } else if (key.compare("FAST_REACT") == 0) {
            uint32_t v;
            conf >> v;
            fast_react = v;
        } else if (key.compare("U_TARGET") == 0) {
            conf >> u_target;
        } else if (key.compare("INT_MULTI") == 0) {
            conf >> int_multi;
        } else if (key.compare("RATE_BOUND") == 0) {
            uint32_t v;
            conf >> v;
            rate_bound = v;
        } else if (key.compare("ACK_HIGH_PRIO") == 0) {
            conf >> ack_high_prio;
        } else if (key.compare("DCTCP_RATE_AI") == 0) {
            conf >> dctcp_rate_ai;
        } else if (key.compare("KMAX_MAP") == 0) {
            int n_k ;
            conf >> n_k;
            for (int i = 0; i < n_k; i++) {
                uint64_t rate;
                uint32_t k;
                conf >> rate >> k;
                rate2kmax[rate] = k;
            }
        } else if (key.compare("KMIN_MAP") == 0) {
            int n_k ;
            conf >> n_k;
            for (int i = 0; i < n_k; i++) {
                uint64_t rate;
                uint32_t k;
                conf >> rate >> k;
                rate2kmin[rate] = k;
            }
        } else if (key.compare("PMAX_MAP") == 0) {
            int n_k ;
            conf >> n_k;
            for (int i = 0; i < n_k; i++) {
                uint64_t rate;
                double p;
                conf >> rate >> p;
                rate2pmax[rate] = p;
            }
        } else if (key.compare("BUFFER_SIZE") == 0) {
            conf >> buffer_size;
        } else if (key.compare("QLEN_MON_FILE") == 0) {
            conf >> qlen_mon_file;
        } else if (key.compare("QLEN_MON_START") == 0) {
            conf >> qlen_mon_start;
        } else if (key.compare("QLEN_MON_END") == 0) {
            conf >> qlen_mon_end;
        } else if (key.compare("MULTI_RATE") == 0) {
            int v;
            conf >> v;
            multi_rate = v;
        } else if (key.compare("SAMPLE_FEEDBACK") == 0) {
            int v;
            conf >> v;
            sample_feedback = v;
        } else if (key.compare("PINT_LOG_BASE") == 0) {
            conf >> pint_log_base;
        } else if (key.compare("PINT_PROB") == 0) {
            conf >> pint_prob;
        }
        fflush(stdout);
    }
    conf.close();

    // overriding config file. I prefer to use cmd arguments
    cc_mode = algorithm; // overrides configuration file
    has_win = windowCheck; // overrides configuration file

    Config::SetDefault("ns3::QbbNetDevice::PauseTime", UintegerValue(pause_time));
    Config::SetDefault("ns3::QbbNetDevice::QcnEnabled", BooleanValue(enable_qcn));

    // set int_multi
    IntHop::multi = int_multi;
    // IntHeader::mode
    if (cc_mode == 7) // timely, use ts
        IntHeader::mode = IntHeader::TS;
    else if (cc_mode == 3) // hpcc, powertcp, use int
        IntHeader::mode = IntHeader::NORMAL;
    else if (cc_mode == 10) // hpcc-pint
        IntHeader::mode = IntHeader::PINT;
    else // others, no extra header
        IntHeader::mode = IntHeader::NONE;

    // Set Pint
    if (cc_mode == 10) {
        Pint::set_log_base(pint_log_base);
        IntHeader::pint_bytes = Pint::get_n_bytes();
        printf("PINT bits: %d bytes: %d\n", Pint::get_n_bits(), Pint::get_n_bytes());
    }

    topof.open(topology_file.c_str());
    flowf.open(flow_file.c_str());
    uint32_t node_num, switch_num, tors, link_num, trace_num;
    topof >> node_num >> switch_num >> tors >> link_num ; // changed here. The previous order was node, switch, link // tors is not used. switch_num=tors for now.
    // std::cout << node_num << " " << switch_num << " " << tors <<  " " << link_num << std::endl;
    flowf >> flow_num;

    NodeContainer serverNodes;
    NodeContainer torNodes;
    NodeContainer spineNodes;
    NodeContainer switchNodes;
    NodeContainer allNodes;

    std::vector<uint32_t> node_type(node_num, 0);

    // std::cout << "switch_num " << switch_num << std::endl;
    for (uint32_t i = 0; i < switch_num; i++) {
        uint32_t sid;
        topof >> sid;
        // std::cout << "sid " << sid << std::endl;
        switchNumToId[i] = sid;
        switchIdToNum[sid] = i;
        if (i < LEAF_COUNT) {
            node_type[sid] = 1;
        }
        else
            node_type[sid] = 2;

    }

    for (uint32_t i = 0; i < node_num; i++) {
        if (node_type[i] == 0) {
            Ptr<Node> node = CreateObject<Node>();
            n.Add(node);
            allNodes.Add(node);
            serverNodes.Add(node);
        }
        else {
            Ptr<SwitchNode> sw = CreateObject<SwitchNode>();
            n.Add(sw);
            switchNodes.Add(sw);
            allNodes.Add(sw);
            sw->SetAttribute("EcnEnabled", BooleanValue(enable_qcn));
            sw->SetNodeType(1);
            if (node_type[i] == 1) {
                torNodes.Add(sw);
            }
            else if (node_type[i] == 2) {
                spineNodes.Add(sw);
            }
        }
    }


    NS_LOG_INFO("Create nodes.");

    Config::SetDefault ("ns3::Ipv4GlobalRouting::FlowEcmpRouting", BooleanValue(true));
    InternetStackHelper internet;
    Ipv4GlobalRoutingHelper globalRoutingHelper;
    internet.SetRoutingHelper (globalRoutingHelper);
    internet.Install(n);

    //
    // Assign IP to each server
    //
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType() == 0) { // is server
            serverAddress.resize(i + 1);
            serverAddress[i] = node_id_to_ip(i);
        }
    }

    NS_LOG_INFO("Create channels.");

    //
    // Explicitly create the channels required by the topology.
    //

    Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
    rem->SetRandomVariable(uv);
    uv->SetStream(50);
    rem->SetAttribute("ErrorRate", DoubleValue(error_rate_per_link));
    rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));

    FILE *pfc_file = fopen(pfcOutFile.c_str(), "w");

    QbbHelper qbb;
    Ipv4AddressHelper ipv4;
    for (uint32_t i = 0; i < link_num; i++)
    {
        uint32_t src, dst;
        std::string data_rate, link_delay;
        double error_rate;
        topof >> src >> dst >> data_rate >> link_delay >> error_rate;

        // std::cout << src << " " << dst << " " << n.GetN() << std::endl;
        Ptr<Node> snode = n.Get(src), dnode = n.Get(dst);


        qbb.SetDeviceAttribute("DataRate", StringValue(data_rate));
        qbb.SetChannelAttribute("Delay", StringValue(link_delay));

        if (error_rate > 0)
        {
            Ptr<RateErrorModel> rem = CreateObject<RateErrorModel>();
            Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
            rem->SetRandomVariable(uv);
            uv->SetStream(50);
            rem->SetAttribute("ErrorRate", DoubleValue(error_rate));
            rem->SetAttribute("ErrorUnit", StringValue("ERROR_UNIT_PACKET"));
            qbb.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
        }
        else
        {
            qbb.SetDeviceAttribute("ReceiveErrorModel", PointerValue(rem));
        }

        fflush(stdout);

        // Assigne server IP
        // Note: this should be before the automatic assignment below (ipv4.Assign(d)),
        // because we want our IP to be the primary IP (first in the IP address list),
        // so that the global routing is based on our IP
        NetDeviceContainer d = qbb.Install(snode, dnode);
        // std::cout << "link " << src << " " << dst << std::endl;
        if (snode->GetNodeType() == 0) {
            Ptr<Ipv4> ipv4 = snode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(0));
            ipv4->AddAddress(1, Ipv4InterfaceAddress(serverAddress[src], Ipv4Mask(0xff000000)));
        }
        if (dnode->GetNodeType() == 0) {
            Ptr<Ipv4> ipv4 = dnode->GetObject<Ipv4>();
            ipv4->AddInterface(d.Get(1));
            ipv4->AddAddress(1, Ipv4InterfaceAddress(serverAddress[dst], Ipv4Mask(0xff000000)));
        }


        if (!snode->GetNodeType()) {
            sourceNodes[src].Add(DynamicCast<QbbNetDevice>(d.Get(0)));
        }

        if (!snode->GetNodeType() && dnode->GetNodeType()) {
            switchDown[switchIdToNum[dst]].Add(DynamicCast<QbbNetDevice>(d.Get(1)));
        }


        if (snode->GetNodeType() && dnode->GetNodeType()) {
            switchToSwitchInterfaces.Add(d);
            switchUp[switchIdToNum[src]].Add(DynamicCast<QbbNetDevice>(d.Get(0)));
            switchUp[switchIdToNum[dst]].Add(DynamicCast<QbbNetDevice>(d.Get(1)));
            switchToSwitch[src][dst].push_back(DynamicCast<QbbNetDevice>(d.Get(0)));
            switchToSwitch[src][dst].push_back(DynamicCast<QbbNetDevice>(d.Get(1)));
        }

        // used to create a graph of the topology
        nbr2if[snode][dnode].idx = DynamicCast<QbbNetDevice>(d.Get(0))->GetIfIndex();
        nbr2if[snode][dnode].up = true;
        nbr2if[snode][dnode].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(0))->GetChannel())->GetDelay().GetTimeStep();
        nbr2if[snode][dnode].bw = DynamicCast<QbbNetDevice>(d.Get(0))->GetDataRate().GetBitRate();
        nbr2if[dnode][snode].idx = DynamicCast<QbbNetDevice>(d.Get(1))->GetIfIndex();
        nbr2if[dnode][snode].up = true;
        nbr2if[dnode][snode].delay = DynamicCast<QbbChannel>(DynamicCast<QbbNetDevice>(d.Get(1))->GetChannel())->GetDelay().GetTimeStep();
        nbr2if[dnode][snode].bw = DynamicCast<QbbNetDevice>(d.Get(1))->GetDataRate().GetBitRate();

        // This is just to set up the connectivity between nodes. The IP addresses are useless
        // char ipstring[16];
        std::stringstream ipstring;
        ipstring << "10." << i / 254 + 1 << "." << i % 254 + 1 << ".0";
        // sprintf(ipstring, "10.%d.%d.0", i / 254 + 1, i % 254 + 1);
        ipv4.SetBase(ipstring.str().c_str(), "255.255.255.0");
        // ipv4.SetBase(ipstring, "255.255.255.0");
        ipv4.Assign(d);

        // setup PFC trace
        DynamicCast<QbbNetDevice>(d.Get(0))->TraceConnectWithoutContext("QbbPfc", MakeBoundCallback (&get_pfc, pfc_file, DynamicCast<QbbNetDevice>(d.Get(0))));
        DynamicCast<QbbNetDevice>(d.Get(1))->TraceConnectWithoutContext("QbbPfc", MakeBoundCallback (&get_pfc, pfc_file, DynamicCast<QbbNetDevice>(d.Get(1))));
    }

    nic_rate = get_nic_rate(n);

#if ENABLE_QP
    //
    // install RDMA driver
    //
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType() == 0) { // is server
            // create RdmaHw
            Ptr<RdmaHw> rdmaHw = CreateObject<RdmaHw>();
            rdmaHw->SetAttribute("ClampTargetRate", BooleanValue(clamp_target_rate));
            rdmaHw->SetAttribute("AlphaResumInterval", DoubleValue(alpha_resume_interval));
            rdmaHw->SetAttribute("RPTimer", DoubleValue(rp_timer));
            rdmaHw->SetAttribute("FastRecoveryTimes", UintegerValue(fast_recovery_times));
            rdmaHw->SetAttribute("EwmaGain", DoubleValue(ewma_gain));
            rdmaHw->SetAttribute("RateAI", DataRateValue(DataRate(rate_ai)));
            rdmaHw->SetAttribute("RateHAI", DataRateValue(DataRate(rate_hai)));
            rdmaHw->SetAttribute("L2BackToZero", BooleanValue(l2_back_to_zero));
            rdmaHw->SetAttribute("L2ChunkSize", UintegerValue(l2_chunk_size));
            rdmaHw->SetAttribute("L2AckInterval", UintegerValue(l2_ack_interval));
            rdmaHw->SetAttribute("CcMode", UintegerValue(cc_mode));
            rdmaHw->SetAttribute("RateDecreaseInterval", DoubleValue(rate_decrease_interval));
            rdmaHw->SetAttribute("MinRate", DataRateValue(DataRate(min_rate)));
            rdmaHw->SetAttribute("Mtu", UintegerValue(packet_payload_size));
            rdmaHw->SetAttribute("MiThresh", UintegerValue(mi_thresh));
            rdmaHw->SetAttribute("VarWin", BooleanValue(var_win));
            rdmaHw->SetAttribute("FastReact", BooleanValue(fast_react));
            rdmaHw->SetAttribute("MultiRate", BooleanValue(multi_rate));
            rdmaHw->SetAttribute("SampleFeedback", BooleanValue(sample_feedback));
            rdmaHw->SetAttribute("TargetUtil", DoubleValue(u_target));
            rdmaHw->SetAttribute("RateBound", BooleanValue(rate_bound));
            rdmaHw->SetAttribute("DctcpRateAI", DataRateValue(DataRate(dctcp_rate_ai)));
            rdmaHw->SetAttribute("PowerTCPEnabled", BooleanValue(powertcp));
            rdmaHw->SetAttribute("PowerTCPdelay", BooleanValue(thetapowertcp));
            rdmaHw->SetPintSmplThresh(pint_prob);
            // create and install RdmaDriver
            Ptr<RdmaDriver> rdma = CreateObject<RdmaDriver>();
            Ptr<Node> node = n.Get(i);
            rdma->SetNode(node);
            rdma->SetRdmaHw(rdmaHw);

            node->AggregateObject (rdma);
            rdma->Init();
            rdma->TraceConnectWithoutContext("QpComplete", MakeBoundCallback (qp_finish, fctOutput));
        }
    }

#endif

    // set ACK priority on hosts
    if (ack_high_prio)
        RdmaEgressQueue::ack_q_idx = 0;
    else
        RdmaEgressQueue::ack_q_idx = 3;

    // setup routing
    CalculateRoutes(n);
    SetRoutingEntries();
    //
    // get BDP and delay
    //
    maxRtt = maxBdp = 0;
    uint64_t minRtt = 1e9;
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType() != 0)
            continue;
        for (uint32_t j = 0; j < node_num; j++) {
            if (n.Get(j)->GetNodeType() != 0)
                continue;
            if (i == j)
                continue;
            uint64_t delay = pairDelay[n.Get(i)][n.Get(j)];
            uint64_t txDelay = pairTxDelay[n.Get(i)][n.Get(j)];
            uint64_t rtt = delay * 2 + txDelay;
            uint64_t bw = pairBw[i][j];
            uint64_t bdp = rtt * bw / 1000000000 / 8;
            pairBdp[n.Get(i)][n.Get(j)] = bdp;
            pairRtt[i][j] = rtt;
            if (bdp > maxBdp)
                maxBdp = bdp;
            if (rtt > maxRtt)
                maxRtt = rtt;
            if (rtt < minRtt)
                minRtt = rtt;
        }
    }
    printf("maxRtt=%lu maxBdp=%lu minRtt=%lu\n", maxRtt, maxBdp, minRtt);


        // config switch
    // The switch mmu runs Dynamic Thresholds (DT) by default.
    uint64_t totalHeadroom;
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType()) { // is switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n.Get(i));
            totalHeadroom = 0;
            sw->m_mmu->SetIngressLossyAlg(bufferalgIngress);
            sw->m_mmu->SetIngressLosslessAlg(bufferalgIngress);
            sw->m_mmu->SetEgressLossyAlg(bufferalgEgress);
            sw->m_mmu->SetEgressLosslessAlg(bufferalgEgress);
            sw->m_mmu->SetABMalphaHigh(1024);
            sw->m_mmu->SetABMdequeueUpdateNS(minRtt);
            sw->m_mmu->SetPortCount(sw->GetNDevices()-1); // set the actual port count here so that we don't always iterate over the default 256 ports.
            sw->m_mmu->SetBufferModel(bufferModel);
            // std::cout << "ports " << sw->GetNDevices() << " node " << i << std::endl;
            for (uint32_t j = 1; j < sw->GetNDevices(); j++) {
                Ptr<QbbNetDevice> dev = DynamicCast<QbbNetDevice>(sw->GetDevice(j));
                uint64_t rate = dev->GetDataRate().GetBitRate();
                // set port bandwidth in the mmu, used by ABM.
                sw->m_mmu->bandwidth[j] = rate;
                for (uint32_t qu = 0; qu < 8; qu++) {
                    if (qu != 1) { // lossless
                        sw->m_mmu->SetAlphaIngress(alpha_values[qu], j, qu);
                        sw->m_mmu->SetAlphaEgress(10000, j, qu);
                    }
                    else { // lossy
                        sw->m_mmu->SetAlphaIngress(10000, j, qu);
                        sw->m_mmu->SetAlphaEgress(alpha_values[qu], j, qu);
                    }

                    // set ecn
                    NS_ASSERT_MSG(rate2kmin.find(rate) != rate2kmin.end(), "must set kmin for each link speed");
                    NS_ASSERT_MSG(rate2kmax.find(rate) != rate2kmax.end(), "must set kmax for each link speed");
                    NS_ASSERT_MSG(rate2pmax.find(rate) != rate2pmax.end(), "must set pmax for each link speed");
                    sw->m_mmu->ConfigEcn(j, rate2kmin[rate], rate2kmax[rate], rate2pmax[rate]);
                    // set pfc
                    double delay = DynamicCast<QbbChannel>(dev->GetChannel())->GetDelay().GetSeconds();
                    uint32_t headroom = (packet_payload_size + 48) * 2 + 3860 + (2 * rate * delay / 8);
                    // std::cout << headroom << std::endl;
                    sw->m_mmu->SetHeadroom(headroom, j, qu);
                    totalHeadroom += headroom;
                }

            }
            sw->m_mmu->SetBufferPool(buffer_size * 1024 * 1024);
            sw->m_mmu->SetIngressPool(buffer_size * 1024 * 1024 - totalHeadroom);
            sw->m_mmu->SetEgressLosslessPool(buffer_size * 1024 * 1024);
            sw->m_mmu->SetEgressLossyPool((buffer_size * 1024 * 1024 - totalHeadroom) * 0.8);
            sw->m_mmu->node_id = sw->GetId();
        }
        if (n.Get(i)->GetNodeType())
            std::cout << "total headroom: " << totalHeadroom << " ingressPool " << buffer_size * 1024 * 1024 - totalHeadroom << " egressLosslessPool " << buffer_size * 1024 * 1024 << " egressLossyPool " << (uint64_t)((buffer_size * 1024 * 1024 - totalHeadroom) * 0.8) <<  std::endl;
    }

    //
    // setup switch CC
    //
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType()) { // switch
            Ptr<SwitchNode> sw = DynamicCast<SwitchNode>(n.Get(i));
            sw->SetAttribute("CcMode", UintegerValue(cc_mode));
            sw->SetAttribute("MaxRtt", UintegerValue(maxRtt));
            sw->SetAttribute("PowerEnabled", BooleanValue(powertcp));
        }
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO("Create Applications.");

    Time interPacketInterval = Seconds(0.0000005 / 2);


    // maintain port number for each host
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType() == 0)
            for (uint32_t j = 0; j < node_num; j++) {
                if (n.Get(j)->GetNodeType() == 0)
                    portNumder[i][j] = rand_range(10000, 11000); // each host pair use port number from 10000
            }
    }
    DestportNumder = portNumder;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* Applications Background*/
    NS_LOG_INFO ("Create background applications");

    // maintain port number for each host
    for (uint32_t i = 0; i < node_num; i++) {
        if (n.Get(i)->GetNodeType() == 0)
            for (uint32_t j = 0; j < node_num; j++) {
                if (n.Get(j)->GetNodeType() == 0)
                    portNumder[i][j] = 10000; // each host pair use port number from 10000
            }
    }


    flow_input.idx = 0;
    if (flow_num > 0) {
        ReadFlowInput();
        // std::cout << flow_input.start_time << std::endl;
        Simulator::Schedule(Seconds(flow_input.start_time) - Simulator::Now(), ScheduleFlowInputs);
    }


    /*General TCP Socket settings. Mostly used by various congestion control algorithms in common*/
    Config::SetDefault ("ns3::TcpSocket::ConnTimeout", TimeValue (MilliSeconds (10))); // syn retry interval
    Config::SetDefault ("ns3::TcpSocketBase::MinRto", TimeValue (MicroSeconds (10000)) );  //(MilliSeconds (5))
    Config::SetDefault ("ns3::TcpSocketBase::RTTBytes", UintegerValue ( maxBdp ));  //(MilliSeconds (5))
    Config::SetDefault ("ns3::TcpSocketBase::ClockGranularity", TimeValue (NanoSeconds (10))); //(MicroSeconds (100))
    Config::SetDefault ("ns3::RttEstimator::InitialEstimation", TimeValue (MicroSeconds (10))); //TimeValue (MicroSeconds (80))
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1073725440)); //1073725440
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1073725440));
    Config::SetDefault ("ns3::TcpSocket::ConnCount", UintegerValue (6));  // Syn retry count
    Config::SetDefault ("ns3::TcpSocketBase::Timestamp", BooleanValue (true));
    Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1000));
    Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (0));
    Config::SetDefault ("ns3::TcpSocket::PersistTimeout", TimeValue (Seconds (20)));

    Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (ns3::TcpCubic::GetTypeId()));

    double startTime = 0.005;
    uint32_t flowCount = 1;
    uint32_t prior = 1;


    uint16_t port = 4444;

    uint64_t flowSize = 1e9;

    for (uint32_t i = 0; i < 16; i++) {
        uint32_t rxServer = 16 + i % 8;
        // std::cout << "tx " << i << " rx " << rxServer << std::endl;
        Ptr<Node> rxNode = n.Get (rxServer);
        Ptr<Ipv4> ip = rxNode->GetObject<Ipv4> ();
        Ipv4InterfaceAddress rxInterface = ip->GetAddress (1, 0);
        Ipv4Address rxAddress = rxInterface.GetLocal ();
        for (uint32_t j = 0; j < 8 ; j++) {
            InetSocketAddress ad (rxAddress, port);
            Address sinkAddress(ad);
            Ptr<BulkSendApplication> bulksend = CreateObject<BulkSendApplication>();
            bulksend->SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId()));
            bulksend->SetAttribute ("SendSize", UintegerValue (flowSize));
            bulksend->SetAttribute ("MaxBytes", UintegerValue(flowSize));
            bulksend->SetAttribute("FlowId", UintegerValue(flowCount++));
            bulksend->SetAttribute("priorityCustom", UintegerValue(prior));
            bulksend->SetAttribute("Remote", AddressValue(sinkAddress));
            bulksend->SetAttribute("InitialCwnd", UintegerValue (10));
            bulksend->SetAttribute("priority", UintegerValue(prior));
            bulksend->SetStartTime (Seconds(startTime));
            bulksend->SetStopTime (Seconds (END_TIME));
            n.Get (i)->AddApplication(bulksend);

            PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
            ApplicationContainer sinkApp = sink.Install (n.Get(rxServer));
            sinkApp.Get(0)->SetAttribute("TotalQueryBytes", UintegerValue(flowSize));
            sinkApp.Get(0)->SetAttribute("priority", UintegerValue(prior));
            sinkApp.Get(0)->SetAttribute("priorityCustom", UintegerValue(prior));
            sinkApp.Get(0)->SetAttribute("flowId", UintegerValue(flowCount));
            sinkApp.Get(0)->SetAttribute("senderPriority", UintegerValue(prior));
            flowCount += 1;
            sinkApp.Start (Seconds(startTime));
            sinkApp.Stop (Seconds (END_TIME));
            sinkApp.Get(0)->TraceConnectWithoutContext("FlowFinish", MakeBoundCallback(&TraceMsgFinish, fctOutput));

            port++;
        }
    }

    topof.close();
    tracef.close();
    double delay = 1.5 * minRtt * 1e-9; // 10 micro seconds
    Simulator::Schedule(Seconds(delay), printBuffer, torStats, torNodes, delay);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // AsciiTraceHelper ascii;
    // qbb.EnableAsciiAll (ascii.CreateFileStream ("eval.tr"));
    // std::cout << "Running Simulation.\n";
    NS_LOG_INFO("Run Simulation.");
    Simulator::Stop(Seconds(END_TIME));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Done.");

    endt = clock();
    // std::cout << (double)(endt - begint) / CLOCKS_PER_SEC << "\n";
}
