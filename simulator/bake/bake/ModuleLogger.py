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
 ModuleLogger.py
 
 This file stores the logger implementation. The logger is responsible for  
 logging the messages and control the level of information showed to the 
 user.
''' 

import sys
import os
from bake.Exceptions import NotImplemented

class ModuleLogger:
    """ The logger class. The logger is responsible for logging the messages 
    and control the level of information showed to the user.
    """
    sendToFile=None
    
    def __init__(self):
        """ Initializes the used variables."""

        self._verbose = None
        self._command_file = open(os.devnull, 'w')
        self._std_file = None
        self._dump_file = None

    def _update_file(self, f):
        """ Opens/changes the output devices accordingly to the verbose level."""

        if self._verbose == 0:
            self._command_file = open(os.devnull, 'w')
            self._std_file = open(os.devnull, 'w')
        elif self._verbose == 1:
            self._command_file = f
            self._std_file = open(os.devnull, 'w')
        elif self._verbose == 2:
            self._command_file = f
            self._std_file = f
            
        if self.sendToFile :
            self._dump_file = open(self.sendToFile, 'w')
            
    def set_verbose(self, verbose):
        """ Sets verbosity level."""
        
        self._verbose = verbose if verbose <= 2 else 2
        
    # empty implementations, to be re-implemented by the descendants 
    def set_current_module(self, name):
        raise NotImplemented()
    def clear_current_module(self):
        raise NotImplemented()
    
    @property
    def stdout(self):
        """ Returns the in use standard out put."""
        
        return self._std_file
    @property
    def stderr(self):
        """ Returns the in use  error out put."""
        
        return self._std_file
    @property
    def commands(self):
        return self._command_file


class StdoutModuleLogger(ModuleLogger):
    """ The Standard output logger, where all the outputs go to the stdout."""

    def __init__(self):
        """ Initializes the used variables."""

        ModuleLogger.__init__(self)
        self._file=sys.__stdout__
        self._update_file(self._file)
        
    def set_current_module(self, name):
        """ Sets stdout as the output as the output for the module."""

        self._update_file(sys.__stdout__)
        
    def clear_current_module(self):
        """ Not implemented, as the output is always the same, one does 
        not need to reset it.
        """
        pass

class LogfileModuleLogger(ModuleLogger):
    """ The file output logger, all the outputs go to the same log file."""
    
    def __init__(self, filename):
        """ Initializes the used variables."""

        ModuleLogger.__init__(self)
        self._file = open(filename, 'w')
        
    def set_current_module(self, name):
        """ Sets the file the output as the output for the module."""
        
        self._update_file(self._file)
        
    def clear_current_module(self):
        """ Not implemented, as the output is always the same, one does 
        not need to reset it.
        """
        pass

class LogdirModuleLogger(ModuleLogger):
    """ Logs the output for a repository,  i.e. one log file per module."""
    
    def __init__(self, dirname):
        """ Initializes the used variables."""

        if not os.path.isdir(dirname):
            os.mkdir(dirname)
        self._dirname = dirname
        self._file = None
        
    def set_current_module(self, name):
        """ Sets the output file in accordance to the module in use, so that 
        the log outputs will be separated by module.
        """
 
        # XXX: we should be checking for other reserved characters
        import re
        filename = re.sub('/', '_', name)
        assert self._file is None
        self._file = open(os.path.join(self._dirname, filename + '.log'), 'w')
        self._update_file(self._file)
        
    def clear_current_module(self):
        """ Cleans the output for the next module."""

        self._file.close()
        self._file = None

