###############################################################################
# Copyright (c) 2019 Mishal Shah
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
# Authors: Mishal Shah  <shahmishal1998@gmail.com>
#
###############################################################################
import unittest
import sys
import os
from bake.ModuleLogger import StdoutModuleLogger
from bake.ModuleAppStore import BaseClient
from bake.Constants import *

class TestModuleAppStore(unittest.TestCase):
    """Tests cases for the Module AppStore Class."""
    def setUp(self):
        self._logger = StdoutModuleLogger();
        self._logger.set_verbose(1)


    def test_search_api(self):
        # first setup the baseclient class otherwise will not be
        # able to make requests
        webclient = BaseClient(self._logger, SEARCH_API, baseurl='http://localhost:8000')
        self.assertNotEqual(webclient, None)

        # Test for when substring is not present
        testResult = None
        try:
            testResult = webclient.search_api('random')
            self.fail('There was no problem as the app was not found')
        except:
            self.assertEqual(testResult, None)

        # Test for when substring is present
        testResult = webclient.search_api('wave')
        self.assertEqual(testResult[0]['app']['name'], 'mmwave')

        # Test for when substring is present
        testResult = webclient.search_api('wave')
        self.assertNotEqual(testResult, None)

        # Test for when no substring is passed
        testResult = webclient.search_api()
        self.assertNotEqual(testResult, None) 


    def test_install_api(self):
        
        # first setup the baseclient class otherwise will not be
        # able to make requests
        webclient = BaseClient(self._logger, INSTALL_API, baseurl='http://localhost:8000')
        self.assertNotEqual(webclient, None)

        # Test for when app is not present
        testResult = None
        try:
            testResult = webclient.install_api('random-app')
            self.fail('There was no problem as the app was not found')
        except:
            self.assertEqual(testResult, None)

        # Test for when app is present on the AppStore
        testResult = webclient.install_api('epidemic-routing')
        self.assertEqual(testResult[0]['name'], 'epidemic-routing')
        self.assertEqual(testResult[1].status_code, 200)

        # Test for when app is present on the AppStore with specific version
        testResult = webclient.install_api('ns3-gym', '0.1')
        self.assertEqual(testResult[0]['name'], 'ns3-gym')
        self.assertEqual(testResult[1].status_code, 200)


if __name__ == '__main__':
    unittest.main()
