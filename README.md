



# Build and Run

**Important:** Run `source config.sh` in your terminal before doing anything. You can also add `source /path/to/repo/config.sh` at the end of `~/.bashrc` to make things easier.

**Configure ns3:**

```bash
cd $NS3DC; CXXFLAGS=-w ./waf configure --build-profile=optimized --disable-example --disable-tests --disable-python
```
**Build and run ns3:**

```bash
cd $NS3DC; ./waf
```