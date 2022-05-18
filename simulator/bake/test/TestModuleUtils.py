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
import unittest
import bake.Utils
from bake.Exceptions import TaskError


class TestModuleUtils(unittest.TestCase):
    """Tests cases for the Utils Class."""
    
    def setUp(self):
        """Common set Up environment, available for all tests."""
        
    def tearDown(self):
        """Cleans the environment environment for the next tests."""

    def test_print_backtrace(self):
        """Tests the _print_backtrace method from Util. """

        testResult = bake.Utils.print_backtrace()
        self.assertTrue(testResult)
        
        try:
            raise TaskError('Process test failure')
        except TaskError as e:
            testResult = bake.Utils.print_backtrace()
            self.assertTrue('Process test failure' in testResult)


   
    def test_split_args(self):
        """Tests the split_args method of Class Utils. """
        
        string = "CCOPTS='-fpic -D_GNU_SOURCE -O0 -U_FORTIFY_SOURCE'"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==1)
        self.assertEqual(testResult[0], 
                         "CCOPTS='-fpic -D_GNU_SOURCE -O0 -U_FORTIFY_SOURCE'")
        
        string = "CCOPTS='-fpic -D_GNU_SOURCE -O0 -U_FORTIFY_SOURCE' -g"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==2)
        self.assertEqual(testResult[0], 
                         "CCOPTS='-fpic -D_GNU_SOURCE -O0 -U_FORTIFY_SOURCE'")
        self.assertEqual(testResult[1], "-g")

        string = "CCOPTS=-fpic -D_GNU_SOURCE -O0 -U_FORTIFY_SOURCE -g"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==5)
        self.assertEqual(testResult[0], "CCOPTS=-fpic")
        self.assertEqual(testResult[1], "-D_GNU_SOURCE")
        self.assertEqual(testResult[4], "-g")

        string = ""
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==0)
        
        string = "install PREFIX=`pwd`/../build"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==2)
        self.assertEqual(testResult[0], "install")
        self.assertEqual(testResult[1], "PREFIX=`pwd`/../build")
        
        string = "install PREFIX='pwd'/../build -g"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==3)
        self.assertEqual(testResult[0], "install")
        self.assertEqual(testResult[1], "PREFIX='pwd'/../build")

        string = "'install PREFIX='pwd'/../build' -g"
        testResult = bake.Utils.split_args(string)
        self.assertTrue(len(testResult)==2)
        self.assertEqual(testResult[0], "'install PREFIX='pwd'/../build'")
        self.assertEqual(testResult[1], "-g")
        

# main call for the tests        
if __name__ == '__main__':
    unittest.main()
