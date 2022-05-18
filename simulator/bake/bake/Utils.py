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
 Utils.py
 
 This file stores the utility functions that are used by the different Bake
 modules.
''' 

import subprocess
import os
import shutil
import sys
from xml.etree import ElementTree
from xml.dom import minidom
from bake.Exceptions import TaskError

def print_backtrace():
    """ Prints the full trace of the exception."""
     
    import sys
    import traceback
    trace = ""
    exception = ""
    
    exceptionHandling = True
    if(not sys.exc_info()[0] or not sys.exc_info()[1]):
        exceptionHandling = False
        
    if exceptionHandling: 
        exc_list = traceback.format_exception_only (sys.exc_info()[0],sys.exc_info()[1])

        for entry in exc_list:
            exception += entry
    
        tb_list = traceback.format_tb(sys.exc_info()[2])
    else:
        tb_list = traceback.format_stack()
        
    for entry in tb_list:
        trace += entry

    toWrite = "\n%s\n%s" % (exception, trace)
    sys.stderr.write(toWrite)
    return toWrite

def split_args(stringP):
    """ Split arguments respecting aggregate strings."""
    
    returnValue = []
    rawSplit = stringP.split()
    compensateElement=False
    elementStr = ''
    for element in rawSplit:
        if "'" in element :
            if element.count("'") % 2 != 0 :
                if compensateElement :
                    compensateElement = False
                    returnValue.append(elementStr + " " + str(element))
                    elementStr = ''
                    element = None
                elif element.find("'") == element.rfind("'") :
                    compensateElement = True
            
        if compensateElement :
            if len(elementStr) > 0 :
                elementStr = elementStr + " " + element
            else :
                elementStr = element 
        else : 
            if element :
                returnValue.append(element)
    
    return returnValue

def prettify(elem):
    """ Returns a pretty-printed XML string for the Element.
    """
    rough_string = ElementTree.tostring(elem, 'utf-8')
    reparsed = minidom.parseString(rough_string)
    string = reparsed.toprettyxml(indent="  ")
    new_string=''
    for line in string.split('\n'):
        if line.strip():
                new_string += line + '\n'

    return new_string

def mergeDirs(sourcePath, destPath):
    """ Merge two folders, creating the structure and copying the files from
        source to destination, when these are missing, and skipping files when
        these are already present on the destination. Pay attention that what
        this function does is a merge of directories contents not of file 
        content.
    """
    
    for root, dirs, files in os.walk(sourcePath):

        #figure out where we're going
        dest = destPath + root.replace(sourcePath, '')

        #if we're in a directory that doesn't exist in the destination folder
        #then create a new folder
        if not os.path.isdir(dest):
            os.mkdir(dest)
#            print 'Directory created at: ' + dest

        #loop through all files in the directory
        for f in files:
            #compute current (old) & new file locations
            oldLoc = root + '/' + f
            newLoc = dest + '/' + f

            if not os.path.isfile(newLoc):
                try:
                    shutil.copy2(oldLoc, newLoc)
#                    print 'File ' + f + ' copied.'
                except IOError:
#                    print 'file "' + f + '" already exists'
                    pass


class ModuleAttribute:
    """ Definition of the Bake attribute. An attribute is basically one of the 
    options the user can have to configure the Bake usage.
    """

    def __init__(self, name, value, help, mandatory):
        """ Initialization, all the fields are mandatory."""
        
        self._name = name
        self.value = value
        self._help = help
        self._mandatory = mandatory
        
    @property
    def name(self):
        """ Returns the stored name of the attribute."""
        
        return self._name
    
    @property
    def help(self):
        """ Returns the help string attached to the attribute."""
        return self._help
    
    @property
    def is_mandatory(self):
        """ Returns if the attribute is mandatory or not."""
        return self._mandatory


class ModuleAttributeBase(object):
    """ Definition of the Bake attribute structure. An attribute may be 
    organized in blocks, this structure stores this grouping of attributes.
    """
    
    def __init__(self):
        self._attributes = dict()
        self._children = []
        
    def children(self):
        """ Returns the children attributes attached to this attribute."""
        
        return self._children
    
    def add_child(self, child, name):
        """ Attach a child attribute to this attribute."""
        
        self._children.append([child, name])
        
    def add_attribute(self, name, value, help, mandatory = False):
        """ Creates a new attribute attached to this one."""
       
        assert not name in self._attributes
        self._attributes[name] = ModuleAttribute(name, value, help, mandatory)
        
    def attributes(self):
        """ Returns the list of attributes attached to this attribute block."""
        
        return self._attributes.values()
    
    def attribute(self, name):
        """ Returns a specific attribute."""
        
        if not name in self._attributes:
            return None
        else:
            return self._attributes[name]

class ColorTool:
    """ Class responsible to handle the colored message printing."""
        
    OK = '\033[34m'
    WARNING = '\033[33m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    @classmethod
    def has_colours(self, stream):
        if not hasattr(stream, "isatty"):
            return False
        if not stream.isatty():
            return False # auto color only on TTYs
        try:
            import curses
            curses.setupterm()
            return curses.tigetnum("colors") > 2
        except:
            # guess false in case of error
            return False
    
    @classmethod
    def disable(self):
        """ Disables the color print. """
        
        ColorTool.OK = ''
        ColorTool.WARNING = ''
        ColorTool.FAIL = ''
        ColorTool.ENDC = ''
        
    @classmethod
    def cPrint(self, color, message):
        """ Print the message with the defined color. """
        
        sys.stdout.write(color + message + self.ENDC)
        sys.stdout.flush()
        
    @classmethod    
    def cPrintln(self,color, message):
        """ Print the message with the defined color and ends with a new line. """
        
        self.cPrint(color, message + os.linesep)
        
        

