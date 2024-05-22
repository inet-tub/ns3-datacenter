
# ns3-datacenter

This repository contains the source code used for the following papers: 
- [PowerTCP (NSDI 2022)](https://www.usenix.org/conference/nsdi22/presentation/addanki)
- [ABM (SIGCOMM 2022)](https://dl.acm.org/doi/10.1145/3544216.3544252)
- [Reverie (NSDI 2024)](https://www.usenix.org/conference/nsdi24)
- [Credence (NSDI 2024)](https://www.usenix.org/conference/nsdi24)

**V1.0 Release**

We extend ns-3.39 to support some of the recent advancements in the datacenter context.
- Various datacenter congestion control algorithms including PowerTCP over both TCP/IP and RDMA stacks can be used simulataneously. 
- The switch MMU is based on SONIC buffer model (purely based on our understanding only). The switch MMU can also be configured based on Reverie model. 
- Support for several Buffer Management algorithms including ABM, Reverie and Credence.
- Integration with pybind11 enables interesting applications such as obtaining predictions from a scikit-learn trained model (see Credence examples).

Previous versions of the repository can be found here: [Releases](https://github.com/inet-tub/ns3-datacenter/releases/)

Many additions to the source code are based on prior work: [ns3-rdma](https://github.com/bobzhuyb/ns3-rdma) and [HPCC](https://github.com/alibaba-edu/High-Precision-Congestion-Control). Please consider citing the following papers if you use this repository in your research.

```bib
@inproceedings{nsdi22,
  author = {Addanki, Vamsi and Michel, Oliver and Schmid, Stefan},
  title = {{PowerTCP}: Pushing the Performance Limits of Datacenter Networks},
  booktitle = {19th USENIX Symposium on Networked Systems Design and Implementation (NSDI 22)},
  year = {2022},
  month = {April},
  address = {Renton, WA},
  url = {https://www.usenix.org/conference/nsdi22/presentation/addanki},
  publisher = {USENIX Association},
}

@inproceedings{abm,
  author = {Addanki, Vamsi and Apostolaki, Maria and Ghobadi, Manya and Schmid, Stefan and Vanbever, Laurent},
  title = {ABM: Active Buffer Management in Datacenters},
  year = {2022},
  month = {August},
  booktitle = {Proceedings of the ACM SIGCOMM 2022 Conference},
  address = {Amsterdam, Netherlands}
}

@inproceedings{reverie,
  author = {Addanki, Vamsi and Bai, Wei and Schmid, Stefan and Apostolaki, Maria},
  title = {Reverie: Low Pass Filter-Based Switch Buffer Sharing for Datacenters with RDMA and TCP Traffic},
  year = {2024},
  month = {April},
  booktitle = {21th USENIX Symposium on Networked Systems Design and Implementation (NSDI 24)},
  address = {Santa Clara, CA},
  publisher = {USENIX Association}
}

@inproceedings{credence,
  author = {Addanki, Vamsi and Pacut, Maciej and Schmid, Stefan},
  title = {Credence: Augmenting Datacenter Switch Buffer Sharing with ML Predictions},
  year = {2024},
  month = {April},
  booktitle = {21th USENIX Symposium on Networked Systems Design and Implementation (NSDI 24)},
  address = {Santa Clara, CA},
  publisher = {USENIX Association}
}


```

# Configure and Build

In the following, `$REPO` = path to the root directory of this repository. Change $REPO accordingly.

**Configure ns3:**

```bash
cd $REPO/simulator/ns-3.39/
./configure.sh
```
**Build:**

```bash
cd $REPO/simulator/ns-3.39/
./waf
```

# Running PowerTCP

Checkout [`simulator/ns-3.39/examples/PowerTCP`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/examples/PowerTCP) for various simulation files and scripts to run PowerTCP. 

# Running ABM

Checkout [`simulator/ns-3.39/examples/ABM`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/examples/ABM) for simulation files and scripts to run ABM and other buffer management algorithms. **Note:** This only runs in the TCP/IP stack and this is what we used in the paper. Checkout [`switch MMU`](simulator/ns-3.39/src/point-to-point/model/switch-mmu.cc) for a more general implementation of ABM suited for both RDMA stack and mixed stacks (RDMA + TCP/IP sharing the buffer).

# Running Credence
Checkout [`simulator/ns-3.39/examples/Credence`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/examples/Credence). Sanitized code and documentation will be updated soon...

# Running Reverie
Checkout [`simulator/ns-3.39/examples/Reverie`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/examples/Reverie). Sanitized code and documentation will be updated by soon...

# Important Files

[`simulator/ns-3.39/src/point-to-point/model/qbb-net-device.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/point-to-point/model/qbb-net-device.cc): This file is modified such that it can send and receive both RDMA and TCP/IP traffic. Note: only bulk-send-application and packet-sink should be used for TCP/IP traffic for correctness. Some examples on how to launch RDMA traffic can be found in [`simulator/ns-3.39/examples/PowerTCP`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/examples/PowerTCP) folder.

[`simulator/ns-3.39/src/point-to-point/model/rdma-hw.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/point-to-point/model/rdma-hw.cc): This file contains the core logic of PowerTCP and Theta-PowerTCP. The entire file is almost same as the one in HPCC simulator.

[`simulator/ns-3.39/src/point-to-point/model/switch-node.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/point-to-point/model/switch-node.cc): This file is also modified to support both RDMA and TCP/IP traffic. Minor change in INT, for PowerTCP, RXBytes is appended instead of TxBytes. This is since $\lambda(t)=\mu(t)+\dot{q}(t) $ i.e., RxRate is TxRate + Queue gradient where $\lambda(t)$ is required for PowerTCP to calculate power.

[`simulator/ns-3.39/src/point-to-point/model/switch-mmu.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/point-to-point/model/switch-mmu.cc): This file has the core logic for buffer management. Currently, we only support Dynamic Thresholds (DT) and Active Buffer Management (ABM). Note: The SIGCOMM version of ABM paper uses the implementation in traffic control layer (see below).

[`simulator/ns-3.39/src/traffic-control/model/gen-queue-disc.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/traffic-control/model/gen-queue-disc.cc): This file contains various buffer management algorithms at the traffic-control layer. It can only be used in the TCP/IP stack. Note: This is what we used for ABM in the paper.

[`simulator/ns-3.39/src/internet/model/tcp-advanced.cc`](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.39/src/internet/model/tcp-advanced.cc): This file contains various datacenter congestion control algorithms including PowerTCP implemented in the TCP/IP stack. This is what we used for ABM in the paper.

# Changes to NS-3.39

```diff
  Makefile (gone)                                    |    0
 README.md                                          |  178 +-
 config.sh (gone)                                   |   15 -
 simulator/ns-3.39/examples/ABM/README.md (new)     |    1 +
 .../examples/ABM/abm-evaluation-burst.cc (new)     |  874 ++++++++
 .../examples/ABM/abm-evaluation-multi.cc (new)     |  848 ++++++++
 .../examples/ABM/abm-evaluation-unimulti.cc (new)  |  905 ++++++++
 .../ns-3.39/examples/ABM/abm-evaluation.cc (new)   |  944 +++++++++
 simulator/ns-3.39/examples/ABM/alphas (new)        |    8 +
 simulator/ns-3.39/examples/ABM/cdf.c (new)         |  144 ++
 simulator/ns-3.39/examples/ABM/cdf.h (new)         |   43 +
 .../ns-3.39/examples/ABM/parseData-multiQ.py (new) |  101 +
 .../examples/ABM/parseData-singleQ.py (new)        |  125 ++
 simulator/ns-3.39/examples/ABM/results.sh (new +x) |  160 ++
 .../examples/ABM/run-buffer-loveland.sh (new +x)   |   91 +
 .../examples/ABM/run-interval-loveland.sh (new +x) |   99 +
 .../ABM/run-multiqueues-lakewood.sh (new +x)       |  137 ++
 .../examples/ABM/run-single-lakewood.sh (new +x)   |  164 ++
 .../examples/ABM/run-single-loveland.sh (new +x)   |  165 ++
 simulator/ns-3.39/examples/ABM/websearch.txt (new) |   16 +
 simulator/ns-3.39/examples/ABM/wscript (new)       |   15 +
 .../ns-3.39/examples/PowerTCP/README.md (new)      |   34 +
 .../examples/PowerTCP/buildsimple.sh (new +x)      |    5 +
 simulator/ns-3.39/examples/PowerTCP/cdf.c (new)    |  144 ++
 simulator/ns-3.39/examples/PowerTCP/cdf.h (new)    |   43 +
 .../examples/PowerTCP/config-burst.txt (new)       |   58 +
 .../examples/PowerTCP/config-fairness.txt (new)    |   58 +
 .../examples/PowerTCP/config-workload.txt (new)    |   58 +
 .../ns-3.39/examples/PowerTCP/config.sh (new)      |    1 +
 .../ns-3.39/examples/PowerTCP/config.txt (new)     |   58 +
 simulator/ns-3.39/examples/PowerTCP/fct.py (new)   |  493 +++++
 .../examples/PowerTCP/flow-burstExp.txt (new)      |   12 +
 .../examples/PowerTCP/flow-fairnessExp.txt (new)   |    5 +
 simulator/ns-3.39/examples/PowerTCP/flow.txt (new) |  211 ++
 .../examples/PowerTCP/generate_longflows.py (new)  |   20 +
 .../examples/PowerTCP/generate_topology.py (new)   |   55 +
 .../examples/PowerTCP/old-plots-all.py (new +x)    | 1054 ++++++++++
 .../ns-3.39/examples/PowerTCP/plot-burst.py (new)  |  112 +
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
 .../ns-3.39/examples/PowerTCP/set_cc.sh (new +x)   |    6 +
 .../ns-3.39/examples/PowerTCP/topology.txt (new)   |  146 ++
 .../ns-3.39/examples/PowerTCP/websearch.txt (new)  |   16 +
 simulator/ns-3.39/examples/PowerTCP/wscript (new)  |   12 +
 simulator/ns-3.39/examples/tcp/dctcp-example.cc    |    2 +
 .../helper/rdma-client-helper.cc (new)             |   65 +
 .../applications/helper/rdma-client-helper.h (new) |   79 +
 .../applications/model/bulk-send-application.cc    |   75 +-
 .../src/applications/model/bulk-send-application.h |   14 +
 .../ns-3.39/src/applications/model/packet-sink.cc  |   48 +
 .../ns-3.39/src/applications/model/packet-sink.h   |   12 +
 .../src/applications/model/rdma-client.cc (new)    |  157 ++
 .../src/applications/model/rdma-client.h (new)     |   87 +
 .../ns-3.39/src/applications/model/udp-client.cc   |    2 +-
 .../ns-3.39/src/applications/model/udp-server.cc   |    2 +-
 .../src/applications/model/udp-trace-client.cc     |    2 +-
 simulator/ns-3.39/src/applications/wscript         |   12 +-
 .../src/core/model/random-variable.cc (new)        | 2157 ++++++++++++++++++++
 .../ns-3.39/src/core/model/random-variable.h (new) |  755 +++++++
 simulator/ns-3.39/src/core/wscript                 |    2 +
 .../src/internet/model/ipv4-global-routing.cc      |  198 +-
 .../src/internet/model/ipv4-global-routing.h       |    5 +
 .../ns-3.39/src/internet/model/ipv4-l3-protocol.cc |    1 +
 simulator/ns-3.39/src/internet/model/rdma.h (new)  |    6 +
 .../src/internet/model/tcp-advanced.cc (new)       |  607 ++++++
 .../src/internet/model/tcp-advanced.h (new)        |  161 ++
 .../src/internet/model/tcp-congestion-ops.h        |   28 +
 .../ns-3.39/src/internet/model/tcp-l4-protocol.cc  |    3 +-
 .../ns-3.39/src/internet/model/tcp-socket-base.cc  |  169 +-
 .../ns-3.39/src/internet/model/tcp-socket-base.h   |   13 +
 .../ns-3.39/src/internet/model/tcp-socket-state.cc |   26 +
 .../ns-3.39/src/internet/model/tcp-socket-state.h  |   30 +
 simulator/ns-3.39/src/internet/wscript             |    3 +
 simulator/ns-3.39/src/network/model/buffer.cc      |    6 +
 simulator/ns-3.39/src/network/model/buffer.h       |    4 +
 simulator/ns-3.39/src/network/model/net-device.cc  |    9 +
 simulator/ns-3.39/src/network/model/net-device.h   |    6 +
 simulator/ns-3.39/src/network/model/node.cc        |   25 +-
 simulator/ns-3.39/src/network/model/node.h         |   25 +-
 simulator/ns-3.39/src/network/model/packet.cc      |    6 +
 simulator/ns-3.39/src/network/model/packet.h       |    5 +-
 .../network/utils/broadcom-egress-queue.cc (new)   |  262 +++
 .../network/utils/broadcom-egress-queue.h (new)    |   81 +
 .../src/network/utils/custom-header.cc (new)       |  344 ++++
 .../src/network/utils/custom-header.h (new)        |  146 ++
 .../src/network/utils/custom-priority-tag.cc (new) |   61 +
 .../src/network/utils/custom-priority-tag.h (new)  |   54 +
 simulator/ns-3.39/src/network/utils/data-rate.cc   |   51 +-
 simulator/ns-3.39/src/network/utils/data-rate.h    |   34 +-
 .../src/network/utils/feedback-tag.cc (new)        |   86 +
 .../ns-3.39/src/network/utils/feedback-tag.h (new) |   67 +
 .../ns-3.39/src/network/utils/int-header.cc (new)  |   94 +
 .../ns-3.39/src/network/utils/int-header.h (new)   |  119 ++
 .../src/network/utils/interface-tag.cc (new)       |  105 +
 .../src/network/utils/interface-tag.h (new)        |   47 +
 .../ns-3.39/src/network/utils/rdma-tag.cc (new)    |   62 +
 .../ns-3.39/src/network/utils/rdma-tag.h (new)     |   49 +
 .../model => network/utils}/seq-ts-header.cc       |   51 +-
 .../model => network/utils}/seq-ts-header.h        |   38 +-
 .../ns-3.39/src/network/utils/unsched-tag.cc (new) |   62 +
 .../ns-3.39/src/network/utils/unsched-tag.h (new)  |   49 +
 simulator/ns-3.39/src/network/wscript              |   18 +
 .../src/point-to-point/helper/qbb-helper.cc (new)  |  453 ++++
 .../src/point-to-point/helper/qbb-helper.h (new)   |  202 ++
 .../src/point-to-point/helper/sim-setting.h (new)  |   53 +
 .../src/point-to-point/model/cn-header.cc (new)    |  177 ++
 .../src/point-to-point/model/cn-header.h (new)     |  102 +
 .../src/point-to-point/model/pause-header.cc (new) |  114 ++
 .../src/point-to-point/model/pause-header.h (new)  |   77 +
 .../ns-3.39/src/point-to-point/model/pint.cc (new) |   44 +
 .../ns-3.39/src/point-to-point/model/pint.h (new)  |   19 +
 .../model/point-to-point-net-device.cc             |    5 +
 .../model/point-to-point-net-device.h              |    8 +-
 .../ns-3.39/src/point-to-point/model/ppp-header.cc |    5 +-
 .../ns-3.39/src/point-to-point/model/ppp-header.h  |   20 +-
 .../src/point-to-point/model/qbb-channel.cc (new) 

```
