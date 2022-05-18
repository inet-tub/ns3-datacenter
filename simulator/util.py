from __future__ import print_function
import sys
import subprocess

def fatal(msg):
    print(msg, file=sys.stderr)
    sys.exit(2)

class CommandError(Exception):
    pass

def run_command(*args, **kwargs):
    if len(args):
        argv = args[0]
    elif 'args' in kwargs:
        argv = kwargs['args']
    else:
        argv = None
    if argv is not None:
        print(" => ", ' '.join(argv))

    cmd = subprocess.Popen(*args, **kwargs)
    retval = cmd.wait()
    if retval:
        raise CommandError("Command %r exited with code %i" % (argv, retval))
