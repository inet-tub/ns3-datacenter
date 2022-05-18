import sys
sys.path.insert(0, "../../build/examples/buffer")
import c

print c.GetBufferLen()
print c.GetBufferChecksum()
buf = c.GetBuffer()
buf[10] = chr(123)
print c.GetBufferChecksum()
print buf[10]
