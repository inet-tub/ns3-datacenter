
import sys
import os
from xml.dom.minidom import getDOMImplementation
from timeit import Timer
import subprocess

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                '..', 'build', 'default', 'benchmarks'))


TIMES = 10000000
TIMES1 = TIMES/4

import testapi_pybindgen
import testapi_boost
import testapi_swig
import testapi_sip


def bench(mod, dom, elem):
    def bench():
        return mod.func1()
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call function with no arguments")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))
    
    def bench():
        return mod.func2(1.0, 2.0, 3.0)
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call function taking 3 doubles")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    def bench():
        return mod.Multiplier()
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call class constructor with no arguments")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    def bench():
        return mod.Multiplier(3.0)
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call class constructor with double")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    obj = mod.Multiplier(3.0)
    def bench():
        return obj.GetFactor()
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call simple method")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    obj = mod.Multiplier(3.0)
    def bench():
        return obj.SetFactor()
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call overloaded method 1")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    obj = mod.Multiplier(3.0)
    def bench():
        return obj.SetFactor(1.0)
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call overloaded method 2")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    obj = mod.Multiplier(3.0)
    def bench():
        return obj.Multiply(5.0)
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call non-overridden virtual method with double")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    class M(mod.Multiplier):
        def Multiply(self, value):
            return super(M, self).Multiply(value)
    obj = M(2.0)
    def bench():
        return obj.Multiply(5.0)

    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call python-overridden virtual method from Python")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES1)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))

    def bench():
        return mod.call_virtual_from_cpp(obj, 5.0)
    tst = elem.appendChild(dom.createElement('test'))
    tst.setAttribute("description", "call python-overridden virtual method from C++")
    tst.setAttribute("time", repr(Timer(bench).timeit(TIMES1)))
    print "%s (%s): %s" % (tst.tagName, tst.getAttribute('description'), tst.getAttribute('time'))


def main():
    impl = getDOMImplementation()
    dom = impl.createDocument(None, "pybindgen-benchmarks", None)
    top = dom.documentElement

    env = top.appendChild(dom.createElement('environment'))
    env.appendChild(dom.createElement('compiler')).appendChild(dom.createTextNode(
            subprocess.Popen(["g++", "-v"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0]))
    env.appendChild(dom.createElement('python')).appendChild(dom.createTextNode(
            sys.version))
    env.appendChild(dom.createElement('swig')).appendChild(dom.createTextNode(
            subprocess.Popen(["swig", "-version"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0]))
    env.appendChild(dom.createElement('boost_python')).appendChild(dom.createTextNode(
            subprocess.Popen(["dpkg", "-s", "libboost-python-dev"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0]))
    env.appendChild(dom.createElement('pybindgen')).appendChild(dom.createTextNode(
            subprocess.Popen(["bzr", "version-info", '--check-clean'],
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT).communicate()[0]))

    env.appendChild(dom.createElement('cpu')).appendChild(dom.createTextNode(
            file("/proc/cpuinfo").read()))

    if len(sys.argv) == 3:
        env.appendChild(dom.createElement('CXXFLAGS')).appendChild(dom.createTextNode(sys.argv[2]))


    res = top.appendChild(dom.createElement('results'))


    print "pybindgen results:"
    pbg = res.appendChild(dom.createElement('pybindgen'))
    pbg.setAttribute("module-file-size", repr(os.stat("build/default/benchmarks/testapi_pybindgen.so").st_size))
    bench(testapi_pybindgen, dom, pbg)

    print "boost_python results:"
    bp = res.appendChild(dom.createElement('boost_python'))
    bp.setAttribute("module-file-size", repr(os.stat("build/default/benchmarks/testapi_boost.so").st_size))
    bench(testapi_boost, dom, bp)

    print "swig results:"
    sw = res.appendChild(dom.createElement('swig'))
    sw.setAttribute("module-file-size", repr(os.stat("build/default/benchmarks/_testapi_swig.so").st_size))
    sw.setAttribute("module-python-file-size", repr(os.stat("build/default/benchmarks/testapi_swig.py").st_size))
    bench(testapi_swig, dom, sw)

    print "sip results:"
    sip = res.appendChild(dom.createElement('sip'))
    sip.setAttribute("module-file-size", repr(os.stat("build/default/benchmarks/testapi_sip.so").st_size))
    bench(testapi_sip, dom, sip)

    if len(sys.argv) == 3:
        f = open(sys.argv[1], "wb")
        dom.writexml(f, "", "  ", "\n")
        f.close()



if __name__ == '__main__':
    main()
