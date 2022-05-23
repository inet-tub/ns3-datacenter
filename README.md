
# ns3-datacenter

This repository contains the source code used for [PowerTCP](https://www.usenix.org/conference/nsdi22/presentation/addanki) and ABM.

We extend ns-3.35 to support some of the recent advancements in the datacenter context.
- Various datacenter congestion control algorithms including PowerTCP over both TCP/IP and RDMA stacks can be used simulataneously. 
- The switch MMU is based on SONIC buffer model (purely based on our understanding only). 
- Support for several Buffer Management algorithms including ABM.

Many additions to the source code are based on prior work: [ns3-rdma](https://github.com/bobzhuyb/ns3-rdma) and [HPCC](https://github.com/alibaba-edu/High-Precision-Congestion-Control). Please consider citing the following papers if you use this repository in your research.

```bib
@inproceedings{nsdi22,
  author = {Addanki, Vamsi and Michel, Oliver and Schmid, Stefan},
  title = {{PowerTCP}: Pushing the Performance Limits of Datacenter Networks},
  booktitle = {19th USENIX Symposium on Networked Systems Design and Implementation (NSDI 22)},
  year = {2022},
  address = {Renton, WA},
  url = {https://www.usenix.org/conference/nsdi22/presentation/addanki},
  publisher = {USENIX Association},
  month = {April},
}

@inproceedings{abm,
  author = {Addanki, Vamsi and Apostolaki, Maria and Ghobadi, Manya and Schmid, Stefan and Vanbever, Laurent},
  title = {ABM: Active Buffer Management in Datacenters},
  year = {2022},
  month = {August},
  booktitle = {Proceedings of the ACM SIGCOMM 2022 Conference},
  address = {Amsterdam, Netherlands}
}

```

# Configure and Build

In the following, `$REPO` = path to the root directory of this repository. Change $REPO accordingly.

**Configure ns3:**

```bash
cd $REPO/simulator/ns-3.35/
CXXFLAGS=-w ./waf configure --build-profile=optimized --disable-example --disable-tests --disable-python
```
**Build**

```bash
cd $REPO/simulator/ns-3.35/
./waf
```

# Running PowerTCP

Checkout [`simulator/ns-3.35/examples/PowerTCP`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.35/examples/PowerTCP) for various simulation files and scripts to run PowerTCP. More documentation will be added in the upcoming days.

# Changes to NS-3.35

```diff
 README.md                                          |    6 +-
 config.sh                                          |    2 -
 .../examples/PowerTCP/buildsimple.sh (new +x)      |    5 +
 simulator/ns-3.35/examples/PowerTCP/cdf.c (new)    |  144 ++
 simulator/ns-3.35/examples/PowerTCP/cdf.h (new)    |   43 +
 .../examples/PowerTCP/config-burst.txt (new)       |   58 +
 .../examples/PowerTCP/config-fairness.txt (new)    |   58 +
 .../examples/PowerTCP/config-workload.txt (new)    |   58 +
 .../ns-3.35/examples/PowerTCP/config.sh (new)      |    1 +
 .../ns-3.35/examples/PowerTCP/config.txt (new)     |   58 +
 simulator/ns-3.35/examples/PowerTCP/fct.py (new)   |  493 +++++
 .../examples/PowerTCP/flow-burstExp.txt (new)      |   12 +
 .../examples/PowerTCP/flow-fairnessExp.txt (new)   |    5 +
 simulator/ns-3.35/examples/PowerTCP/flow.txt (new) |  211 ++
 .../examples/PowerTCP/generate_longflows.py (new)  |   20 +
 .../examples/PowerTCP/generate_topology.py (new)   |   55 +
 .../ns-3.35/examples/PowerTCP/plot-burst.py (new)  |  112 +
 .../examples/PowerTCP/plot-fairness.py (new)       |   88 +
 .../examples/PowerTCP/plot-workload.py (new)       |  589 ++++++
 .../PowerTCP/powertcp-evaluation-burst.cc (new)    | 1109 ++++++++++
 .../PowerTCP/powertcp-evaluation-fairness.cc (new) | 1088 ++++++++++
 .../PowerTCP/powertcp-evaluation-workload.cc (new) | 1356 ++++++++++++
 .../examples/PowerTCP/results-burst.sh (new +x)    |   44 +
 .../examples/PowerTCP/results-fairness.sh (new +x) |   48 +
 .../examples/PowerTCP/results-workload.sh (new +x) |  112 +
 .../examples/PowerTCP/script-burst.sh (new +x)     |   92 +
 .../examples/PowerTCP/script-fairness.sh (new +x)  |   75 +
 .../examples/PowerTCP/script-workload.sh (new +x)  |  202 ++
 .../ns-3.35/examples/PowerTCP/set_cc.sh (new +x)   |    6 +
 .../ns-3.35/examples/PowerTCP/topology.txt (new)   |  146 ++
 .../ns-3.35/examples/PowerTCP/websearch.txt (new)  |   16 +
 simulator/ns-3.35/examples/PowerTCP/wscript (new)  |   15 +
 simulator/ns-3.35/examples/tcp/dctcp-example.cc    |    2 +
 .../helper/rdma-client-helper.cc (new)             |   65 +
 .../applications/helper/rdma-client-helper.h (new) |   79 +
 .../applications/model/bulk-send-application.cc    |   75 +-
 .../src/applications/model/bulk-send-application.h |   14 +
 .../ns-3.35/src/applications/model/packet-sink.cc  |   48 +
 .../ns-3.35/src/applications/model/packet-sink.h   |   12 +
 .../src/applications/model/rdma-client.cc (new)    |  157 ++
 .../src/applications/model/rdma-client.h (new)     |   87 +
 .../ns-3.35/src/applications/model/udp-client.cc   |    2 +-
 .../ns-3.35/src/applications/model/udp-server.cc   |    2 +-
 .../src/applications/model/udp-trace-client.cc     |    2 +-
 simulator/ns-3.35/src/applications/wscript         |   12 +-
 .../src/core/model/random-variable.cc (new)        | 2157 ++++++++++++++++++++
 .../ns-3.35/src/core/model/random-variable.h (new) |  755 +++++++
 simulator/ns-3.35/src/core/wscript                 |    2 +
 .../src/internet/model/ipv4-global-routing.cc      |  198 +-
 .../src/internet/model/ipv4-global-routing.h       |    5 +
 .../ns-3.35/src/internet/model/ipv4-l3-protocol.cc |    1 +
 simulator/ns-3.35/src/internet/model/rdma.h (new)  |    6 +
 .../src/internet/model/tcp-advanced.cc (new)       |  607 ++++++
 .../src/internet/model/tcp-advanced.h (new)        |  161 ++
 .../src/internet/model/tcp-congestion-ops.h        |   28 +
 .../ns-3.35/src/internet/model/tcp-l4-protocol.cc  |    3 +-
 .../ns-3.35/src/internet/model/tcp-socket-base.cc  |  169 +-
 .../ns-3.35/src/internet/model/tcp-socket-base.h   |   13 +
 .../ns-3.35/src/internet/model/tcp-socket-state.cc |   26 +
 .../ns-3.35/src/internet/model/tcp-socket-state.h  |   30 +
 simulator/ns-3.35/src/internet/wscript             |    3 +
 simulator/ns-3.35/src/network/model/buffer.cc      |    6 +
 simulator/ns-3.35/src/network/model/buffer.h       |    4 +
 simulator/ns-3.35/src/network/model/net-device.cc  |    9 +
 simulator/ns-3.35/src/network/model/net-device.h   |    6 +
 simulator/ns-3.35/src/network/model/node.cc        |   25 +-
 simulator/ns-3.35/src/network/model/node.h         |   25 +-
 simulator/ns-3.35/src/network/model/packet.cc      |    6 +
 simulator/ns-3.35/src/network/model/packet.h       |    5 +-
 .../network/utils/broadcom-egress-queue.cc (new)   |  262 +++
 .../network/utils/broadcom-egress-queue.h (new)    |   81 +
 .../src/network/utils/custom-header.cc (new)       |  344 ++++
 .../src/network/utils/custom-header.h (new)        |  146 ++
 .../src/network/utils/custom-priority-tag.cc (new) |   61 +
 .../src/network/utils/custom-priority-tag.h (new)  |   54 +
 simulator/ns-3.35/src/network/utils/data-rate.cc   |   51 +-
 simulator/ns-3.35/src/network/utils/data-rate.h    |   34 +-
 .../src/network/utils/feedback-tag.cc (new)        |   86 +
 .../ns-3.35/src/network/utils/feedback-tag.h (new) |   67 +
 .../ns-3.35/src/network/utils/int-header.cc (new)  |   94 +
 .../ns-3.35/src/network/utils/int-header.h (new)   |  119 ++
 .../src/network/utils/interface-tag.cc (new)       |  105 +
 .../src/network/utils/interface-tag.h (new)        |   47 +
 .../ns-3.35/src/network/utils/rdma-tag.cc (new)    |   62 +
 .../ns-3.35/src/network/utils/rdma-tag.h (new)     |   49 +
 .../model => network/utils}/seq-ts-header.cc       |   51 +-
 .../model => network/utils}/seq-ts-header.h        |   38 +-
 .../ns-3.35/src/network/utils/unsched-tag.cc (new) |   62 +
 .../ns-3.35/src/network/utils/unsched-tag.h (new)  |   49 +
 simulator/ns-3.35/src/network/wscript              |   18 +
 .../src/point-to-point/helper/qbb-helper.cc (new)  |  453 ++++
 .../src/point-to-point/helper/qbb-helper.h (new)   |  202 ++
 .../src/point-to-point/helper/sim-setting.h (new)  |   53 +
 .../src/point-to-point/model/cn-header.cc (new)    |  177 ++
 .../src/point-to-point/model/cn-header.h (new)     |  102 +
 .../src/point-to-point/model/pause-header.cc (new) |  114 ++
 .../src/point-to-point/model/pause-header.h (new)  |   77 +
 .../ns-3.35/src/point-to-point/model/pint.cc (new) |   44 +
 .../ns-3.35/src/point-to-point/model/pint.h (new)  |   19 +
 .../model/point-to-point-net-device.cc             |    5 +
 .../model/point-to-point-net-device.h              |    8 +-
 .../ns-3.35/src/point-to-point/model/ppp-header.cc |    5 +-
 .../ns-3.35/src/point-to-point/model/ppp-header.h  |   20 +-
 .../src/point-to-point/model/qbb-channel.cc (new)  |  174 ++
 .../src/point-to-point/model/qbb-channel.h (new)   |  172 ++
 .../src/point-to-point/model/qbb-header.cc (new)   |  131 ++
 .../src/point-to-point/model/qbb-header.h (new)    |   76 +
 .../point-to-point/model/qbb-net-device.cc (new)   |  698 +++++++
 .../point-to-point/model/qbb-net-device.h (new)    |  263 +++
 .../model/qbb-remote-channel.cc (new)              |   81 +
 .../model/qbb-remote-channel.h (new)               |   47 +
 .../src/point-to-point/model/rdma-driver.cc (new)  |   71 +
 .../src/point-to-point/model/rdma-driver.h (new)   |   44 +
 .../src/point-to-point/model/rdma-hw.cc (new)      | 1275 ++++++++++++
 .../src/point-to-point/model/rdma-hw.h (new)       |  165 ++
 .../point-to-point/model/rdma-queue-pair.cc (new)  |  246 +++
 .../point-to-point/model/rdma-queue-pair.h (new)   |  171 ++
 .../src/point-to-point/model/switch-mmu.cc (new)   |  412 ++++
 .../src/point-to-point/model/switch-mmu.h (new)    |  119 ++
 .../src/point-to-point/model/switch-node.cc (new)  |  364 ++++
 .../src/point-to-point/model/switch-node.h (new)   |   64 +
 .../src/point-to-point/model/trace-format.h (new)  |   92 +
 simulator/ns-3.35/src/point-to-point/wscript       |   37 +-
 simulator/ns-3.35/src/wifi/model/wifi-phy-state.h  |    2 +-
 124 files changed, 18888 insertions(+), 109 deletions(-)

```