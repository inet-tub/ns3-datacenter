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
'''
BakeTestSuite.py

Calls all the available bake tests in sequence.
'''

import glob
import unittest

# finds the test files, should start with Test and finish with .py
test_file_strings = glob.glob('Test*.py')

#test_file_strings = ["TestModuleSource.py"]
# puts the file in the format of modules to be imported
module_strings = ["test."+str[0:len(str)-3] for str in test_file_strings]

# search for the tests on the modules
suites = [unittest.defaultTestLoader.loadTestsFromName(str) for str 
          in module_strings]

# adds the tests on the suite to be run
testSuite = unittest.TestSuite(suites)

# runs the full tests
text_runner = unittest.TextTestRunner().run(testSuite)
