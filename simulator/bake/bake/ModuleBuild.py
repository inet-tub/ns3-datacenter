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
 ModuleBuild.py
 
 This file stores the real build implementation for each one of the handled
 tools. It is this class that defines how a build with, for example, make
 will be done and how different is it from the build done with cmake 
''' 

import bake.Utils
import os
import distro
import platform
import subprocess
try:
    import commands
    from commands import getstatusoutput, getoutput
except ImportError:
    from subprocess import getstatusoutput, getoutput
import re
import sys
import shlex
from bake.Utils import ModuleAttributeBase
from bake.Exceptions import NotImplemented
from bake.Exceptions import TaskError 
from bake.Module import ModuleDependency

class ModuleBuild(ModuleAttributeBase):
    """ Generic build, to be extended by the specialized classes, 
    one for each handled kind of tool. 
    """

    def __init__(self):
        """ Default values for the generic attributes."""
        
        ModuleAttributeBase.__init__(self)
#        self._libpaths = []
        self.add_attribute('objdir', 'no', 'Module supports objdir != srcdir.', mandatory=False)
        self.add_attribute('patch', '', 'code to patch before build', mandatory=False)
        self.add_attribute('v_PATH', '', 'Directory, or directories separated'
                           ' by a \";\", to append to PATH environment variable', mandatory=False)
        self.add_attribute('v_LD_LIBRARY', '', 'Directory, or directories'
                           ' separated by a \";\", to append LD_LIBRARY'
                           ' environment variable', mandatory=False)
        self.add_attribute('v_PKG_CONFIG', '', 'Directory, or directories'
                           ' separated by a \";\", to append to PKG_CONFIG'
                           ' environment variable', mandatory=False)
        self.add_attribute('post_installation', '', 'UNIX Command to run'
                           ' after the installation', mandatory=False)
        self.add_attribute('no_installation', '', 'Does not call the installation'
                           ' by default', mandatory=False)
        self.add_attribute('pre_installation', '', 'UNIX Command to run'
                           ' before the installation', mandatory=False)
        self.add_attribute('supported_os', '', 'List of supported Operating'
                           ' Systems for the module', mandatory=False)
        self.add_attribute('ignore_predefined_flags', 'False', 'True if the'
                           ' build should ignore the predefined flag settings')
        self.add_attribute('new_variable', '', 'Appends the value to the'
                           ' system variable on the format VARIABLE1=value1'
                           ';VARIABLE2=value2', mandatory=False)
        # self.add_attribute('condition_to_build', '', 'Condition that, if '
        # 'existent, should be true for allowing the instalation')        

    @classmethod
    def subclasses(self):
        return ModuleBuild.__subclasses__()

    @classmethod
    def create(cls, name):
        """ Instantiates the Build class."""
        
        for subclass in ModuleBuild.subclasses():
            if subclass.name() == name:
                instance = subclass()
                return instance
        return None

    @property
    def objdir(self):
        return self.attribute('objdir').value
    def build(self, env, jobs):
        raise NotImplemented()
    def clean(self, env):
        raise NotImplemented()
    def check_version(self, env):
        raise NotImplemented()
    
    def check_os(self, supportedOs) :
        """ Verifies the minimum OS requirements."""
        
        osName = platform.system().lower()
        
        if len(supportedOs) == 0 :
            elements = []
        else :
            elements = supportedOs.strip().split(';')
            
        supportedOS = False
        
        for element in elements : 
            especification = element.strip().split(' ')
            # if the main version is correct e.g linux/darwin/windows`
            if(osName.startswith(especification[0].lower())):
                # if we need to go into a distribuition of the OS e.g. Debian/Fedora
                if(len(especification)>1):
                    (distname,version,id)=distro.linux_distribution()
                    for providedName in especification:
                        if distname.lower() == providedName.lower():
                            supportedOS = True
                else :
                    supportedOS = True
        
        return supportedOS 

    
    def perform_pre_installation(self, env):
        """ Executes a list of Linux commands BEFORE calling the build process."""

        if self.attribute('pre_installation').value != '':
            commandList = env.replace_variables(self.attribute('pre_installation').value).split(' or ')
                        
            for comandToExecute in commandList :
                try:
                    env._logger.commands.write("    > " +env.replace_variables(comandToExecute)+ '\n');
                    resultStatus = getstatusoutput(env.replace_variables(comandToExecute))
                    if(resultStatus[0] == 0) :
                        return True
                except Exception as e:
                    print ("   > Error executing pre installation : " + str(e) + "\n")

        return False
    
    def perform_post_installation(self, env):
        """ Executes a list of Linux commands AFTER the build is finished """
        
        if self.attribute('post_installation').value != '':
            try:
                env._logger.commands.write(" > " + env.replace_variables(self.attribute('post_installation').value))
                var = getoutput(env.replace_variables(self.attribute('post_installation').value))
                
                if env.debug:
                    print("  -> " +  var)
            except Exception as e:
                print ("   > Error executing post installation : " + e )

    # applies a patch if available
    def threat_patch(self, env, patchStr):
        """ Applies a patch, or a series of patches, over the source code.  """
        
        hasPatch = env.check_program('patch')
        if hasPatch == False:
            raise TaskError('Patch tool is not present and it is required for'
                            ' applying: %s, in: %s' 
                            % (self.attribute('patch').value, env._module_name))

        vectorPath = env.replace_variables(patchStr).split(';')      
        for item in vectorPath:
      
            if not env.exist_file(item) :
                raise TaskError('Patch file is not present! missing file:'
                                ' %s, in: %s' % (item, env._module_name))

            try:
                #check if patch has already been applied :
                env._logger.commands.write('cd ' + env.srcdir + '; patch -p1 -R --dry-run < ' + item + '\n')
                status = getstatusoutput('cd ' + env.srcdir + '; patch -p1 -R --dry-run < ' + item + '\n')

                if status[0] != 0:
                    try:
                        env._logger.commands.write('cd ' + env.srcdir + '; patch -p1 < ' + item + '\n')
                        status = getstatusoutput('cd ' + env.srcdir + '; patch -p1 < ' + item)

                    except:
                        raise TaskError('Patch error: %s, in: %s' % (item, env._module_name))
            
                    # if there were an error
                    if status[0] != 0:
                        if status[0] == 256:
                            env._logger.commands.write(' > Patch problem: Ignoring'
                                                        ' patch, either the patch file'
                                                        ' does not exist or it was '
                                                        'already applied!\n')
                        else:
                            raise TaskError('Patch error %s: %s, in: %s' % 
                                            (status[0], item, env._module_name))
            except:
                raise TaskError('Patch error: %s, in: %s' % (item, env._module_name))

    # Threats the parameter variables
    def threat_variables(self, env):
        """ Append the defined variables to the internal environment.  """

        elements = []
        if self.attribute('v_PATH').value != '':
            elements = env.replace_variables(self.attribute('v_PATH').value).split(";")
            env.add_libpaths(elements)
            env.add_binpaths(elements)
            
        if self.attribute('v_LD_LIBRARY').value != '':
            elements = env.replace_variables(self.attribute('v_LD_LIBRARY').value).split(";")
            env.add_libpaths(elements)

        if self.attribute('v_PKG_CONFIG').value != '':
            elements = env.replace_variables(self.attribute('v_PKG_CONFIG').value).split(";")
            env.add_pkgpaths(elements)

        if self.attribute('new_variable').value != '':
            elements = env.replace_variables(self.attribute('new_variable').value).split(";")
            env.add_variables(elements)


    def _flags(self):
        """ Adds the defined flags as a default for the build.  """
        
        variables = []
        if self.attribute('ignore_predefined_flags').value == 'True':
            return variables
                           
        if self.attribute('CFLAGS').value != '':
            variables.append('CFLAGS=%s'% (self.attribute('CFLAGS').value))
            
        if self.attribute('CXXFLAGS').value != '':
            variables.append('CXXFLAGS=%s'% (self.attribute('CXXFLAGS').value))
        return variables


class NoneModuleBuild(ModuleBuild):
    """ Class defined for the modules that do not need a build mechanism, 
    e.g system dependencies.
    """
        
    def __init__(self):
        ModuleBuild.__init__(self)
    @classmethod
    def name(cls):
        return 'none'
    def build(self, env, jobs):
        pass
    def clean(self, env):
        pass
    def check_version(self, env):
        return True


class InlineModuleBuild(ModuleBuild):
    """ Class defined for the modules that will use a Python code to be 
    installed. The build may be programmed in Python using all the Bake 
    functionalities.
    """
    
    def __init__(self):
        ModuleBuild.__init__(self)
    @classmethod
    def name(cls):
        return 'inline'
    
    @classmethod
    def className(self, code): 
        if code :
            myre = re.compile(".*class (?P<class_name>[a-zA-Z0-9_-]*)\(.*")
            m = myre.match(code)
            if m :
                return m.groupdict()['class_name']
        
        return self.__class__.__name__
    
    def check_version(self, env):
        return True


class PythonModuleBuild(ModuleBuild):
    """ Performs the build for python based projects."""
    
    def __init__(self):
        ModuleBuild.__init__(self)
    @classmethod
    def name(cls):
        """ Specific build type identifier."""
        
        return 'python'
    
    def build(self, env, jobs):
        """ Specific build implementation method. Basically call the setup.py 
        program passed as parameter."""
        
        # TODO: Add the options, there is no space for the configure_arguments
        env.run([sys.executable, os.path.join(env.srcdir, 'setup.py'), 'build',
                  '--build-base=' + env.objdir], directory=env.srcdir)
        
        if self.attribute('no_installation').value != True:
            sudoOp=[]
            if(env.sudoEnabled):
                sudoOp = ['sudo']
                
            env.run(sudoOp + [sys.executable, os.path.join(env.srcdir, 'setup.py'), 'install',
                              '--install-base=' + env.installdir,
                              '--install-purelib=' + env.installdir + '/lib',
#                             --install-platlib=' + env.installdir + '/lib.$PLAT,
                              '--install-scripts=' + env.installdir + '/scripts',
                              '--install-headers=' + env.installdir + '/include',
                              '--install-data=' + env.installdir + '/data',
                              ],
                    directory=env.srcdir)

    def clean(self, env):
        """ Call the code with the setup.py with the clean option, 
        to remove the older code.
        """
        
        env.run([sys.executable, os.path.join(env.srcdir, 'setup.py'), 'clean',
                 '--build-base=' + env.objdir],
                directory=env.srcdir)

    def distclean(self, env):
        """ Call the code with the setup.py with the distclean option, 
        to remove the older code.
        """
        
        env.run([sys.executable, os.path.join(env.srcdir, 'setup.py'), 'distclean'],
                directory=env.srcdir)
        
    def check_version(self, env):
        """Verifies only if python exists in the machine."""
        
        try: 
            env.run([sys.executable, '--version'])
        except TaskError as e:
            return False
            
        return True

class WafModuleBuild(ModuleBuild):
    """ Performs the build for Waf based projects."""

    def __init__(self):
        """ Instantiate the list of specific attributes for the waf build."""
        
        ModuleBuild.__init__(self)
        self.add_attribute('CC', '', 'C compiler to use')
        self.add_attribute('CXX', '', 'C++ compiler to use')
        self.add_attribute('CFLAGS', '', 'Flags to use for C compiler')
        self.add_attribute('CXXFLAGS', '', 'Flags to use for C++ compiler')
        self.add_attribute('LDFLAGS', '', 'Flags to use for Linker')
        self.add_attribute('configure_arguments', '', 'Arguments to pass to'
                           ' "waf configure"')
        self.add_attribute('build_arguments', '', 'Arguments to pass to "waf"')
        self.add_attribute('install_arguments', '', 'Command-line arguments'
                           ' to pass to waf install')

    @classmethod
    def name(cls):
        """ Specific build type identifier."""
    
        return 'waf'
    
    def _binary(self, srcdir):
        """ Searches for the waf program."""

        if os.path.isfile(os.path.join(srcdir, 'waf')):
            waf_binary = os.path.join(srcdir, 'waf')
        else:
            waf_binary = 'waf'
        return [sys.executable, waf_binary]
    
    def _env(self, objdir):
        """ Verifies if the main environment variables where defined and 
        sets them accordingly.
        """
        
        env = dict()
        for a, b in [['CC', 'CC'],
                    ['CXX', 'CXX'],
                    ['CFLAGS', 'CFLAGS'],
                    ['CXXFLAGS', 'CXXFLAGS'],
                    ['LDFLAGS', 'LINKFLAGS']]:
            if self.attribute(a).value != '':
                env[b] = self.attribute(a).value
#        todo: Evaluate the situations where a waf lock may be required, and if so
#        implement something on this line
#        env['WAFLOCK'] = '.lock-waf_%s_build'%sys.platform #'.lock-%s' % os.path.basename(objdir)
        return env
    
    def build(self, env, jobs):
        """ Specific build implementation method. In order: 
        1. Call waf configuration, if the configuration is set, 
        2. Call waf with the set build arguments, 
        3. Call waf with the install parameter. 
        """
        
        extra_configure_options = []

        """ 
        Check if default configure arguments have been already 
        defined while adding the current module as a dependency.
        """

        configure_arguments = None

        if self.attribute('configure_arguments').value != '' :
            configure_arguments = self.attribute('configure_arguments').value

        mod_dep = ModuleDependency.lookup_obj(env._module_name)

        if mod_dep != None:                
            if mod_dep.__class__.name() == 'waf':
                if mod_dep.configure_arguments() != "" :
                    configure_arguments = mod_dep.configure_arguments()

        if configure_arguments != None:
            extra_configure_options = [env.replace_variables(tmp) for tmp in
                                       bake.Utils.split_args(env.replace_variables(configure_arguments))]
            
            env.run(self._binary(env.srcdir) + extra_configure_options,
                    directory=env.srcdir,
                    env=self._env(env.objdir))

        extra_build_options = []
        if self.attribute('build_arguments').value != '':
            extra_build_options = [env.replace_variables(tmp) for tmp in
                                   bake.Utils.split_args(env.replace_variables(self.attribute('build_arguments').value))]
        jobsrt=[]
        if not jobs == -1:
            jobsrt = ['-j', str(jobs)]
            
        env.run(self._binary(env.srcdir) + extra_build_options + jobsrt,
                directory=env.srcdir,
                env=self._env(env.objdir))
        
        if self.attribute('no_installation').value != True:

            sudoOp=[]
            if(env.sudoEnabled):
                sudoOp = ['sudo']

            try :
                options = bake.Utils.split_args(env.replace_variables(self.attribute('install_arguments').value))
                env.run(sudoOp + self._binary(env.srcdir) + ['install'] + options,
                directory=env.srcdir,
                env=self._env(env.objdir))
            except TaskError as e:
                print('    Could not install, probably you do not have permission to'
                      ' install  %s: Verify if you have the required rights. Original'
                      ' message: %s' % (env._module_name, e._reason))
       
    def clean(self, env):
        """ Call waf with the clean option to remove the results of the 
        last build.
        """

        env.run(self._binary(env.srcdir) + ['-k', 'clean'],
                directory=env.srcdir,
                env=self._env(env.objdir))
            
    def distclean(self, env):
        """ Call waf with the distclean option to remove the results of the 
        last build.
        """

        env.run(self._binary(env.srcdir) + ['-k', 'distclean'],
                directory=env.srcdir,
                env=self._env(env.objdir))
            
    def check_version(self, env):
        """ Verifies the waf version."""
        
        for path in [os.path.join(env.srcdir, 'waf'), 'waf']:
            if env.check_program(path, version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)\.(\d+)',
                                 version_required=(1, 5, 9)):
                return True
            
        return False


class Cmake(ModuleBuild):
    """ Performs the build for CMake based projects."""
    
    def __init__(self):
        """ Instantiate the list of specific attributes for the CMake build."""

        ModuleBuild.__init__(self)
        self.add_attribute('CC', '', 'C compiler to use')
        self.add_attribute('CXX', '', 'C++ compiler to use')
        self.add_attribute('CFLAGS', '', 'Flags to use for C compiler')
        self.add_attribute('CXXFLAGS', '', 'Flags to use for C++ compiler')
        self.add_attribute('LDFLAGS', '', 'Flags to use for Linker')
        self.add_attribute('cmake_arguments', '', 'Command-line arguments'
                           ' to pass to cmake --build')
        self.add_attribute('configure_arguments', '', 'Command-line arguments'
                           ' to pass to cmake configuration')
        self.add_attribute('install_arguments', '', 'Command-line arguments'
                           ' to pass to make install')

    @classmethod
    def name(cls):
        """ Specific build type identifier."""
        
        return 'cmake'

    def _variables(self):
        """ Verifies if the main environment variables were defined and 
        sets them accordingly.
        """

        variables = []
        for a, b in [['CC', 'C_COMPILER'],
                    ['CXX', 'CXX_COMPILER'],
                    ['CFLAGS', 'CFLAGS'],
                    ['CXXFLAGS', 'CXXFLAGS'],
                    ['LDFLAGS', 'EXE_LINKER_FLAGS']]:
            if self.attribute(a).value != '':
                variables.append('-DCMAKE_%s=%s' % (b, self.attribute(a).value))
                
        return variables

    def build(self, env, jobs):
        """ Specific build implementation method. In order: 
        1. Call cmake to create the make files, with configure arguments
        2. Call cmake --build to build the code, with cmake arguments
        3. Call 'cmake --build . --target install' with install arguments
        """

        # Check that objdir was set for cmake
        if env.objdir is None:
            raise TaskError('objdir not configured; please configure objdir for cmake')
        # Check that ModuleEnvironment.start_build() created objdir
        try:
            env.run(['test', '-d', env.objdir],
                    directory=env.srcdir)
        except:
            raise TaskError('objdir %s configured but missing; possible problem in ModuleEnvironment.start_build()' % env.objdir)

        jobsrt=[]
        if not jobs == -1:
            jobsrt = ['-j', str(jobs)]

        options = []
        if self.attribute('configure_arguments').value != '':
            options = bake.Utils.split_args(
                          env.replace_variables(self.attribute('configure_arguments').value))

        env.run(['cmake', env.srcdir, '-DCMAKE_INSTALL_PREFIX:PATH='
            + env.installdir] + self._variables() + options, directory=env.objdir)
        
        options = []
        if self.attribute('cmake_arguments').value != '':
            options = bake.Utils.split_args(
                          env.replace_variables(self.attribute('cmake_arguments').value))

        env.run(['cmake', '--build', env.objdir] + options + jobsrt, directory=env.objdir)

        if self.attribute('no_installation').value != True:

            sudoOp=[]
            if(env.sudoEnabled):
                sudoOp = ['sudo']

            try:
                options = bake.Utils.split_args(env.replace_variables(self.attribute('install_arguments').value))
                env.run(sudoOp + ['cmake', '--build', '.', '--target', 'install'] + options, directory=env.objdir)
            except TaskError as e:
                print('    Could not install, probably you do not have permission to'
                      ' install  %s: Verify if you have the required rights. Original'
                      ' message: %s' % (env._module_name, e._reason))

    def clean(self, env):
        """ Call make clean to remove the results of the last build."""

        if not os.path.isfile(os.path.join(env.objdir, 'Makefile')):
            return
        
        env.run(['make','-i', 'clean'], directory=env.objdir)
        
    def distclean(self, env):
        """ Call make distclean to remove the results of the last build."""

        if not os.path.isfile(os.path.join(env.objdir, 'Makefile')):
            return
        
        env.run(['make','-i', 'distclean'], directory=env.objdir)
        
    def check_version(self, env):
        """ Verifies if CMake and Make are available and their versions."""

        if not env.check_program('cmake', version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)\.(\d+)',
                                 version_required=(2, 8, 2)):
            return False
        
        if not env.check_program('make', version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)',
                                 version_required=(3, 80)):
            return False
        
        return True

# Class to handle the make build tool
class Make(ModuleBuild):
    def __init__(self):
        """ Instantiate the list of specific attributes for the make build."""

        ModuleBuild.__init__(self)
        self.add_attribute('CFLAGS', '', 'Flags to use for C compiler')
        self.add_attribute('CXXFLAGS', '', 'Flags to use for C++ compiler')
        self.add_attribute('LDFLAGS', '', 'Flags to use for Linker')
        self.add_attribute('build_arguments', '', 'Targets to make before install')
        self.add_attribute('configure_arguments', '', 'Command-line arguments'
                           ' to pass to make')
        self.add_attribute('install_arguments', '', 'Command-line arguments'
                           ' to pass to make install')
        
    @classmethod
    def name(cls):
        """ Specific build type identifier."""

        return 'make'
    
    def build(self, env, jobs):
        """ Specific build implementation method. In order: 
        1. Call make configure, if the configurations are available, 
        2. Call make with the set build arguments 
        3. Call make with the install arguments.
        """

        # Check that objdir was created if set for make
        if env.objdir is not None:
            # Check that ModuleEnvironment.start_build() created objdir
            try:
                env.run(['test', '-d', env.objdir],
                        directory=env.srcdir)
            except:
                raise TaskError('objdir %s configured but missing; possible problem in ModuleEnvironment.start_build()' % env.objdir)

        # Configures make, if there is a configuration argument that was passed as parameter
        options = []      
        if self.attribute('configure_arguments').value != '':
            options = bake.Utils.split_args(env.replace_variables(self.attribute('configure_arguments').value))
            env.run(['make'] + self._flags() + options,  directory=env.srcdir)
        
        
        jobsrt=[]
        if not jobs == -1:
            jobsrt = ['-j', str(jobs)]

        options = bake.Utils.split_args(env.replace_variables(self.attribute('build_arguments').value))
        env.run(['make']+jobsrt + self._flags() + options, directory=env.srcdir)
           
        if self.attribute('no_installation').value != str(True):

            sudoOp=[]
            if(env.sudoEnabled):
                sudoOp = ['sudo']

            try:
                options = bake.Utils.split_args(env.replace_variables(self.attribute('install_arguments').value))
                env.run(sudoOp + ['make', 'install']  + self._flags() + options, directory=env.srcdir)
            except TaskError as e:
                raise TaskError('    Could not install, probably you do not have permission to'
                      ' install  %s: Verify if you have the required rights. Original'
                      ' message: %s' % (env._module_name, e._reason))

    def clean(self, env):
        """ Call make clean to remove the results of the last build ."""

        if not os.path.isfile(os.path.join(env.objdir, 'Makefile')):
            return
        
        env.run(['make', 'clean'], directory=env.objdir)
        
    def distclean(self, env):
        """ Call make distclean to remove the results of the last build."""

        if not os.path.isfile(os.path.join(env.objdir, 'Makefile')):
            return
        
        env.run(['make', '-i', 'distclean'], directory=env.objdir)

    def check_version(self, env):
        """ Verifies if Make are available and its versions."""
        
        if platform.system() == 'FreeBSD':
            # FreeBSD make program does not identify version the same way
            # as does GNU make, so we just check for availability of make
            if not env.check_program('make'):
                return False
            else:
                return True

        if not env.check_program('make', version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)',
                                 version_required=(3, 80)):
            return False
        return True


class Autotools(ModuleBuild):
    def __init__(self):
        """ Instantiate the list of specific attributes for the Autotools build."""
        
        ModuleBuild.__init__(self)
        self.add_attribute('CC', '', 'C compiler to use')
        self.add_attribute('CXX', '', 'C++ compiler to use')
        self.add_attribute('CFLAGS', '', 'Flags to use for C compiler')
        self.add_attribute('CXXFLAGS', '', 'Flags to use for C++ compiler')
        self.add_attribute('LDFLAGS', '', 'Flags to use for Linker')
        self.add_attribute('maintainer', 'no', 'Maintainer mode ?')
        self.add_attribute('configure_arguments', '', 'Command-line arguments'
                           ' to pass to configure')
        self.add_attribute('install_arguments', '', 'Command-line arguments'
                           ' to pass to make install')
        
    @classmethod
    def name(cls):
        """ Specific build type identifier."""
  
        return 'autotools'

    def _variables(self):
        """ Verifies if the main environment variables where defined and 
        sets them accordingly."""

        variables = []
        for tmp in ['CC', 'CXX', 'CFLAGS', 'CXXFLAGS', 'LDFLAGS']:
            if self.attribute(tmp).value != '':
                variables.append('%s=%s' % (tmp, self.attribute(tmp).value))
                
        return variables
    
    def build(self, env, jobs):
        """ Specific build implementation method. In order: 
        1. Call autoreconf, if on maintainer mode
        2. Call make configure, if the configure arguments are available, 
        3. Call make to perform the build 
        4. Call make with the install arguments.
        """

        if self.attribute('maintainer').value != 'no':
            env.run(['autoreconf', '--install'],
                    directory=env.srcdir)
            
        options = []
        if self.attribute('configure_arguments').value != '':
            command= (env.replace_variables(env.replace_variables(self.attribute('configure_arguments').value)))
            
            if not "--prefix" in command:
                command = command + ' --prefix=' + env.objdir
                
            command = shlex.split(command)
            env.run(command, directory=env.objdir)
        
        
        jobsrt=[]
        if not jobs == -1:
            jobsrt = ['-j', str(jobs)]
    
        env.run(['make']+jobsrt, directory=env.objdir)
        
        if self.attribute('no_installation').value != True:

            sudoOp=[]
            if(env.sudoEnabled):
                sudoOp = ['sudo']

            try :
                options = bake.Utils.split_args(env.replace_variables(self.attribute('install_arguments').value))
                env.run(sudoOp + ['make', 'install'] + options, directory=env.objdir)
            except TaskError as e:
                print('    Could not install, probably you do not have permission to'
                      ' install  %s: Verify if you have the required rights. Original'
                      ' message: %s' % (env._module_name, e._reason))

    def clean(self, env):
        """ Call make maintainerclean or distclean to remove the results of 
        the last build.
        """

        if not os.path.isfile(os.path.join(env.objdir, 'Makefile')):
            return
        
        if self.attribute('maintainer').value != 'no':
            env.run(['make', '-k', 'maintainerclean'], directory=env.objdir)
        else:
            env.run(['make', '-k', 'distclean'], directory=env.objdir)
            
        try:
            os.remove(os.path.join(env.objdir, 'config.cache'))
        except OSError:
            pass

    def distclean(self, env):
        """ Call make distclean to remove the results of the last build."""

        Autotools.clean(self, env)

        
    def check_version(self, env):
        """ Verifies if Autoreconf and Make are available and their versions."""

        if not env.check_program('autoreconf', version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)',
                                 version_required=(2, 13)):
            return False
        
        if not env.check_program('make', version_arg='--version',
                                 version_regexp=b'(\d+)\.(\d+)',
                                 version_required=(3, 80)):
            return False
        
        return True
            

