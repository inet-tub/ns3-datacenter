#!/usr/bin/env python3

###############################################################################
# Copyright (c) 2013 INRIA
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Authors: Daniel Camara  <daniel.camara@inria.fr>
#          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
###############################################################################
# bake.py
#
# Bake executable, in fact it is just a wrapper for the main Bake class.  

import sys, os
import bake

try:
    dir = oridir = os.getcwd()

    while True:
        script = os.path.join(dir, "bake.py")
        if os.path.isfile(script):
            if dir == oridir:
                break
            os.chdir(dir)
            args = ' '.join(sys.argv[1:])
            os.system("python bake.py " + args)
            exit()
        else:
            tmp = os.path.dirname(dir)
            if tmp == dir:
                break
            dir = tmp
    
    b = bake.Bake()
    b.main(sys.argv)
except SystemExit as e: 
    sys.exit(e)
except: 
    print("  > Unexpected exception!\n" 
          "    Please register the error at https://www.nsnam.org/bugzilla, \n"
          "    with a copy of the trace below and, if possible, a list of steps to reproduce the error!<")
    sys.stdout.flush()
    bake.Utils.print_backtrace()
