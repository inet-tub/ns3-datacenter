#! /usr/bin/env python3
import sys
from optparse import OptionParser
import os
from xml.dom import minidom as dom
import shlex

import constants
from util import run_command, fatal, CommandError


def build_netanim(qmakepath):
    qmake = 'qmake'
    qmakeFound = False
    try:
        run_command([qmake, '-v'])
        print("qmake found")
        qmakeFound = True
    except:
        print("Could not find qmake in the default path")

    try:
        if qmakeFound == False:
                run_command(['qmake-qt5', '-v'])
                qmake = 'qmake-qt5'
                print("qmake-qt5 found")
    except:
        print("Could not find qmake-qt5 in the default path")
        
    if qmakepath:
        print("Setting qmake to user provided path")
        qmake = qmakepath
    try:    
        run_command([qmake, 'NetAnim.pro'])
        run_command(['make'])
    except OSError:
        print("Error building NetAnim. Ensure the path to qmake is correct.")
        print("Could not find qmake or qmake-qt5 in the default PATH.")
        print("Use ./build.py --qmake-path <Path-to-qmake>, if qmake is installed in a non-standard location")
        print("Note: Some systems use qmake-qt5 instead of qmake")
        print("Skipping NetAnim ....")
        pass
    except:
        print("Error building NetAnim.")
        print("Skipping NetAnim ....")
        pass

def build_ns3(config, build_examples, build_tests, args, build_options):
    cmd = [sys.executable, "ns3", "configure"] + args

    if build_examples:
        cmd.append("--enable-examples")

    if build_tests:
        cmd.append("--enable-tests")

    run_command(cmd) # ns3 configure ...
    run_command([sys.executable, "ns3", "build"] + build_options)


def main(argv):
    parser = OptionParser()
    parser.add_option('--disable-netanim',
                      help=("Don't try to build NetAnim (built by default)"), action="store_true", default=False,
                      dest='disable_netanim')
    parser.add_option('--qmake-path',
                      help=("Provide absolute path to qmake executable for NetAnim"), action="store",
                      dest='qmake_path')
    parser.add_option('--enable-examples',
                      help=("Do try to build examples (not built by default)"), action="store_true", default=False,
                      dest='enable_examples')
    parser.add_option('--enable-tests',
                      help=("Do try to build tests (not built by default)"), action="store_true", default=False,
                      dest='enable_tests')
    parser.add_option('--build-options',
                      help=("Add these options to ns-3's \"ns3 build\" command"),
                      default='', dest='build_options')
    (options, args) = parser.parse_args()

    cwd = os.getcwd()

    try:
        dot_config = open(".config", "rt")
    except IOError:
        print("** ERROR: missing .config file; you probably need to run the download.py script first.", file=sys.stderr)
        sys.exit(2)

    config = dom.parse(dot_config)
    dot_config.close()

    if options.disable_netanim:
        print("# Skip NetAnimC (by user request)")
        for node in config.getElementsByTagName("netanim"):
            config.documentElement.removeChild(node)
    elif sys.platform in ['cygwin', 'win32']:
        print("# Skip NetAnim (platform not supported)")
    else:
        netanim_config_elems = config.getElementsByTagName("netanim")
        if netanim_config_elems:
            netanim_config, = netanim_config_elems
            netanim_dir = netanim_config.getAttribute("dir")
            print("# Build NetAnim")
            os.chdir(netanim_dir)
            print("Entering directory `%s'" % netanim_dir)
            try:
                try:
                    build_netanim(options.qmake_path)
                except CommandError:
                    print("# Build NetAnim: failure (ignoring NetAnim)")
                    config.documentElement.removeChild(netanim_config)
            finally:
                os.chdir(cwd)
            print("Leaving directory `%s'" % netanim_dir)

    if options.enable_examples:
        print("# Building examples (by user request)")
        build_examples = True
    else:
        build_examples = False

    if options.enable_tests:
        print("# Building tests (by user request)")
        build_tests = True
    else:
        build_tests = False

    if options.build_options is None:
        build_options = None
    else:
        build_options = shlex.split(options.build_options)

    print("# Build NS-3")
    ns3_config, = config.getElementsByTagName("ns-3")
    d = os.path.join(os.path.dirname(__file__), ns3_config.getAttribute("dir"))
    print("Entering directory `%s'" % d)
    os.chdir(d)
    try:
        build_ns3(config, build_examples, build_tests, args, build_options)
    finally:
        os.chdir(cwd)
    print("Leaving directory `%s'" % d)


    return 0

if __name__ == '__main__':
    sys.exit(main(sys.argv))
