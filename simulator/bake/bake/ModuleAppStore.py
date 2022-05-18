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
''' 
 ModuleAppStore.py
 
 This file stores the class Module AppStore responsible for the interaction
 between Bake and the ns-3-AppStore.
''' 
import requests
import sys


class BaseClient:
    ''' Main class to interact with the ns-3-AppStore
    '''
    BASE_URL = "https://apps.nsnam.org"

    def __init__(self, logger, api, baseurl=BASE_URL, debug=False):
        ''' Internal variables initialization.'''
        self._logger = logger
        self._api = api
        self._baseurl = baseurl
        self._debug = debug

    def _error(self, string):
        ''' Handles the exceptions '''
        raise Exception(string)

    def search_api(self, params=None, ns_enabled=None):
        ''' Handles the search function API '''
        url = self._baseurl + self._api
        if params and ns_enabled:
            data = {}
            data["q"] = str(params)
            data["ns"] = str(ns_enabled)
            res = requests.post(url=url, data=data)
        elif params:
            data = {}
            data["q"] = str(params)
            res = requests.post(url=url, data=data)
        else:
            res = requests.post(url=url)

        if res.status_code == 200:
            return res.json()
        else:
            self._error("No apps found")
            sys.exit(1)

    def install_api(self, module_name=None, version=None, ns=None):
        ''' Handles the getconf, install function API '''
        url = self._baseurl + self._api
        data = {}
        data["module_name"] = str(module_name)
        if version is not None:
            data["version"] = str(version)

        if ns is not None:
            data["ns"] = str(ns)

        res = requests.post(url=url, data=data)
        if res.status_code == 200:
            # Case (i)  : app_type = Fork, without bakefile
            # Case (ii) : app_type = Fork, with bakefile
            # Case (iii): app_type = Module, without bakefile
            # Case (iv) : app_type = Module, with bakefile
            resp = res.json()
            if ((resp['app_type'] == 'F' and resp['bakefile_url'] == "/media/") or resp['app_type'] == 'M' and resp['bakefile_url'] == "/media/"):
                self._error("Bakefile for the Module/App was not found on the AppStore\n")
                sys.exit(1)
            else:
                # Read the object file (bakeconf)
                res_obj = requests.get(self._baseurl + resp['bakefile_url'])
                return resp, res_obj
        elif len(data)==0:
            self._error("Module/App not compatible with the configured ns version\n")
            sys.exit(1)
        else:
            self._error("Module/App not found on the AppStore\n")
            sys.exit(1)
