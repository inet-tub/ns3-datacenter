# CXXFLAGS="-w -Wall -std=c++11 -shared -fPIC $(python3 -m pybind11 --includes) $(python3-config --ldflags) -I /home/vamsi/.local/include -L/usr/lib/x86_64-linux-gnu/libpython3.11.so "
CXXFLAGS="-w" ./waf configure --build-profile=release --enable-examples --disable-tests --disable-python
