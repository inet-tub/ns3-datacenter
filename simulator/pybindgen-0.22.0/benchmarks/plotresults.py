
import pylab
from xml.dom.minidom import parse, Node
import sys
import numpy as np
import shutil
import os

DPI = 75

def getText(nodelist):
    rc = ""
    for node in nodelist:
        if node.nodeType == node.TEXT_NODE:
            rc = rc + node.data
    return rc

def main(argv):
    input_fname = sys.argv[1]
    dom = parse(input_fname)
    outdir = sys.argv[2]
    res = dom.getElementsByTagName("results")[0]
    tools = [e for e in res.childNodes if e.nodeType == Node.ELEMENT_NODE]

    # get the pybindgen revno
    pbg_env = dom.getElementsByTagName("environment")[0].getElementsByTagName("pybindgen")[0]
    pbg_txt = getText(pbg_env.childNodes).split("\n")
    for l in pbg_txt:
        k, s, v = l.partition(':')
        k = k.strip()
        if k == 'revno':
            v = v.strip()
            revno = v
            break

    num_tests = len(tools[0].getElementsByTagName("test"))
    shutil.rmtree(outdir, True)
    os.mkdir(outdir)
    shutil.copy2(input_fname, outdir)

    figures = []


    sizes = [float(t.getAttribute('module-file-size')) for t in tools]
    labels= [t.tagName for t in tools]
    ind = range(len(sizes))    
    pylab.bar(ind, sizes)
    pylab.xticks([0.5+x for x in ind], labels)
    pylab.title("Extension module file size (B)")
    fname = "sizes.png"
    pylab.savefig(os.path.join(outdir, fname), dpi=DPI)
    figures.append(fname)
    
    
    for t in range(num_tests):
        pylab.figure()
        labels = []
        values = []
        for x, tool in enumerate(tools):
            labels.append(tool.tagName)
            values.append(float(tool.getElementsByTagName("test")[t].getAttribute('time')))
        ind = range(len(values))
        pylab.bar(ind, values)
        pylab.xticks([0.5+x for x in ind], labels)

        desc = tools[0].getElementsByTagName("test")[t].getAttribute("description")
        pylab.title(desc)
        
        fname = "%s.png" % (desc)
        pylab.savefig(os.path.join(outdir, fname), dpi=DPI)
        figures.append(fname)

    index_html = file("%s/index.html" % outdir, "wt")
    print >> index_html, """
<html>
<head>
<title> PyBindGen Benchmarks </title>
</head>

<body>

  <div>Details in the <a href=\"%s\">Raw XML file</a>.
  <a href=\"http://bazaar.launchpad.net/~gjc/pybindgen/trunk/files/%s/benchmarks/\">Source files for the benchmarks</a>.
  </div>

""" % (os.path.basename(input_fname), revno)

    for fig in figures:
        print >> index_html, """
  <div>
    <img src=\"%s\"/>
  </div>
""" % (fig,)

    print >> index_html, """
</body>
"""
    
    index_html.close()
    
    

if __name__ == '__main__':
    main(sys.argv)

