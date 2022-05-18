#!/bin/bash

# Please make sure that REPO is the root directory of this repository.
# Everything else depends on REPO.

# path to the repository
export DCREPO=/home/vamsi/src/phd/ns3-datacenter

# path to simulator
export DCSIM=$DCREPO/simulator

#path to ns3
export NS3DC=$DCSIM/ns-3.35

alias ns3-dc="$NS3DC/ns3"
