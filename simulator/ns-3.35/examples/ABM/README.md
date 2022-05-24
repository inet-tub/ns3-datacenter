# Running ABM Simulations

Please consider this as a beta release. From local working directory to git, there were some naming changes where old names are still stuck here. Please raise an issue if you see "wien" anywhere or the program crashes with "unknown attritube" error. Full cleanup and a release can be expected in the upcoming weeks.

# Simulations from the paper

Running the following four scripts essentially produces the results for all the simulations in the paper.

- `run-main.sh`
- `run-buffer.sh`
- `run-interval.sh`
- `run-nprio.sh`

The simulation data is then, written to `dump_sigcomm/` folder. To parse the results, run `results.sh`. This produces a data file with all the metrics needed for plots. More details will be updated soon.