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
 Exeptions.py

 This file stores the Exceptions raised by Bake.
''' 

class TaskError(Exception):
    """ Error found during the execution of the required options. """
    
    def __init__(self, reason = ''):
        self._reason = reason
        return
    @property
    def reason(self):
        return self._reason

class MetadataError(Exception):
    """ Error reading the configuration. """
    
    def __init__(self, reason = ''):
        self._reason = reason
        return
    def reason(self):
        return self._reason

class NotImplemented(Exception):
    """ A not yet implemented option was met. """
    
    def __init__(self):
        pass
