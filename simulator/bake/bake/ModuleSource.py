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
 ModuleSource.py
 
 This file stores the real source fetching implementations for each one of 
 the handled source code repository tools. It is this class that defines how 
 a download of a zip file, or a mercurial repository will be made.  
''' 
import urllib
try:
    from urllib.parse import urlparse
    from urllib.request import urlretrieve
except ImportError:
    from urlparse import urlparse
    from urllib import urlretrieve
import bake.Utils
from bake.Exceptions import TaskError
from bake.Utils import ModuleAttributeBase
import os
import re
import platform
try:
    import distro
except ImportError:
    import sys
    print(">>> Error: missing Python 'distro' package; please install")
    sys.exit(1)
import subprocess
import importlib
try:
    import commands
    from commands import getoutput
except ImportError:
    from subprocess import getoutput
from datetime import date

class ModuleSource(ModuleAttributeBase):
    """ Generic Source class, stores the generic methods for the source 
    code repository tools.
    """
    
    def __init__(self):
        """ Generic attributes definition."""

        ModuleAttributeBase.__init__(self)
        self.add_attribute('module_directory', '', 'Source module directory', 
                           mandatory=False)
        self.add_attribute('patch', '', 'code to patch after download', 
                           mandatory=False)
        
        self.add_attribute('new_variable', '', 'Appends the value to the'
                           ' system variable on the format VARIABLE1=value1'
                           ';VARIABLE2=value2', mandatory=False)

        self.add_attribute('post_download', '', 'UNIX Command to run'
                           ' after the download', mandatory=False)

    @classmethod
    def subclasses(self):
        return ModuleSource.__subclasses__()
    
    @classmethod
    def create(cls, name):
        """Instantiates the class that is called by the requested name."""
        
        # Runs over all the Classes and instantiates the one that has the name
        # equals to the name passed as parameter
        for subclass in ModuleSource.subclasses():
            if subclass.name() == name:
                return subclass()
        return None

    # Methods to be implemented by the inherited classes
    def diff(self, env):
        raise NotImplemented()
    def download(self, env):
        raise NotImplemented()
    def update(self, env):
        raise NotImplemented()
    def check_version(self, env):
        raise NotImplemented()
    
    def perform_post_download(self, env):
        """ Executes a list of Linux commands AFTER the download is finished """
        
        if self.attribute('post_download').value != '':
            try:
                env._logger.commands.write(" > " + env.replace_variables(self.attribute('post_download').value))
                var = getoutput(env.replace_variables(self.attribute('post_download').value))
                
                if env.debug:
                    print("  -> " +  var)
            except Exception as e:
                print ("   > Error executing post download : " + e )


    @classmethod
    def source_systemtool(self):
        """Returns the name of the system instalation tool for this machine."""
        tools = dict()
        tools['debian']= 'apt-get'
        tools['ubuntu']= 'apt-get'
        tools['linuxmint']= 'apt-get'
        tools['fedora']= 'yum'
        tools['redhat']= 'yum'
        tools['centos']= 'yum'
        tools['suse']= 'yast'
        tools['darwin']= 'port'
            
        osName = platform.system().lower()
        if osName.startswith('linux'):
            (distribution, version, version_id) = distro.linux_distribution()
            if not distribution:
                distribution = osName
            else:
                distribution = distribution.lower()
        else:
            distribution = osName
        
        if distribution in tools:
            return tools[distribution]
        else :
            return ''
        
    def _check_dependency_expression(self, env, valueToTest):
        """Verifies if the preconditions exist or not."""
        
        # if there is no test to do, return true
        if(not valueToTest):
            return True
        
        stringToChange = valueToTest
        
        ### Clean expression
        # elements to ignore
        lib_var = [r'\b(or)\b', r'\b(not)\b', r'\b(and)\b',r'\b(if)\b']
        
        stringToChange = re.sub(r'(\(|\))',' ',stringToChange)
        for element in lib_var :
            stringToChange = re.sub(element,'',stringToChange) 
        
        stringToChange = re.sub(' +',' ', stringToChange)
        
        # split the command names
        if re.search(' ', stringToChange):
            elementsToChange = stringToChange.split()
        else :
            elementsToChange = [stringToChange]
        
        # add the call to the function that verifies if the program exist
        elementsSet = set([])
        for element in elementsToChange:
            elementsSet.add(element) 
        
        
        stringToChange = self._add_command_calls(valueToTest.replace('\\',''),
                                                 elementsSet)
               
        # Evaluate if all the programs exist
        returnValue = eval(stringToChange)
        return returnValue     

    def _split_path_expression(self, inputString):
        """Split and clean the path expression"""
        if not inputString:
            return set([])

        ### Clean expression
        # elements to ignore
        # XXX the 'not', 'and', and 'if' separators are being treated
        # as 'or'; also, there should be some string checking for validity
        lib_var = [r'\b(or)\b', r'\b(not)\b', r'\b(and)\b',r'\b(if)\b']
        
        inputString = re.sub(r'(\(|\))',' ',inputString)
        for element in lib_var :
            inputString = re.sub(element,'',inputString) 
        
        inputString = re.sub(' +',' ', inputString)
        
        # split the command names
        if re.search(' ', inputString):
            elementsToChange = inputString.split()
        else :
            elementsToChange = [inputString]
        
        # add the call to the function that verifies if the program exist
        elementsSet = set([])
        for element in elementsToChange:
            elementsSet.add(element) 

        return elementsSet

    def _check_file_expression(self, valueToTest):
        """Verifies if the system has the requested file or symbolic link"""
        
        returnValue = False
        # if there is no test to do, return true
        if(not valueToTest):
            return True

        # The expression may contain a list of 'or' separated paths to check
        # Split them into a set 
        elementsSet = self._split_path_expression (valueToTest)
        
        for e in elementsSet:
           if (os.path.isfile (e) or os.path.islink(e)):
               returnValue = True
               break
        return returnValue     

    def _check_executable_expression(self, valueToTest):
        """Verifies if the system has the requested executable"""
        
        returnValue = False
        # if there is no test to do, return true
        if(not valueToTest):
            return True

        # The expression may contain a list of 'or' separated paths to check
        # Split them into a set 
        elementsSet = self._split_path_expression (valueToTest)
        
        for e in elementsSet:
           if (os.path.exists (e) and os.access (e, os.X_OK)):
               returnValue = True
               break
        return returnValue     

    def _check_import(self, valueToTest):
        """Verifies if the system has the requested python import"""

        try:
            importlib.import_module(valueToTest, package=None)
        except ImportError:
            return False
        return True

class NoneModuleSource(ModuleSource):
    """ This class defines an empty source, i.e. no source code fetching is 
    needed. For compatibility purposes, it is possible to create a given module 
    has no need for source code fetching.
    """

    def __init__(self):
        ModuleSource.__init__(self)
    @classmethod
    def name(cls):
        return 'none'
    def diff(self, env):
        pass
    def download(self, env):
        pass
    def update(self, env):
        pass
    def check_version(self, env):
        return True


class InlineModuleSource(ModuleSource):
    """ This class enables one to create a python function, using the Bake 
    framework, to  directly to search for code.
    """
    
    def __init__(self):
        ModuleSource.__init__(self)
        
    @classmethod
    def name(cls):
        return 'inline'


class BazaarModuleSource(ModuleSource):
    """Handles the modules that have the sources stored in a bazaar repository."""
    
    def __init__(self):
        """ Specific attributes definition."""
        
        ModuleSource.__init__(self)
        self.add_attribute('url', '', 'The url to clone from',
                           mandatory=True)
        self.add_attribute('revision', None, 'The revision to update to after' 
                           ' the clone.')
    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        return 'bazaar'
    
    def diff(self, env):
        pass

    def download(self, env):
        """ Downloads the code, of a specific version, using Bazaar."""
        
        rev_arg = []
        if not self.attribute('revision').value is None:
            rev_arg.extend(['-r', self.attribute('revision').value])
        env.run(['bzr', 'branch'] + rev_arg + [self.attribute('url').value, 
                                              env.srcdir])

    def update(self, env):
        """ Updates the code using a specific version from the repository."""
        
        rev_arg = []
        if not self.attribute('revision').value is None:
            rev_arg.extend(['-r', self.attribute('revision').value])
        env.run(['bzr', 'pull'] + rev_arg + [self.attribute('url').value], 
                directory=env.srcdir)
        
    def check_version(self, env):
        """ Checks if the tool is available and with the needed version."""
        return env.check_program('bzr', version_arg='--version',
                                 version_regexp='(\d+)\.(\d+)',
                                 version_required=(2, 1))

    
class MercurialModuleSource(ModuleSource):
    """Handles the modules that have the sources stored in a mercurial 
    repository.
    """
    
    def __init__(self):
        """ Specific attributes definition."""

        ModuleSource.__init__(self)
        self.add_attribute('url', '', 'The url to clone from',
                            mandatory=True)
        self.add_attribute('revision', 'tip', 'The revision to update to '
                           'after the clone. '
                           'If no value is specified, the default is "tip"')
    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        return 'mercurial'

    def download(self, env):
        """ Downloads the code, of a specific version, using Mercurial."""
        
        env.run(['hg', 'clone', '-U', self.attribute('url').value, env.srcdir])
        env.run(['hg', 'update', '-r', self.attribute('revision').value],
                    directory=env.srcdir)
        
    def update(self, env):
        """ Updates the code using a specific version from the repository."""

        env.run(['hg', 'pull', self.attribute('url').value], 
                directory=env.srcdir)
        env.run(['hg', 'update', '-r', self.attribute('revision').value],
                directory=env.srcdir)
        
    def check_version(self, env):
        """ Checks if the tool is available and with the needed version."""
        return env.check_program('hg')


import shutil
class ArchiveModuleSource(ModuleSource):
    """Handles the modules that have the sources as a single tarball like file."""
    
    def __init__(self):
        """ Specific attributes definition."""

        ModuleSource.__init__(self)
        self.add_attribute('url', None, 'The url to clone from',
                           mandatory=True)
        self.add_attribute('additional-module', None, 
                           "Tags this module as an additional sub-module to be"
                           " added to another module.",
                           mandatory=False)
        self.add_attribute('extract_directory', None,
                           "The name of the directory the source code will "
                           "be extracted to naturally. If no value is "
                           "specified, directory is assumed to be equal to "
                           "the  archive without the file extension.")
    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        return 'archive'

    def _decompress(self, filename, env):
        """Uses the appropriate tool to uncompress the sources."""
        
        import tempfile
        import os
        tempdir = tempfile.mkdtemp(dir=env.srcrepo)
        extensions = [
            ['tar', ['tar', 'xf']],
            ['tar.gz', ['tar', 'zxf']],
            ['tar.Z', ['tar', 'zxf']],
            ['tar.bz2', ['tar', 'jxf']],
            ['zip', ['unzip']],
            ['rar', ['unrar', 'e']],
            ['tar.xz', ['tar', 'Jxf']],
            ['xz', ['unxz']],
            ['7z', ['7z', 'x']],
            ['tgz', ['tar', 'xzvf']],
            ['tbz2', ['tar', 'jxf']]
           ]
        
        # finds the right tool
        for extension, command in extensions:
            if filename.endswith(extension):
                env.run(command + [filename], directory=tempdir)
                if self.attribute('extract_directory').value is not None:
                    actual_extract_dir = self.attribute('extract_directory').value
                else:
                    actual_extract_dir = os.path.basename(filename)[0:-len(extension) - 1]
                    
                # finally, rename the extraction directory to the target 
                # directory name.
                try:
                    destDir=os.path.join(tempdir, actual_extract_dir)
                    
                    if os.path.isdir(env.srcdir):         
                        bake.Utils.mergeDirs(destDir, env.srcdir)
                    else:
                        os.rename(destDir, env.srcdir)
                        
                    shutil.rmtree(tempdir) # delete directory
                except (OSError, IOError) as e:
                    raise TaskError("Rename problem for module: %s, from: %s, " 
                                    "to: %s, Error: %s" 
                                    % (env._module_name,os.path.join(tempdir, 
                                                                     actual_extract_dir), 
                                       env.srcdir, e))
                return
        raise TaskError('Unknown Archive Type: %s, for module: %s' % 
                        (filename, env._module_name))

    def download(self, env):
        """Downloads the specific file."""
        

         
        url_local = self.attribute('url').value
       
        filename = os.path.basename(urlparse(url_local).path)
        tmpfile = os.path.join(env.srcrepo, filename)
        try:
            urlretrieve(url_local, filename=tmpfile)
        except IOError as e:
            raise TaskError('Download problem for module: %s, URL: %s, Error: %s' 
                            % (env._module_name,self.attribute('url').value, e))
            
        self._decompress(tmpfile, env)
        
    def update(self, env):
        """ Empty, no update is possible for files.""" 
        pass

    def check_version(self, env):

        """Verifies if the right program exists in the system to handle the
         given compressed source file.
         """
        extensions = [
            ['tar', 'tar'],
            ['tar.gz', 'tar'],
            ['tar.Z', 'tar'],
            ['tar.bz2', 'tar'],
            ['tgz', 'tar'],
            ['zip', 'unzip'],
            ['rar', 'unrar'],
            ['7z', '7z'],
            ['xz', 'unxz'],
            ['Z', 'uncompress']
            ]
        try:
            filename = os.path.basename(urlparse(self.attribute('url').value).path)
        except AttributeError as e:
            return False

        for extension, program in extensions:
            if filename.endswith(extension):
                return env.check_program(program)
        return False
        
        
class SystemDependency(ModuleSource):
    """Handles the system dependencies for a given module.  If a system
       dependency is not met, advise the user on how to install the
       missing dependency, if possible.  Dependencies may be expressed
       by requesting bake to check for a specific file (such as a library 
       or header file) in one or more locations, or checking for an 
       executable program in one or more locations.
    """
    
    def __init__(self):
        """ Specific attributes definition."""

        ModuleSource.__init__(self)
        self.add_attribute('dependency_test', None, 
                           '(DEPRECATED) The name of the dependency',
                           mandatory=False)
        self.add_attribute('file_test', None, 'System file to try to locate',
                           mandatory=False)
        self.add_attribute('executable_test', None, 'Executable to try to locate',
                           mandatory=False)
        self.add_attribute('import_test', None, 'Python import to try',
                           mandatory=False)
        self.add_attribute('try_to_install', 'false', 
                           '(DEPRECATED) If should try to install or not',
                           mandatory=False)
        self.add_attribute('sudoer_install', None, 
                           '(DEPRECATED) Try to install the dependency as sudoer',
                           mandatory=False)
        self.add_attribute('name_yum', None, 
                           'The package name of the module, for RPMs',
                           mandatory=False)
        self.add_attribute('name_apt-get', None, 
                           'The package name of the module, for use with apt-get ',
                           mandatory=False)
        self.add_attribute('name_yast', None, 
                           'The package name of the module, for use with yast',
                           mandatory=False)
        self.add_attribute('name_port', None, 
                           'The package name of the module, for use with port (Mac OS)',
                           mandatory=False)
        self.add_attribute('more_information', None, 
                           'Gives users a better hint of where to search for the module' ,
                           mandatory=True)
    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        return 'system_dependency'
    
    def _get_command(self, distribution):
        """Finds the proper installer command line, given the OS distribution.
        """
        
        distributions = [
            ['debian', 'apt-get install '],
            ['ubuntu', 'apt-get install '],
            ['linuxmint', 'apt-get install '],
            ['fedora', 'yum install '],
            ['redhat', 'yum install '],
            ['centos', 'yum install '],
            ['suse', 'yast --install '],
            ['darwin', 'port install '],
            ]
 
        for name, command in distributions:
            if distribution.startswith(name):
                return command
        return ''

    def remove(self, env):
        """ Removes the the present version of the dependency."""
        
        # if the download is dependent of the machine's architecture 
        osName = platform.system().lower()
        if(not osName.startswith('linux') and not osName.startswith('darwin')):
            raise TaskError("Not a Linux/Mac OS machine, self installation is not"
                            " possible in %s for module: %s,  %s" % 
                            (osName, env._module_name, 
                             self.attribute('error_message').value))
        
        (distribution, version, version_id) = distro.linux_distribution()

        if not distribution:
            distribution = 'darwin' # osName
        else:
            distribution = distribution.lower()

        command = self._get_command(distribution)
        command = command.rstrip().rsplit(' ', 1)[0] + ' remove'
        installerName = self.attribute('name_' + command.split()[0]).value
            
        # if didn't find the specific installer name uses the default one
        if(not installerName):
            installerName = env._module_name

        # if should try to remove as sudoer
        sudoer=self.attribute('sudoer_install').value
        if sudoer: sudoer = sudoer.lower()
        if(sudoer =='true' and (not env.sudoEnabled)):
            raise TaskError('    Module: \"%s\" requires sudo rights, ask your'
                            ' system admin to remove \"%s\" from your machine.\n'
                            '    More information from the module: \"%s\"' 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))

        if(env.sudoEnabled):
            command = "sudo "+ command
            command = command

        # uses apt-get/yum/... to remove the module
        try:
            env.run((command + " " + installerName).split(" "), 
                    directory=env.srcrepo)
        except IOError as e:
            raise TaskError('    Removing module problem: \"%s\", Message: %s, Error: %s' 
                            % (env._module_name,  
                               self.attribute('more_information').value, e))
        except TaskError as e1:
            if(env.sudoEnabled):
                e1.reason = ("    Removing problem for module: \"%s\", "
                            "\n    Probably either you miss rights or the module is"
                            " not present on your package management databases."
                            "\n    Try to either talk to your system admin or review your "
                            "library database to add \"%s\"\n"
                            "    More information from the module: \"%s\"" 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))
            else:
                e1.reason = ("    Removing problem for module: \"%s\", "
                            "\n    Probably you either need super user rights"
                            " to remove the packet, or the module is"
                            " not present on your package management databases."
                            "\n    Try calling bake with the --sudo option and/or " 
                            "review your library database to add \"%s\"\n"
                                "    More information from the module: \"%s\"" 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))

            raise TaskError("    Removing module problem: \"%s\",\n    Probably you"
            "miss sudo rights or the module is not present on your package "
            "management databases. \n    Try calling bake with --sudo or reviewing your"
                            " library database to add \"%s\"\n"
                                "    More information from the module: \"%s\"" 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))
        return True

    def _add_command_calls(self, stringToChange, elements):
        """ Define the command calls to be executed. """
        
        for element in elements:
            stringToChange= re.sub(element + "(\s|\)|$)" , 
                                   'env.check_program(\'' + element.replace('\\','') + 
                                   '\')\\1', stringToChange)
        return stringToChange



    def download(self, env):
        """ Verifies if the system dependency exists, if exists returns true, 
        if not, and we are in a supported machine, tries to download and install 
        the dependency.  
        """
        
        selfInstalation = self.attribute('try_to_install').value
        if selfInstalation: selfInstalation = selfInstalation.lower()

        if not selfInstalation == 'true' :
            raise TaskError('    Module: \"%s\" is required by other modules but it is not available on your system.\n' 
                    '     Ask your system admin or review your library database to add \"%s\"\n'
                    '    More information from the module: \"%s\"' 
                            % (env._module_name, env._module_name,
                               self.attribute('more_information').value))
        
        # even if should try to install, if it is not a supported machine 
        # we will not be able to
        osName = platform.system().lower().strip()
        if((osName.startswith('linux') or osName.startswith('darwin')) and 
           selfInstalation == 'true'):
            (distribution, version, version_id) = distro.linux_distribution()
            
            if not distribution:
                distribution = osName.split()[0] # osName
            else:
                distribution = distribution.lower()
                
            command = self._get_command(distribution)
            
            # didn't recognize the distribution, asks user to install by himself
            if command == '' : 
                raise TaskError('    Module: \"%s\" is required by other modules but it is not available on your system.\n' 
                    '    Ask your system admin\n'
                    '    > More information from the module: \"%s\"' 
                            % (env._module_name, 
                               self.attribute('more_information').value))
            
            installerName = self.attribute('name_' + command.split()[0]).value
            
            # if didn't find the specific installer name uses the default one
            if(not installerName):
                installerName = env._module_name
            
            if(not command):
                selfInstalation = 'false'
            
        else :
            selfInstalation = 'false'
        
        if not env._sudoEnabled :
            raise TaskError('    Module: \"%s\" is required by other modules and is not available on your system.\n' 
                            '    Ask your system admin to install it.\n'
                            '    > More information from the module: \"%s\"' 
                            % (env._module_name, 
                               self.attribute('more_information').value))

        
        errorTmp = None
        sudoer=self.attribute('sudoer_install').value
        if selfInstalation=='true':
            # Try to install if possible
            
            # if should try to install as sudoer
            if sudoer: sudoer = sudoer.lower()
            if(sudoer=='true' and (not env.sudoEnabled)):
                raise TaskError('   Module: \"%s\" requires sudo rights, if' 
                                ' you have the right, call bake with the'
                                ' --sudo option, or ask your system admin'
                                ' to install \"%s\" in your machine.\n'
                                '    > More information from the module: \"%s\"' 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))

            # if the user asked to install everything as sudoer... do it!
            if(env.sudoEnabled):
                command = "sudo "+ command
                command = command

            try:
                env.run((command + installerName).split(" "), 
                        directory=env.srcrepo)
                return True
            except IOError as e:
                errorTmp = ('    Self installation problem for module: \"%s\", ' 
                            'Error: %s' % (env._module_name,  e))
            except TaskError as e1:
                if(env.sudoEnabled):
                    e1.reason = ("    Self installation problem for module: \"%s\", "
                            "\n    Probably either you miss sudo rights or the module is"
                            " not present on your package management databases."
                            "\n    Try to either talk to your system admin or review your "
                            "library database to add \"%s\"\n"
                            "    > More information from the module: \"%s\"" 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))
                else:
                    e1.reason = ("    Self installation problem for module: \"%s\", "
                            "\n    Probably either you need super user rights to install the packet,"
                            "or that the module is"
                            " not present on your package management databases."
                            "\n    Try calling bake with the --sudo option and/or " 
                            "review your library database to add \"%s\"\n"
                            "    > More information from the module: \"%s\"" 
                            % (env._module_name, installerName, 
                               self.attribute('more_information').value))
                raise e1
            
        return True
    
    def update(self, env):
        """Empty, no Update available for system dependency. """       
        pass
    
    def build(self, env):
        """ Empty, no build is possible for system dependencies.""" 
        pass

    def check_version(self, env):
        """Verifies if the right program exists in the system to handle 
        the given compressed source file.
        """
#        
#        distributions = [
#            ['debian', 'apt-get'],
#            ['ubuntu', 'apt-get'],
#            ['fedora', 'yum'],
#            ['redhat', 'yum'],
#            ['centos', 'yum'],
#            ['suse', 'yast'],
#            ['darwin', 'port'],
#            ]
#
        
#        (distribution, version, version_id) = distro.linux_distribution()
#        if not distribution:
#            distribution = 'darwin' # osName
#        else:
#            distribution = distribution.lower()
            
        program = self.source_systemtool()
        
        if not program == '':
            return env.check_program(program)
#        for dist, program in distributions:
#            if distribution.startswith(dist):
#                return env.check_program(program)
        return False


class CvsModuleSource(ModuleSource):
    """Handles the modules that have the sources stored in a CVS repository."""
    
    def __init__(self):
        """ Specific attributes definition."""

        ModuleSource.__init__(self)
        self.add_attribute('root', '', 
                           'Repository root specification to checkout from.',
                           mandatory=True)
        self.add_attribute('module', '', 'Module to checkout.', mandatory=True)
        self.add_attribute('checkout_directory', None, "Name of directory "
                           "checkout defaults to. If unspecified, defaults"
                           " to the name of the module being checked out.")
        self.add_attribute('date', None, 'Date to checkout')

    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        
        return 'cvs'

    def download(self, env):
        """ Downloads the last CVS code, or from a specific date. """
        
        import tempfile
        try:
            tempdir = tempfile.mkdtemp(dir=env.srcrepo)
        except OSError as e:
            raise TaskError('Could not create temporary directory %s, Error: %s' 
                            % (env.srcrepo, e))

        env.run(['cvs', '-d', self.attribute('root').value, 'login'],
                directory=tempdir)
        
        checkout_options = []
        if not self.attribute('date').value is None:
            checkout_options.extend(['-D', self.attribute('date').value])
        env.run(['cvs', '-d', self.attribute('root').value, 'checkout'] + 
                checkout_options + [self.attribute('module').value],
                directory=tempdir)
        
        if self.attribute('checkout_directory').value is not None:
            actual_checkout_dir = self.attribute('checkout_directory').value
        else:
            actual_checkout_dir = self.attribute('module').value
            
        import os
        import shutil
        try:
            os.rename(os.path.join(tempdir, actual_checkout_dir), env.srcdir)
            shutil.rmtree(tempdir)
        except AttributeError as e:
            raise TaskError('Atribute type error expected String, Error: %s' 
                            % e)
        

    def update(self, env):
        """Updates the code for the date specified, or for the today's code. """
        
        # just update does not work, it has to give a date for the update
        # either a date is provided, or takes today as date
        update_options = []
        if not self.attribute('date').value is None:
            update_options.extend(['-D', self.attribute('date').value])
        else:
            update_options.extend(['-D', str(date.today())]) 
        
        env.run(['cvs', 'up'] + update_options, directory=env.srcdir)
        
    def check_version(self, env):
        """ Checks if the tool is available and with the needed version."""

        return env.check_program('cvs')


class GitModuleSource(ModuleSource):
    """Handles the modules that have the sources stored in a git repository."""
    
    def __init__(self):
        ModuleSource.__init__(self)
        self.add_attribute('url', '', 'Url to clone the source tree from.',
                           mandatory=True)
        self.add_attribute('revision', '',
                           "Revision to checkout. Defaults to origin/master"
                           " reference.")
        self.add_attribute('branch', '', 'Branch to checkout.')
        self.add_attribute('fetch_option', '', 'Options to add git fetch command.')
    @classmethod
    def name(cls):
        """ Identifier of the type of the tool used."""
        
        return 'git'

    def download(self, env):
        import tempfile
        import os
        try:
            tempdir = tempfile.mkdtemp(dir=env.srcrepo)
        except AttributeError as e1:
            raise TaskError('Attribute type error, expected String, Error: %s' % e1)
        except OSError as e2:
            raise TaskError('Could not create temporary file, Error: %s' % e2)

        checkedOut = False
            
        env.run(['git', 'init'], directory=tempdir)
        env.run(['git', 'remote', 'add', 'origin', self.attribute('url').value],
                directory=tempdir)
        if self.attribute('fetch_option').value != '':
            env.run(['git', 'fetch', self.attribute('fetch_option').value],
                    directory=tempdir)
        else:
            env.run(['git', 'fetch'], directory=tempdir)

        if self.attribute('branch').value != '':
            env.run(['git', 'checkout', self.attribute('branch').value],
                    directory=tempdir)
            checkedOut = True
        
        if self.attribute('revision').value != '':
            env.run(['git', 'checkout', self.attribute('revision').value],
                    directory=tempdir)
            checkedOut = True

        if not checkedOut:
            env.run(['git', 'pull', 'origin', 'master'], directory=tempdir)

        os.rename(tempdir, env.srcdir)

    def update(self, env):
        """ Updates the code using a specific version from the repository."""

        env.run(['git', 'stash'], directory=env.srcdir)
        env.run(['git', 'rebase', self.attribute('revision').value], directory=env.srcdir)
        try:
            env.run(['git', 'stash','pop'], directory=env.srcdir)
        except TaskError as t:
            if not ' 1' in t.reason:
                raise t
            env._logger.commands.write('  No perceived changes on the local repository.\n')

        
#        env.run(['git', 'fetch'], directory=env.srcdir)
#        env.run(['git', 'checkout', self.attribute('revision').value],
#                          directory=env.srcdir)

    def check_version(self, env):
        """ Checks if the tool is available and with the needed version."""
        return env.check_program('git')
