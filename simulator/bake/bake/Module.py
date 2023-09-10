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
 Module.py

 This file stores the generic implementation of the bake options. e.g. how 
 the download works, independently of the technology/repository used to  
 store the code.
''' 

import copy
import os
import re
import sys
import shutil

from bake.FilesystemMonitor import FilesystemMonitor
from bake.Exceptions import TaskError
from bake.Utils import ColorTool
from bake.Exceptions import NotImplemented
from bake.ModuleSource import SystemDependency
from bake.Utils import ModuleAttributeBase

class ModuleDependency(ModuleAttributeBase):
    """ Dependency information. """
    instances = []
    def __init__(self, name = '' , optional = False):

        ModuleAttributeBase.__init__(self)

        self.add_attribute('name', name, 'Name of the Module', mandatory=True)
        self.add_attribute('optional', str(optional), 'Whether module is optional', mandatory=True)        
        
        self.__class__.instances.append(self)        
        
    @classmethod
    def subclasses(self):
        return ModuleDependency.__subclasses__()

    def name(self):
        return self._name

    @classmethod
    def name(cls):
        return 'default'
        
    @property
    def _name(self):
        return self.attribute('name').value

    @property
    def _optional(self):
        return self.is_optional()
    
    @classmethod
    def create(cls, build_type):                
        for subclass in ModuleDependency.subclasses():
            if subclass.name() == build_type:
                instance = subclass()
                return instance
        
        return ModuleDependency()

    def is_optional(self):
        return bool(self.attribute('optional').value.upper() == "TRUE")
    
    def configure_arguments(self):        
        raise NotImplemented()
    
    @classmethod
    def lookup_obj(cl,name):
        for instance in cl.instances:
            if instance._name == name:
                return instance
        
        return None

class WafModuleDependency(ModuleDependency):

    def __init__(self):        

        ModuleDependency.__init__(self)
        self.add_attribute('configure_arguments', '', 'Arguments to pass to'
                           ' "waf configure"')
    
    @classmethod
    def name(cls):
        return 'waf'

    def configure_arguments(self):          
        return self.attribute('configure_arguments').value

class Module:
    followOptional = None
    FAIL = 0
    OK = 1

    def __init__(self, name, 
                 source,
                 build,
                 mtype,
                 min_ver,
                 max_ver,
                 dependencies = [],
                 built_once = False,
                 installed = []):
        self._name = name
        self._type = mtype
        self._dependencies = copy.copy(dependencies)
        self._source = source
        self._build = build
        self._built_once = built_once
        self._installed = installed
        self._minVersion = min_ver
        self._maxVersion = max_ver


    @property
    def installed(self):
        """ Returns if the module was already installed or not. """
        return self._installed
    @installed.setter
    def installed(self, value):
        """ Stores the given value on the module installed option. """
        self._installed = copy.copy(value)

    def _directory(self):
        return self._name

    def handleStopOnError(self, e):
        """Handles the stop on error parameter, prints the standard 
        message and calls exist."""
        colorTool = ColorTool()
        colorTool.cPrintln(colorTool.OK, " > Stop on error enabled (for more information call bake with -vv or -vvv)")
        colorTool.cPrintln(colorTool.FAIL, "   >> " + e._reason)
        os._exit(1)

    def printResult(self, env, operation, result):
        """Prints the result of the operation."""
        colorTool = ColorTool()
        resultStr = "OK"
        color=colorTool.OK
        if result == self.FAIL:
            resultStr = "Problem"
            color=colorTool.FAIL
            
        if env._logger._verbose > 0:
            print()
            colorTool.cPrintln(color, " >> " + operation + " " + 
                                    self._name + " - " +resultStr)
        else:
            colorTool.cPrintln(color, resultStr)

    def _do_download(self, env, source, name, forceDownload):
        """ Recursive download function, do the download for each 
        target module. """
        
        srcDirTmp = name
        if source.attribute('module_directory').value :
            srcDirTmp = source.attribute('module_directory').value
            
        env.start_source(name, srcDirTmp)
        rt = source.check_version(env)
        
        if forceDownload:
            try: # when forced download, removes the repository if it exists
                if os.path.isdir(env.srcdir):
                    shutil.rmtree(env.srcdir)
            except OSError as e:
                env._logger.commands.write('Could not remove source files'
                                            ' %s for module: %s \n Error: %s\n' % 
                                            (env.srcdir, env._module_name, 
                                             str(e)))
        aditionalModule=False
        if source.attribute('additional-module'):
            aditionalModule = source.attribute('additional-module').value
                 
        if os.path.isdir(env.srcdir) and not aditionalModule :
            colorTool = ColorTool()
            if env._logger._verbose == 0:
                colorTool.cPrint(colorTool.OK, "(Nothing to do, source directory already exists) - ")
            else:      
                colorTool.cPrintln(colorTool.OK, "  >>> No actions needed, the source directory for " + 
                               self._name + " already exists.")
                sys.stdout.write ("      If you want to update the module, use update instead download, or, if you want a fresh copy," + os.linesep
                      +"      either remove it from the source directory, or use the --force_download option.")
                
            if self._source.attribute('new_variable').value != '':
                elements = env.replace_variables(self._source.attribute('new_variable').value).split(";")
                env.add_variables(elements)


            env.end_source()
        else:
            try:
                source.download(env)
                if self._source.attribute('patch').value != '':
                    self._build.threat_patch(env, self._source.attribute('patch').value)
                if self._source.attribute('new_variable').value != '':
                    elements = env.replace_variables(self._source.attribute('new_variable').value).split(";")
                    env.add_variables(elements)
                if self._source.attribute('post_download').value != '':
                    self._source.perform_post_download(env)
            finally:
                env.end_source()
        for child, child_name in source.children():
            self._do_download(env, child, os.path.join(name, child_name))

    def download(self, env, forceDownload):
        """ General download function. """
        
        if self._build.attribute('supported_os').value :
            if not self._build.check_os(self._build.attribute('supported_os').value) : 
                import platform
                import distro
                osName = platform.system().lower()
                (distname,version,ids)=distro.linux_distribution()
                print('    Downloading, but this module works only on \"%s\"' 
                      ' platform(s), %s is not supported for %s %s %s %s' % 
                      (self._build.attribute('supported_os').value, 
                       self._name, platform.system(), distname,version,ids))
            
        try:
            self._do_download(env, self._source, self._name, forceDownload)

            if isinstance(self._source, SystemDependency):
                self.printResult(env, "Dependency ", self.OK)
            else:
                self.printResult(env, "Download", self.OK)
                
 
            return True
        except TaskError as e:
            if isinstance(self._source, SystemDependency):
                self.printResult(env, "Dependency ", self.FAIL)
            else:
                self.printResult(env, "Download", self.FAIL)
            env._logger.commands.write(e.reason+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()           
            if env.stopOnErrorEnabled:
                self.handleStopOnError(e)
            return False
        except:
            if isinstance(self._source, SystemDependency):
                self.printResult(env, "Install", self.FAIL)
            else:
                self.printResult(env, "Download", self.FAIL)
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()
            if env.stopOnErrorEnabled:
                er = sys.exc_info()[1]
                self.handleStopOnError(TaskError('Error: %s' % (er)))
            return False


    def _do_update(self, env, source, name):
        """ Recursive update function, do the update for each 
        target module. """
        
        srcDirTmp = name
        if source.attribute('module_directory').value :
            srcDirTmp = source.attribute('module_directory').value
            
        env.start_source(name, srcDirTmp)
        
        try:
            source.update(env)
        finally:
            env.end_source()
        for child, child_name in source.children():
            self._do_update(env, child, os.path.join(name, child_name))

    def update(self, env):
        """ Main update function. """
        
        try:
            self._do_update(env, self._source, self._name)
            self.printResult(env, " Update ", self.OK)
            return True
        except TaskError as e:
            self.printResult(env, " Update ", self.FAIL)
            env._logger.commands.write(e.reason+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()           
            if env.stopOnErrorEnabled:
                self.handleStopOnError(e)
            return False
        except:
            self.printResult(env, " Update ", self.FAIL)
            env._logger.commands.write(e.reason+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()
            if env.stopOnErrorEnabled:
                er = sys.exc_info()[1]
                self.handleStopOnError(TaskError('Error: %s' % (er)))
            return False
      
    def distclean(self, env):
        """ Main distclean source function, call the modules distclean. """
        
        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value

        env.start_build(self._name, srcDirTmp,
                        self._build.objdir)
        if not os.path.isdir(env.objdir) or not os.path.isdir(env.srcdir):
            env.end_build()
            return
        try:
            self._build.distclean(env)
            env.end_build()
            self._built_once = False
            self.printResult(env, "Distclean ", self.OK)
            return True
        except TaskError as e:
            self.printResult(env, "Distclean ", self.FAIL)
            env._logger.commands.write(e.reason+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()           
            return False
        except:
            env.end_build()
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()
            return False
       
    def fullclean(self, env):
        """ Main full clean function, deletes the source and installed files. """
        
        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value
            
        env.start_build(self._name, srcDirTmp, self._build.objdir)
        sys.stdout.write(" >> Removing source: " + self._name + ": " + env.srcdir)
        try: 
            shutil.rmtree(env.srcdir)
            self.printResult(env, "Removing source: ", self.OK)
        except Exception as e:
            err = re.sub(r'\[\w+ \w+\]+', ' ', str(e)).strip()
            env._logger.commands.write("    > " + err +'\n')
#            print (e)
            pass

        if os.path.isdir(env.objdir):
            sys.stdout.write(" >> Removing build: " + env.objdir)
            try: 
                shutil.rmtree(env.objdir)
                self.printResult(env, "Removing build: ", self.OK)
            except Exception as e:
                self.printResult(env, "Removing build: ", self.FAIL)
                err = re.sub(r'\[\w+ \w+\]+', ' ', str(e)).strip()
                env._logger.commands.write("    > " + err +'\n')
 
        if os.path.isdir(env.installdir):
            sys.stdout.write(" >> Removing installation: " + env.installdir)
            try: 
                shutil.rmtree(env.installdir)
                self.printResult(env, "Installation removed", self.OK)
            except Exception as e:
                self.printResult(env, "Installation removed", self.FAIL)
                err = re.sub(r'\[\w+ \w+\]+', ' ', str(e)).strip()
                env._logger.commands.write("    > " + err +'\n')
           
        return True
         

    def uninstall(self, env):
        """ Main uninstall function, deletes the installed files. """
         
        for installed in self._installed:
            try:
                os.remove(installed)
            except OSError:
                pass
            
        # delete directories where files were installed if they are empty
        dirs = [os.path.dirname(installed) for installed in self._installed]
        def uniq(seq):
            keys = {}
            for e in seq:
                keys[e] = 1
            return keys.keys()
        for d in uniq(dirs):
            try:
                os.removedirs(d)
            except OSError:
                pass
        self._installed = []


    def build(self, env, jobs, force_clean):
        """ Main build function. """
        
        # if there is no build we do not need to proceed 
        if self._build.name() == 'none' or self._source.name() == 'system_dependency':
            srcDirTmp = self._name
            if self._source.attribute('module_directory').value :
                srcDirTmp = self._source.attribute('module_directory').value
            
            if self._build.attribute('pre_installation').value != '':
                self._build.perform_pre_installation(env)
            if self._build.attribute('patch').value != '':
                self._build.threat_patch(env, self._build.attribute('patch').value)
            if self._build.attribute('post_installation').value != '':
                self._build.perform_post_installation(env)
                
            self._build.threat_variables(env)

            return True
        
        # delete in case this is a new build configuration
        # and there are old files around
        if force_clean:
            self.uninstall(env)
            if not self._built_once:
                self.clean(env)

        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value
            
        env.start_build(self._name, srcDirTmp,
                        self._build.objdir)
        
        # setup the monitor
        monitor = FilesystemMonitor(env.installdir)
        monitor.start()

        if self._build.attribute('supported_os').value :
            if not self._build.check_os(self._build.attribute('supported_os').value) : 
                import platform
                import distro
                osName = platform.system().lower()
                (distname,version,ids)=distro.linux_distribution()
                self.printResult(env, "Building", self.FAIL)
                print('    This module works only on \"%s\"' 
                      ' platform(s), %s is not supported for %s %s %s %s' % 
                      (self._build.attribute('supported_os').value, 
                       self._name, platform.system(), distname,version,ids))
                return

        if not os.path.isdir(env.installdir):
            os.mkdir(env.installdir)
        if self._build.objdir != '' and not os.path.isdir(env.objdir):
            os.mkdir(env.objdir)

        try:
            if not os.path.isdir(env.srcdir):
                raise TaskError('Source is not available for module %s: '
                            'directory %s not found.  Try %s download first.' % 
                            (env._module_name,env.srcdir, sys.argv[0]))

            if self._build.attribute('pre_installation').value != '':
                self._build.perform_pre_installation(env)
                
            self._build.threat_variables(env)

            if self._build.attribute('patch').value != '':
                self._build.threat_patch(env, self._build.attribute('patch').value)

            self._build.build(env, jobs)
            self._installed = monitor.end()
            if self._build.attribute('post_installation').value != '':
                self._build.perform_post_installation(env)
            env.end_build()
            self._built_once = True
            self.printResult(env, "Built", self.OK)
            return True
        except TaskError as e:
            self.printResult(env, "Building", self.FAIL)
            env._logger.commands.write("   > " + e.reason+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()           
            env.end_build()
            
            if env.stopOnErrorEnabled:
                self.handleStopOnError(e)

            return False
        except:
            self._installed = monitor.end()
            env.end_build()
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()
            if env.stopOnErrorEnabled:
                er = sys.exc_info()[1]
                self.handleStopOnError(TaskError('Error: %s' % (er)))
            return False

    def check_build_version(self, env):
        """ Checks the version of the selected build tool in the machine. """
        
        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value
            
        env.start_build(self._name, srcDirTmp,
                        self._build.objdir)
        
        retval = self._build.check_version(env)
        env.end_build()
        return retval

    def is_downloaded(self, env):
        """ Checks if the source code is not already available. """
        
        srcDirTmp = self._name
        if self._source.name() == 'system_dependency' :
            return True
        
        if self._source.name() == 'none' :
            return True

        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value

        env.start_source(self._name,srcDirTmp)
        retval = os.path.isdir(env.srcdir)
        env.end_source()
        return retval


    def check_source_version(self, env):
        """ Checks if the version of the available version control tool. """

        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value
        env.start_source(self._name, srcDirTmp)
        retval = self._source.check_version(env)
        env.end_source()
        return retval


    def update_libpath(self, env):
        """ Makes it available for the next modules the present libpath. """
        
        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value
            
        env.start_build(self._name, srcDirTmp,
                        self._build.objdir)
        env.add_libpaths([env._lib_path()])
        env.end_build()

    def clean(self, env):
        """ Main cleaning build option handler. """
        
        srcDirTmp = self._name
        if self._source.attribute('module_directory').value :
            srcDirTmp = self._source.attribute('module_directory').value

        env.start_build(self._name, srcDirTmp,
                        self._build.objdir)
        if not os.path.isdir(env.objdir) or not os.path.isdir(env.srcdir):
            env.end_build()
            return
        try:
            self._build.clean(env)
            env.end_build()
            self._built_once = False
            self.printResult(env, "Clean ", self.OK)
            return True
        except TaskError as e:
            self.printResult(env, "Clean ", self.FAIL)
            err = re.sub(r'\[\w+ \w+\]+', ' ', str(e)).strip()
            env._logger.commands.write(err+'\n')
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()           
            if env.stopOnErrorEnabled:
                self.handleStopOnError(e)
            return False
        except:
            env.end_build()
            if env.debug :
                import bake.Utils
                bake.Utils.print_backtrace()
            if env.stopOnErrorEnabled:
                er = sys.exc_info()[1]
                self.handleStopOnError(TaskError('Error: %s' % (er)))
            return False

    def is_built_once(self):
        return self._built_once
    def get_source(self):
        return self._source
    def get_build(self):
        return self._build
    def name(self):
        return self._name
    def dependencies(self):
        return self._dependencies
    def mtype(self):
        return self._type
    def minver(self):
        return self._minVersion
    def maxver(self):
        return self._maxVersion
    def addDependencies(self, depend):
        for d in self._dependencies:
            if d.name() == depend.name():
                return
        self._dependencies.append(depend)        
