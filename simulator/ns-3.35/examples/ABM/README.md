# ABM Simulations

## Config

Edit [config.sh](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.35/examples/ABM/config.sh) to match your CPU. It is optimal to set `N_CORES` to a value lower than the number of CPU cores on your system. [run-main.sh](https://github.com/inet-tub/ns3-datacenter/tree/master/simulator/ns-3.35/examples/ABM/run-main.sh) launches a total of `87` simulations, with `N_CORES` number of simulations running in parallel. The script enters a waiting mode when `N_CORES` simulations are running, until a simulation finishes and then launches a new simulation. Each simulation takes about 9 to 10 hours and this duration may also depend on your CPU.


## Simulations from the paper

Running the following four scripts essentially produces the results for all the simulations in the paper.

- [run-main.sh](./run-main.sh)
- [run-buffer.sh](./run-buffer.sh)
- [run-interval.sh](./run-interval.sh)
- [run-nprio.sh](./run-nprio.sh)

The simulation data is then written to `dump_sigcomm/` folder. To parse the results, run `results.sh > ./plots/results-all.dat`. This produces a datafile written to plots folder with all the metrics needed for plots. Run `python3 ./plots_sigcomm.py` from plots folder to generate plots. For convinience, please use [plots-rename.sh](./plots-rename.sh) to rename the plots with corresponding figure numbers in the paper.