###############################################################################
# Copyright (c) 2013 INRIA
# Copyright (c) 2019 Mishal Shah (added search, getconf, install options)
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
#          Mishal Shah <shahmishal1998@gmail.com>
###############################################################################
''' 
 Bake.py

 This is the main Bake file, it stores all the classes related to the
 basic Bake operation. The class Bake is responsible to identify and 
 execute the defined options  
'''

import xml.etree.ElementTree as ET
try:
 from xml.etree.ElementTree import ParseError
except ImportError:
 from xml.parsers.expat import ExpatError as ParseError
import sys
import os
import distro
import signal
import copy
import requests
import bake.Utils
from bake.Configuration import Configuration
from bake.ModuleEnvironment import ModuleEnvironment
from bake.ModuleLogger import StdoutModuleLogger, LogfileModuleLogger, LogdirModuleLogger
from optparse import OptionParser
from bake.Dependencies import Dependencies, DependencyUnmet
from bake.Exceptions import MetadataError
from bake.Utils import ColorTool
from bake.Exceptions import TaskError 
from bake.ModuleSource import SystemDependency 
from bake.ModuleBuild import NoneModuleBuild
from bake.Module import ModuleDependency
from bake.ModuleAppStore  import BaseClient
from bake.Constants import *


def signal_handler(signal, frame):
    """ Handles Ctrl+C keyboard interruptions """
    
    print (os.linesep + ' > Bake was aborted! (Ctrl+C)')
    os._exit(130)
        
class MyOptionParser(OptionParser):
    def format_description(self, formatter):
        import os
        import sys
        return self.description % os.path.basename(sys.argv[0])
        

class Bake:
    """ Main Bake class """
    
    main_options = "" 
    
    def __init__(self):
        pass
    
    def _error(self, string):
        """ Handles hard exceptions, the kind of exceptions Bake should not 
        recover from."""

        import sys
        print(' > Error: %s ' % string)
        if Bake.main_options.debug:
            import bake.Utils
            bake.Utils.print_backtrace()           
        else:
            print('   For more information call Bake with --debug and/or'
                  ' -v, -vvv, for full verbose mode (bake --help)')
        sys.exit(1)
        
    def _fix_config(self, config, args):
        """Handles the fix_config command line option. It intends to fix 
        manually changed files and updates the in-use configuration with 
        new values."""
        
        parser = OptionParser(usage='usage: %prog fix-config [options]')
        self._enable_disable_options(parser)
        parser.add_option("-f", "--conffile", action="store", type="string",
                          dest="bakeconf", default="bakeconf.xml",
                          help="The Bake meta-data configuration from where to"
                          " get the updated modules file to use. Default: %default.")
        parser.add_option("--sourcedir", action="store", type="string",
                          dest="sourcedir", default=None,
                          help="The directory where the source code of all modules "
                          "will be downloaded.")
        parser.add_option("-i", "--installdir", action="store", type="string",
                          dest="installdir", default=None,
                          help="The directory where all modules will be installed.")

        parser.add_option("-t", "--target-file", action="store", type="string",
                          dest="targetfile", default=None,
                          help="New target file, if not defined Bake"
                          " overwrites the present configuration file.")

        (options, args_left) = parser.parse_args(args)

        if options.bakeconf == "bakeconf.xml":
            options.bakeconf = self.check_configuration_file(options.bakeconf, False);

        config = self.check_configuration_file(config, True)

        contribconf = []
        try:
            for cfile in os.listdir("contrib"):
                if cfile.endswith(".xml"):
                    contribconf.append("contrib/"+cfile)
        except Exception as e:
            True

        # Stores the present configuration         
        old_config = Configuration(config)
        old_config.read()
        
        if options.targetfile:
            new_config = Configuration(options.targetfile,
                                   relative_directory_root=old_config.get_relative_directory_root())
        else:
            new_config = Configuration(config,
                                   relative_directory_root=old_config.get_relative_directory_root())
        
            
        try:
            new_config.read_metadata(options.bakeconf)
        except Exception as e:
            self._error('Problem reading Configuration file "%s" \n Error: %s'  % (options.bakeconf, str(e)))

        for cconf in contribconf:
            try:
                new_config.read_metadata(cconf)
            except Exception as e:
                self._error('Problem reading Configuration file "%s" \n Error: %s'  % (cconf, str(e)))
        
        # Checks if the directories where set and if so set the new config file
        # with the new parameters, or let the old ones
        if options.installdir:
            new_config.set_installdir(options.installdir)
        else:
            new_config.set_installdir(old_config.get_installdir())
        if options.sourcedir:
            new_config.set_sourcedir(options.sourcedir)
        else:    
            new_config.set_sourcedir(old_config.get_sourcedir())

        # copy installed files.
        for old_module in old_config.modules():
            new_module = new_config.lookup(old_module.name())
            if new_module is None:
                # ignore old modules that do not exist in the new configuration
                continue
            new_module.installed = old_module.installed

        # copy which modules are enabled into new config
        for old_module in old_config.enabled():
            new_module = new_config.lookup(old_module.name())
            if new_module is None:
                # ignore old enabled modules that do not exist in the new configuration
                continue
            new_config.enable(new_module)

        # copy which modules are disabled into new config
        for old_module in old_config.disabled():
            new_module = new_config.lookup(old_module.name())
            if new_module is None:
                # ignore old disabled modules that do not exist in the new configuration
                continue
            new_config.disable(new_module)

        # now, parse new enabled/disabled options
        self._parse_enable_disable(options, new_config)

        # copy old variables into new config for all modules
        for old_module in old_config.modules():
            new_module = new_config.lookup(old_module.name())
            if new_module is None:
                # ignore old modules that do not exist in the new configuration
                continue
            old_build = old_module.get_build()
            new_build = new_module.get_build()
            for old_attribute in old_build.attributes():
                if new_build.attribute(old_attribute.value) is None:
                    continue
                new_build.attribute(old_attribute.name).value = old_attribute.value

        new_config.write()

    def _enable_disable_options(self, parser):
        """ Allows the parser to recognize --enable and --disable options."""

        parser.add_option("-e", "--enable", action="append", type="string", 
                          dest="enable", default=[],
                          help="A module to enable in the Bake configuration")
        parser.add_option("-d", "--disable", action="append", type="string", 
                          dest="disable", default=[],
                          help="A module to disable in the Bake configuration")
        parser.add_option("-a", "--enable-all", action="store_true",
                          dest="enable_all", default=None,
                          help="Enable all modules.")
        parser.add_option("-m", "--enable-minimal", action="store_true",
                          dest="enable_minimal", default=None,
                          help="Disable all non-mandatory dependencies.")

    def resolve_contrib_dependencies (self, module, fmod, configuration):
        """ Handles the contrib type dependencies"""
        for dep in module.dependencies ():
            dep_mod = configuration.lookup (dep._name)
            if dep_mod.mtype() == "ns-contrib":
                # Do not prepend contrib prefix to user supplied contrib name more than once
                if not(module.get_source().attribute("module_directory").value.startswith(fmod + '/contrib')):
                    dep_mod.get_source().attribute("module_directory").value = fmod+'/contrib/'+dep_mod.get_source().attribute("module_directory").value
                dep_mod.addDependencies(ModuleDependency(fmod, False))
                self.resolve_contrib_dependencies (dep_mod, fmod, configuration)

    def _enable(self, enable, configuration):
        """ Handles the --enable option, setting defined modules as enable."""
        for module_name in enable:
            module = configuration.lookup(module_name)
            if not module:
                self._error('Module "%s" not found' % module_name)
            if module.mtype() == "ns-contrib":
                found=0
                fmod = None
                for mod in enable:
                    if configuration.lookup(mod).mtype() == "ns" and ((mod>=module.minver() and (module.maxver() is None or mod<=module.maxver())) or (mod == "ns-3-dev" and module.maxver() is None)):
                        found+= 1
                        fmod = mod
                if not found==1:
                    self._error('Module "%s" has unmet dependency: %s' % (module_name, module.minver()))

                # Do not prepend contrib prefix to user supplied contrib name more than once
                if not(module.get_source().attribute("module_directory").value.startswith(fmod + '/contrib')):
                    module.get_source().attribute("module_directory").value = fmod+'/contrib/'+module.get_source().attribute("module_directory").value
                    
                module.addDependencies(ModuleDependency(fmod, False))
                self.resolve_contrib_dependencies (module, fmod, configuration)
            configuration.enable(module)

    def _disable(self, disable, configuration):
        """ Handles the --disable option, setting the defined modules as disable."""

        for module_name in disable:
            module = configuration.lookup(module_name)
            if not module:
                self._error('Module "%s" not found' % module_name)
            configuration.disable(module)
            if module.mtype() == "ns":
                enabled_list = configuration.enabled()
                for mod in enabled_list:
                    if mod.mtype() == "ns-contrib":
                        configuration.disable(mod)

    def _variables_process(self, items, configuration, is_append):
        """ Handles the defined configured variables ."""
        
        for module_name, name, value in items:
            if module_name:
                module = configuration.lookup(module_name)
                if not module:
                    self._error('Module "%s" not found' % module_name)
                if not module.get_build().attribute(name):
                    self._error('Module "%s" has no attribute "%s"' % 
                                (module_name, name))
                if is_append:
                    module.get_build().attribute(name).value = \
                        module.get_build().attribute(name).value + ' ' + value
                else:
                    module.get_build().attribute(name).value = value
            else:
                for module in configuration.modules():
                    if module.get_build().attribute(name):
                        if is_append and module.get_build().attribute(name).value :
                            module.get_build().attribute(name).value = \
                                module.get_build().attribute(name).value + ' ' + value
                        else:
                            module.get_build().attribute(name).value = value
        
    def _parse_enable_disable(self, options, configuration):
        """ Identify the enabled and disabled options passed as parameters 
        in the configuration.
        """
        
        # enables/disables the explicit enable/disable modules passed as argument
        self._enable(options.enable, configuration)
        for mod in options.disable:
            if not mod in options.enable:
                self._error('Module "%s" not enabled' % mod)
        self._disable(options.disable, configuration)
        
        # if the option -a is used, meaning all the modules should be enabled
        if options.enable_all:
            for module in configuration.modules():
                configuration.enable(module)
            
                
        # if the option -m is used, meaning the minimum configuration should be used
        # it disables all the non mandatory dependencies
        if options.enable_minimal:
            enabled = []
            def _enabled_iterator(module):
                """ Assigns the module as enabled."""    
                enabled.append(module)
                return True
            
            self._iterate(configuration, _enabled_iterator,
                          configuration.enabled(),
                          follow_optional=True)
            enabled_optional = []
            def _enabled_optional_iterator(module):
                enabled_optional.append(module)
                return True
            self._iterate(configuration, _enabled_optional_iterator,
                          configuration.enabled(),
                          follow_optional=False)
            for module in enabled:
                if not module in enabled_optional:
                    configuration.disable(module)

    def _parse_variable(self, string, configuration):
        """ Verifies if the module and requested attribute exists."""
        
        retval = []
        data = string.split(":")
        
        # if it is an setting for all the modules that contains such variable
        if len(data) == 1:
            name, value = string.split("=")
            for module in configuration.modules():
                if module.get_build().attribute(name):
                    retval.append((module, name, value))
            if not retval:
                print ('Error: no module contains variable %s' % name)
        # if it is a setting for a specific module 
        elif len(data) == 2:
            name, value = data[1].split("=")
            module = configuration.lookup(data[0])
            if not module:
                self._error('non-existing module %s in variable'
                            ' specification %s' % (name, string))
            if not module.get_build().attribute(name):
                self._error('non-existing variable %s in module %s' % 
                            (name, module._name))
            retval.append((module, name, value))
        # if the variable is set incorrectly 
        else:
            self._error('invalid variable specification: "%s"' % string)
        return retval
    
    def _read_resource_file(self, configuration):
        """ Reads the predefined elements on the uer's resource file."""
        
        rcPredefined = []
        fileName = os.path.join(os.path.expanduser("~"), ".bakerc")
        
        if os.path.isfile(fileName):
            rcPredefined = configuration.read_predefined(fileName)
    
        return rcPredefined

    def _get_predefined(self, configuration):
        """ Gets the values of enable and disable as a predefined setting."""
        
        predefined =ET.Element('predefined', {'name':'last'})
        for e in configuration._enabled:
            enable_node = ET.Element('enable', {'name':e.name()})
            predefined.append(enable_node)
        
        for e in configuration._disabled:
            enable_node = ET.Element('disable', {'name':e.name()})
            predefined.append(enable_node)
        return predefined


    def save_resource_file(self, configuration, fileName):
        """ Saves the pretty resource file."""
        
        try:
            fout = open(fileName, "w")
            fout.write(bake.Utils.prettify(configuration))
            fout.close()
        except IOError as e:
            ""
            # print ('Problems writing the  resource file, error: %s' % e)

    def _save_resource_configuration(self, configuration):
        """ Saves the last call to the predefined elements on the 
        user's resource file.
        """
        
        allPredefined = []
        fileName = os.path.join(os.path.expanduser("~"), ".bakerc")
        lastConfig = self._get_predefined(configuration)
        
        if os.path.isfile(fileName):
            try:
                et = ET.parse(fileName)
                root = et.getroot()
                for element in root.findall('predefined'):
                    if element.attrib['name'] == "last":
                        root.remove(element)
                        break
            
                root.append(lastConfig)
                self.save_resource_file(root, fileName)
                return
            except ParseError as e :
                print ('Problems reading the resource file, error: %s'% e)
            
        # There is no configuration file, so wee need to create one
        configuration = ET.Element('configuration', {})      
        configuration.append(lastConfig)
        self.save_resource_file(configuration, fileName)

    def _list(self, config, args):
        """ Handles the list option for %prog """
        
        # sets the options the parser should recognize for the configuration
        parser = OptionParser(usage='usage: %prog list [options]')
        parser.add_option("-f", "--conffile", action="store", type="string",
                          dest="bakeconf", default="bakeconf.xml",
                          help="The Bake meta-data configuration file to use. "
                          "Default: %default.")
        parser.add_option("-c", "--contrib", action="store_true",
                          dest="contrib", default="False",
                          help="Show only contrib modules.")
        (options, args_left) = parser.parse_args(args)
        listconf = Configuration(config)
        contrib_list = []
        module_list = []

        contribconf = []
        try:
            for cfile in os.listdir("contrib"):
                if cfile.endswith(".xml"):
                    contribconf.append("contrib/"+cfile)
        except Exception as e:
            True

        try:
            listconf.read_metadata(options.bakeconf)
        except Exception as e:
            self._error('Problem reading Configuration file "%s" \n Error: %s'  % (options.bakeconf, str(e)))

        for cconf in contribconf:
            try:
                listconf.read_metadata(cconf)
            except Exception as e:
                self._error('Problem reading Configuration file "%s" \n Error: %s'  % (cconf, str(e)))

        for mod in listconf.modules():
            if mod.mtype() == "ns-contrib":
                contrib_list.append(mod.name())
            elif not options.contrib == True:
                module_list.append(mod.name())

        contrib_list.sort()
        module_list.sort()
        for m in module_list:
            print("module: "+m)
        for c in contrib_list:
            print("contrib: "+c)
        
    def _configure(self, config, args):
        """ Handles the configuration option for %prog """
        
        # sets the options the parser should recognize for the configuration
        parser = OptionParser(usage='usage: %prog configure [options]')
        self._enable_disable_options(parser)
        parser.add_option("-f", "--conffile", action="store", type="string",
                          dest="bakeconf", default="bakeconf.xml",
                          help="The Bake meta-data configuration file to use. "
                          "Default: %default.")
        parser.add_option("-g", "--gui", action="store_true",
                          dest="gui", default="False",
                          help="Use a GUI to define the configuration.")
        parser.add_option("-s", "--set", action="append", type="string", 
                          dest="set",
                          default=[],
                          help="Format: module:name=value. A variable to set"
                          " in the Bake configuration for the matching module.")
        parser.add_option("--append", action="append", type="string", 
                          dest="append", default=[],
                          help="Format: module:name=value. A variable to"
                          " append to in the Bake build "
                          "configuration for the especified module.")
        parser.add_option("--sourcedir", action="store", type="string",
                          dest="sourcedir", default="source",
                          help="The directory where the source code of all modules "
                          "will be downloaded.")
        parser.add_option("-i", "--installdir", action="store", type="string",
                          dest="installdir", default="build",
                          help="The directory where all modules will be installed.")
        parser.add_option("-p", "--predefined", action="store", type="string",
                          dest="predefined", default=None,
                          help="A predefined configuration to apply")

        parser.add_option('--logfile', help='File in which we want to store log output '
                          'of requested operation', action="store", type="string", dest="logfile",
                          default='')
        parser.add_option('--logdir', help='Directory in which we want to store log output '
                          'of requested operation. One file per module.', action="store",
                          type="string", dest="logdir",
                          default='')
        parser.add_option('-v', '--verbose', action='count', dest='verbose', 
                          default=0, help='Increase the log verbosity level')
        parser.add_option('-q', '--quiet', action='count', dest='quiet', 
                          default=0, help='Increase the log quietness level')
        parser.add_option("-c", "--clean", action="store_true",
                          dest="remove", default=False,
                          help="Remove all enabled modules")

        # sets the configuration values got from the line command
        (options, args_left) = parser.parse_args(args)
        if options.bakeconf == "bakeconf.xml":
            options.bakeconf = self.check_configuration_file(options.bakeconf, False);

        contribconf = []
        try:
            for cfile in os.listdir("contrib"):
                if cfile.endswith(".xml"):
                    contribconf.append("contrib/"+cfile)
        except Exception as e:
            True

        configuration = Configuration(config)

        if not options.remove:
            try:
                configuration.read()
                for m in  configuration.enabled():
                    if m.name() not in options.enable:
                        options.enable.append(m.name())
            except Exception as e:
                True
       
        try:
            configuration.read_metadata(options.bakeconf)
        except Exception as e:
            self._error('Problem reading Configuration file "%s" \n Error: %s'  % (options.bakeconf, str(e)))

        for cconf in contribconf:
            try:
                configuration.read_metadata(cconf)
            except Exception as e:
                self._error('Problem reading Configuration file "%s" \n Error: %s'  % (cconf, str(e)))
                   
        configuration.set_sourcedir(options.sourcedir)
        configuration.set_installdir(options.installdir)
        
        # if used the predefined settings, reads the predefined configuration
        if options.predefined:
            data = options.predefined.split(':')
            requested = None
            predefined = configuration.read_predefined(options.bakeconf)
            
            # if the user has a bake configuration
            rcPredefined = self._read_resource_file(configuration)
            predefined = rcPredefined + predefined
            
                 
            if len(data) == 1:
                requested = data[0]
            elif len(data) == 2:
                predefined += configuration.read_predefined(data[0])
                requested = data[1]
            else:
                self._error('Invalid --predefined content: "%s"' % predefined)
            for p in requested.split(','):
                found = False
                for predef in predefined:
                    if predef.name == p:
                        found = True
                        self._enable(predef.enable, configuration)
                        self._disable(predef.disable, configuration)
                        self._variables_process(predef.variables_set, 
                                                configuration, is_append=False)
                        self._variables_process(predef.variables_append, 
                                                configuration, is_append=True)
                        directories = predef.directories
                        if 'sourcedir' in directories:
                            configuration.set_sourcedir(directories['sourcedir'])
                        if 'installdir' in directories:
                            configuration.set_installdir(directories['installdir'])
                        break
                if not found:
                    self._error('--predefined: "%s" not found.' % p)
                    
        # Registers the modules are that enabled/disabled 
        # handles the -a, -m, --disable, --enable tags           
        self._parse_enable_disable(options, configuration)

        # handles the set command line option, to overwrite the specific 
        # module setting with the new specified value
        for variable in options.set:
            matches = self._parse_variable(variable, configuration)
            for module, name, value in matches:
                module.get_build().attribute(name).value = value

        # handles the append command line option, to add the new 
        # value to the module setting
        for variable in options.append:
            matches = self._parse_variable(variable, configuration)
            for module, name, value in matches:
                current_value = module.get_build().attribute(name).value
                module.get_build().attribute(name).value = current_value + ' ' + value
        configuration.write()
        
        if not configuration._enabled and not options.append and not options.remove:
            env =  self._get_dummy_env(options)
            env._logger.commands.write(' > No module enabled: Bake configuration requires at least one module to be enabled'
                                       ' (enable, predefined), or appended.\n'
                                       '   Argument(s) %s is not enough for an unambiguous action.\n' % (args_left))
            self._error('No module enabled, please use -e <name of the module>, -p <predefined modules> or -a, to activate all modules.')

        self._save_resource_configuration(configuration)
        
        
    dependencyChain=None
    def _iterate(self, configuration, functor, targets, follow_optional=True):
        """Iterates over the configuration modules applying the functor 
        function and solve reminding dependencies.
        """
        
        deps = Dependencies()
        
        # execute just one time to get the optional dependencies chain
        if not self.dependencyChain:
            deps.checkDependencies(targets,configuration.modules())
            self.dependencyChain = deps.dependencies
        else :
            deps.dependencies= self.dependencyChain
#        
#        
        
        class Wrapper:
            def __init__(self, module):
                self._module = module
            def function(self):
                retval = functor(self._module)
                configuration.write()
                return retval
        # for all the modules saves the configuration
        for m in configuration.modules():
            wrapper = Wrapper(m)
            deps.add_dst(m, wrapper.function)
        # Review the dependencies of all the configured modules
        for m in configuration.modules():
            for dependency in m.dependencies():                
                src = configuration.lookup (dependency._name)
                
                # verifies if the dependency really exists in the configuration
                # if not we could have a problem of a corrupt, or badly 
                # configured xml file, e.g. misspelled module name  
                if src is None:
                    self._error('Dependency "%s" not found' % dependency._name)
                 
                if not src in configuration.disabled():
                    # if it is set to add even the optional modules, or the 
                    # dependency is not optional, add the module it depends on 
                    # as a dependency 
                    if follow_optional or not dependency.is_optional():
                        deps.add_dep(src, m, optional=dependency.is_optional())
                        
        try:
            deps.resolve(targets)
#            deps.dump2(sys.stdout)
        except DependencyUnmet as error:
            if not error.method() =='':
                errorAppend = ' ' + error.method()
            else:
                 errorAppend = ' failed'
               
            self._error(' Critical dependency, module "' + error.failed().name()+'"' + errorAppend)

    def _read_config(self, config, directory=None):
        """Reads the configuration file."""

        configuration = Configuration(config, directory)
        if not configuration.read():
            sys.stderr.write('The configuration file has been changed or has moved.\n'
                             'Running \'fix-config\'. You should consider running it\n'
                             'yourself to tweak some parameters if needed.\n')
            self._fix_config(config, [])
            print(">> " + config)
            configuration = Configuration(config)
            if not configuration.read():
                self._error('Oops. \'fix-config\' did not succeed. You should consider\n'
                            'deleting your bakefile and running \'configure\' again.')

        return configuration

    def _option_parser(self, operation_name):
        """Adds generic options to the options parser. Receives the name of the 
        present option as parameter.
        """
        
        parser = OptionParser(usage='usage: %prog ' + operation_name + ' [options]')
        parser.add_option('--logfile', help='File in which we want to store log output '
                          'of requested operation', action="store", type="string", dest="logfile",
                          default='')
        parser.add_option('--logdir', help='Directory in which we want to store log output '
                          'of requested operation. One file per module.', action="store",
                          type="string", dest="logdir",
                          default='')
        parser.add_option('-v', '--verbose', action='count', dest='verbose', 
                          default=0, help='Increase the log verbosity level')
        parser.add_option('-q', '--quiet', action='count', dest='quiet', 
                          default=0, help='Increase the log quietness level')
        parser.add_option("-o", "--one", action="store", type="string",
                          dest="one", default="",
                          help="Process only the module specified.")
        parser.add_option("-a", "--all", action="store_true",
                          dest="all", default=False,
                          help="Process all modules")
        parser.add_option("--stop-on-error", action="store_true", 
                          dest="stopOnError", default=False,
                          help="Stop on the first error found and do not advance while the error is not corrected.")
        parser.add_option("-s", "--start", action="store", type="string",
                          dest="start", default="",
                          help="Process all modules enabled starting from the module specified.")
        parser.add_option("--after", action="store", type="string",
                          dest="after", default="",
                          help="Process all modules enabled starting after the module specified.")
        parser.add_option("-i", "--environment-file-identification", 
                          action="store", type="string",
                          dest="environment_file_identification", 
                          default="bakeSetEnv.sh",
                          help="Name of the environment setting file")
        parser.add_option("-x", "--no-environment-file", action='store_true', 
                          dest='no_environment_file', default=False,
                          help='Do not create the environment file for this run')
        parser.add_option("--sudo", action="store_true",
                          dest="call_with_sudo", default=False, 
                          help='Best effort attempt to install dependencies and modules, when'
                          ' required,  using sudo. The user has to have sudo rights (be careful using it).')
      
        return parser


    def createEnvironment(self, config, options, directory=None):
        """ Auxiliary function to create an instance of the module environment"""
         
        configuration = self._read_config(config, directory)
        if options.logdir == '' and options.logfile == '':
            logger = StdoutModuleLogger()
        elif options.logdir != '':
            assert options.logfile == ''
            logger = LogdirModuleLogger(options.logdir)
        else:
            assert options.logfile != ''
            logger = LogfileModuleLogger(options.logfile)
        verbose = options.verbose - options.quiet
        verbose = verbose if verbose >= 0 else 0
        logger.set_verbose(verbose)
        env = ModuleEnvironment(logger, 
            configuration.compute_installdir(), 
            configuration.compute_sourcedir(), 
            Bake.main_options.debug)
        return configuration, env

    def _do_operation(self, config, options, functor, directory=None):
        """Applies the function, passed as parameter, over the options."""
        
        configuration, env = self.createEnvironment(config, options, directory)
        must_disable = []
        if options.one != '':
            if options.all or options.start != '' or options.after != '':
                self._error('incompatible options')
            module = configuration.lookup(options.one)
            functor(configuration, module, env)
            configuration.write()
        elif options.all:
            if options.start != '' or options.after != '':
                self._error('incompatible options')
            def _iterator(module):
                return functor (configuration, module, env)
            self._iterate(configuration, _iterator, configuration.modules())
        elif options.start != '':
            if options.after != '':
                self._error('incompatible options')
            must_process = []
            first_module = configuration.lookup(options.start)
            def _iterator(module):
                if module == first_module:
                    must_process.append(0)
                if len(must_process) != 0:
                    return functor (configuration, module, env)
                else:
                    return True
            self._iterate(configuration, _iterator, configuration.enabled())
        elif options.after != '':
            # this is a list because the inner function below
            # is not allowed to modify the outer function reference
            must_process = [] 
            first_module = configuration.lookup(options.after)
            def _iterator(module):
                if len(must_process) != 0:
                    return functor (configuration, module, env)
                elif module == first_module:
                    must_process.append(1)
                return True
            self._iterate(configuration, _iterator, configuration.enabled())
        else:
            def _iterator(module):
                return functor (configuration, module, env)
            self._iterate(configuration, _iterator, configuration.enabled())
        return env

    def _get_enabled_ns(self, config):
        """ returns the enabled ns versions"""
        config = self.check_configuration_file(config, True);

        import os
        if os.path.isfile(config):
            configuration = self._read_config(config)
        else:
            print(" > Couldn't find the " + config + " configuration file. \n"
                  "   Call bake with -f [full path configuration file name].\n")
            return
   
        enabled = []
        def _iterator(module):
            if module.mtype()=="ns":
                enabled.append(module.name())
            return True

        self._iterate(configuration, _iterator, configuration.enabled())

        return enabled

    def _search(self, config, args):
        """Handles the search command line option"""
        parser = self._option_parser('search')

        (options, args_left) = parser.parse_args(args)

        ns_enabled = self._get_enabled_ns(config)

        logger = StdoutModuleLogger()
        webclient = BaseClient(logger, SEARCH_API)
        # lists all the apps from the AppStore
        if len(args_left) == 0:
            response = webclient.search_api()
            for app in response:
                sys.stdout.write(app['name'] + " (" + app['app_type'] + ") - " + app['abstract'] + "\n")
                sys.stdout.flush()
        # lists the apps matching the substring from the AppStore
        elif len(args_left) == 1:
            response = webclient.search_api(args_left[0], ns_enabled)
            for app in response:
                sys.stdout.write(app['app']['name'] + " (" + app['version'] + ") - " + app['app']['abstract'] + "\n")
                sys.stdout.flush()
        else:
            self._error("Please provide only one parameter to search for an app")

    
    def _deploy(self, config, args):
        """Handles the deploy command line option."""

        print("Downloading, building and installing the selected modules and dependencies.")
        print("Please, be patient, this may take a while!")
        returnValue = self._download(config, args);
        if not returnValue:
            return self._build(config, args)


    def _getconf(self, config, args):
        """Handles the getconf command line option"""
        parser = self._option_parser('getconf')
        (options, args_left) = parser.parse_args(args)

        ns_enabled = self._get_enabled_ns(config)

        logger = StdoutModuleLogger()
        webclient = BaseClient(logger, INSTALL_API, "http://localhost:8000")

        if len(args_left) == 1 and ns_enabled is not None:
            argument_module = args_left[0]
            if len(argument_module.split("=="))==2:
                sys.stdout.write("Collecting " + argument_module + "\n")
                module_name, version = argument_module.split("==")[0], argument_module.split("==")[1]
                response, bakefile_obj = webclient.install_api(module_name, version, ns=ns_enabled)
            elif len(argument_module.split("="))==2:
                self._error('Invalid requirement: %s \n\
                = is not a valid operator. Did you mean == ?'  % args_left[0])
            elif len(argument_module.split("=="))==1:
                sys.stdout.write("Collecting " + argument_module + "\n")
                module_name = args_left[0]
                response, bakefile_obj = webclient.install_api(module_name, ns=ns_enabled)

            # Create contrib directory if it does not exist
            try:
                os.mkdir('contrib')
            except:
                pass
            # download the bakeconf xml file in contrib directory
            with open("contrib/" + response['name'] + "-" + response['version'] + ".xml", 'wb') as f:
                f.write(bakefile_obj.content)

            return response
        else:
            self._error("ns not configured")


    def _install(self, config, args):
        """Handles the install command line option"""
        parser = self._option_parser('install')
        (options, args_left) = parser.parse_args(args)
        res = self._getconf(config, args)
        # Take an input of modules to enable/disable/enable-minimal
        arguments_for_configure = []
        arguments_for_configure.append("-e")
        arguments_for_configure.append("ns-" + res['ns'])
        arguments_for_configure.append("-e")
        arguments_for_configure.append(res['name'])

        ## Ask if the non-mandatory dependecies are to be disabled
        enable_minimal = raw_input("Disable all non-mandatory dependencies? (Y/n): ")
        if enable_minimal.lower() == 'y':
            arguments_for_configure.append("-m")

        configureResult = self._configure(config, arguments_for_configure)
        if not configureResult:
            self._deploy(config, [])

    
    def _download(self, config, args):
        """Handles the download command line option."""

        parser = self._option_parser('download')
        parser.add_option("--force_download", action='store_true', 
                          dest='force_download', default=False,
                          help='Force the download of all modules again')

        
        (options, args_left) = parser.parse_args(args)
        def _do_download(configuration, module, env):
            
            if module._source.name() == 'none':
                return True  

            dependencyExists = False
            if isinstance(module._source, SystemDependency):  
                
                sys.stdout.write (" >> Searching for system dependency " + module.name() + " - ")
                sys.stdout.flush()
                # We support one of the following three attributes:
                # file_test, executable_test, and dependency_test (deprecated)
                if (module._source.attribute('file_test').value is not None):
                    dependencyExists = module._source._check_file_expression (
                      module._source.attribute('file_test').value)
                elif (module._source.attribute('executable_test').value is not None):
                    dependencyExists = module._source._check_executable_expression (
                      module._source.attribute('executable_test').value)
                # XXX Deprecated attribute; will be removed in future
                elif (module._source.attribute('dependency_test').value is not None):
                    dependencyExists = module._source._check_dependency_expression(env,
                      module._source.attribute('dependency_test').value)
                elif (module._source.attribute('import_test').value is not None):
                    dependencyExists = module._source._check_import(
                      module._source.attribute('import_test').value)
                
                # if the dependency exists there is nothing else to do
                if (dependencyExists) :
                    env.start_source(module.name(), ".")
                    module.printResult(env, "Search", module.OK)
                    env.end_source()
                    return True

            if not dependencyExists:
                # Dependency did not exist
                targetDir=''
                if module._source.attribute('module_directory') and not module._source.attribute('module_directory').value.strip() =='':
                    targetDir=' (target directory:%s)'%module._source.attribute('module_directory').value
                
                if not isinstance(module._source, SystemDependency):
                    sys.stdout.write (" >> Downloading " + module.name() + targetDir + " - ")
                sys.stdout.flush()
                if env._logger._verbose > 0:
                    print()

                env._sudoEnabled=options.call_with_sudo
                ModuleEnvironment._stopOnError=options.stopOnError
                valueToReturn=module.check_source_version(env)

            
                if valueToReturn: 
                    return module.download(env, options.force_download)
                else:
                    if isinstance(module._source, SystemDependency):
                        module.printResult(env, "Dependency ", module.FAIL)
                    else:
                        module.printResult(env, "Download", module.FAIL)
               
                    if isinstance(module._source, SystemDependency):
                        env._logger.commands.write(' Module: \"%s\" is required by other modules but it is not available on your system.\n' 
                                        '     Ask your system admin or review your library database to add \"%s\"\n'
                                        '     More information from the module: \"%s\"\n'% (module.name(), module.name(),
                               module._source.attribute('more_information').value))
                        return False

                    else:
                        tool = module._source.name()
                        raise TaskError('    Unavailable Downloading tool %s'
                                ' for module "%s". Try to call \"%s check\"\n' % 
                                (tool, module.name(), 
                                 os.path.basename(sys.argv[0])))
        self._do_operation(config, options, _do_download)

    def _update(self, config, args):
        """Handles the update command line option."""

        parser = self._option_parser('update')
        (options, args_left) = parser.parse_args(args)
        self._check_source_version(config, options)

        
        def _do_update(configuration, module, env):
            if module._source.name() == 'none':
                return True  

            targetDir=''
            if module._source.attribute('module_directory') and not module._source.attribute('module_directory').value.strip() =='':
                targetDir=' (target directory:%s)'%module._source.attribute('module_directory').value

            if not isinstance(module._source, SystemDependency):
                sys.stdout.write (" >> Updating " + module.name() + targetDir + " - ")
            sys.stdout.flush()
            if env._logger._verbose > 0:
                print()
                
            return module.update(env)

        self._do_operation(config, options, _do_update)

    def _check_build_version(self, config, options):
        """Checks if the required build tools are available in the machine."""
        
        def _do_check(configuration, module, env):
            if not module.check_build_version(env):
                env._logger.commands.write('    Unavailable building tool for'
                                            ' module "%s"\n' % module.name())
                return False
            return True
        self._do_operation(config, options, _do_check)


    def _check_source_version(self, config, options):
        """Checks if the source can be handled by the programs in the machine."""
        okForTool=True
        def _do_check(configuration, module, env):
            if not module.check_source_version(env):
                env._logger.commands.write('    Unavailable source tool'
                                            ' for module %s\n' % module.name())
                okForTool=False
                return False
            return True
        self._do_operation(config, options, _do_check)
        return okForTool

    def _check_source_code(self, config, options, directory=None):
        """ Checks if we have already downloaded the matching source code."""
        
        def _do_check(configuration, module, env):
            if not module.is_downloaded(env):
                env._logger.commands.write('    Unavailable source code for'
                                            ' module %s. Try %s download first.\n'
                                             %(module.name(), sys.argv[0]))
                return False
            return True
        self._do_operation(config, options, _do_check, directory)


    def _build(self, config, args):
        """Handles the build command line option."""
        
        parser = self._option_parser('build')
        parser.add_option('-j', '--jobs', help='Allow N jobs at once.'
                          ,type='int', action='store', 
                          dest='jobs', default=-1)
        parser.add_option('--force-clean', help='Forces the call of the clean'
                          ' option for the build.', action="store_true", 
                          default=False, dest='force_clean')
        (options, args_left) = parser.parse_args(args)
        #self._check_build_version(config, options)
        self._check_source_code(config, options)
        
        def _do_build(configuration, module, env):
            
            if isinstance(module._source, SystemDependency) or isinstance(module._build, NoneModuleBuild) :
                if isinstance(module._build, NoneModuleBuild):
                    # Only to threat the variables and pre and post instalation
                    # that may be set even for none build kind of modules
                    module.build(env, options.jobs, options.force_clean)
                return True
            
            sys.stdout.write(" >> Building " + module.name()  + " - ")

            sys.stdout.flush()                
            if env._logger._verbose > 0:
                print

            env._sudoEnabled=options.call_with_sudo
            ModuleEnvironment._stopOnError=options.stopOnError
                
            if module.check_build_version(env):
                retval = module.build(env, options.jobs, options.force_clean)
                if retval:
                    module.update_libpath(env)
                return retval
            else:
                module.printResult(env, "Building", module.FAIL)
                print("   >> Unavailable building tool for module %s, install %s" 
                      %(module.name(),module._build.name()))

        env = self._do_operation(config, options, _do_build)
        
        if not options.no_environment_file:
            env.create_environment_file(options.environment_file_identification)

    def _clean(self, config, args):
        """Handles the clean command line option."""
        
        parser = self._option_parser('clean')
        (options, args_left) = parser.parse_args(args)
        self._check_build_version(config, options)
        
        def _do_clean(configuration, module, env):
            if isinstance(module._source, SystemDependency) or isinstance(module._build, NoneModuleBuild):
                return True

            sys.stdout.write(" >> Clean " + module.name()  + " - ")
            module.clean(env)
            return True
        self._do_operation(config, options, _do_clean)

    def _distclean(self, config, args):
        """Handles the distclean command line option."""
        
        parser = self._option_parser('distclean')
        (options, args_left) = parser.parse_args(args)


        def _do_distclean(configuration, module, env):
            if isinstance(module._source, SystemDependency) or isinstance(module._build, NoneModuleBuild):
                return True

            sys.stdout.write(" >> Distribution clean " + module.name()  + " - ")
            returnValue = module.distclean(env)
            return True
        self._do_operation(config, options, _do_distclean)

    def _fullclean(self, config, args):
        """Handles the fullclean command line option."""
        
        parser = self._option_parser('fullclean')
        (options, args_left) = parser.parse_args(args)

        def _do_fullclean(configuration, module, env):
            if isinstance(module._source, SystemDependency) or isinstance(module._build, NoneModuleBuild):
                return True

            returnValue = module.fullclean(env)
            return returnValue
        self._do_operation(config, options, _do_fullclean)

    def _uninstall(self, config, args):
        """Handles the uninstall command line option."""
        
        parser = self._option_parser('uninstall')
        (options, args_left) = parser.parse_args(args)
        def _do_uninstall(configuration, module, env):
            sys.stdout.write(" >> Uninstall " + module.name()  + " - ")
            module.uninstall(env)
            return True
        self._do_operation(config, options, _do_uninstall)

    def _shell(self, config, args):
        """Handles the shell command line option."""
        
        parser = self._option_parser('build')
        (options, args_left) = parser.parse_args(args)
        
        def _do_env_update(configuration, module, env):
            module.update_libpath(env)
            return True
        env = self._do_operation(config, options, _do_env_update)
        import os
        env.run([os.environ['SHELL']], directory=env.objdir, interactive=True)

    def _check(self, config, args):
        """Handles the check command line option."""
        
        checkPrograms = [['python3', 'Python3'],
                         ['git', 'Git'],
                         ['tar', 'Tar tool'],
                         ['unzip', 'Unzip tool'],
                         ['make', 'Make'],
                         ['cmake', 'CMake'],
                         ['patch', 'patch tool'],
                         ]
        if sys.platform == 'darwin':
            checkPrograms.insert(1,['clang++', 'Clang C++ compiler'])
        else:
            checkPrograms.insert(1,['g++', 'GNU C++ compiler'])
        parser = self._option_parser('build')
        (options, args_left) = parser.parse_args(args)
        def _do_env_check(configuration, module, env):
            return True
        
        env = self._get_dummy_env(options)
        colorTool = ColorTool()
        for element in checkPrograms:
            if env.check_program(element[0]):
                colorTool.cPrintln(colorTool.OK, " > " + element[1] + " - OK")                    
            else:
                colorTool.cPrintln(colorTool.WARNING, " > " + element[1] + 
                                 " - is missing")
        print ('\n')
        colorTool.cPrint(colorTool.OK, " > Path searched for tools:")
        for item in env.path_list():
            sys.stdout.write (' ' + item)
            sys.stdout.flush()
        print ('\n')
          
    def _get_dummy_env(self, options):
        """ Returns a dummy environment just for verifying the user's system configuration. """
        configuration = Configuration("")
                        
        if options.logdir == '' and options.logfile == '':
            logger = StdoutModuleLogger()
        elif options.logdir != '':
            assert options.logfile == ''
            logger = LogdirModuleLogger(options.logdir)
        else:
            assert options.logfile != ''
            logger = LogfileModuleLogger(options.logfile)
        verbose = options.verbose - options.quiet
        verbose = verbose if verbose >= 0 else 0
        logger.set_verbose(verbose)
        logger._update_file(logger._file)

        return ModuleEnvironment(logger, "","", Bake.main_options.debug)

    def _show_one_builtin(self, builtin, string, variables):
        """Go over the available builtins handling tools."""

        import textwrap
        if builtin.name() != 'none':
            print ('%s %s' % (string, builtin.name()))
            if variables:
                for attribute in builtin().attributes():
                    print ('    %s=%s' % (attribute.name, attribute.value))
                    lines = ['      %s' % line for line in textwrap.wrap(attribute.help)]
                    print ('\n'.join(lines))

    def _show_variables(self, module):
        """Handles the show the variables available for source and build."""
        
        source = module.get_source()
        if source.attributes():
            print ('  source %s' % source.name())
            for attribute in source.attributes():
                print ('    %s=%s' % (attribute.name, attribute.value))
        build = module.get_build()
        
        if build.attributes():
            print ('  build %s' % build.name())
            for attribute in build.attributes():
                print ('    %s=%s' % (attribute.name, attribute.value))

    def _show_builtin(self, config, args):
        """Handles the show one builtin command line option."""
        
        from bake.ModuleSource import ModuleSource
        from bake.ModuleBuild import ModuleBuild
        parser = OptionParser(usage='usage: %prog show [options]')
        parser.add_option('-a', '--all', action='store_true', dest='all', 
                          default=False,
                          help='Display all known information about builtin source and build commands')
        parser.add_option('--source', action='store_true', dest='source', 
                          default=False,
                          help='Display information about builtin source commands')
        parser.add_option('--build', action='store_true', dest='build', 
                          default=False,
                          help='Display information about builtin build commands')
        parser.add_option('--variables', action='store_true', dest='variables', 
                          default=False,
                          help='Display variables for builtin commands')
        (options, args_left) = parser.parse_args(args)
            
        if options.all :
            options.source = True
            options.build = True
            options.variables = True
        elif not options.source and not options.build :
            options.source = True
            options.build = True
          
            
        if options.source:
            for source in ModuleSource.subclasses():
                self._show_one_builtin(source, 'source', options.variables)
                
        if options.build:
            for build in ModuleBuild.subclasses():
                self._show_one_builtin(build, 'build', options.variables)

    systemDependencies=dict()
   
    def show_module(self, state, options, config, label):
        """ Handles the printing of the information of modules and dependencies."""
        
        depen=dict()

        if not state:
            return
        for mod in state:
            if mod.mtype():
                print('module %s: %s (%s)' % (mod.mtype(), mod.name(), label))
            else:
                print('module: %s (%s)' % (mod.name(), label))
            dependencies = mod.dependencies()
            
            # Stores the system dependencies
            if isinstance(mod._source, SystemDependency) and label=="enabled":
                self.systemDependencies[mod.name()] = mod._source
                
            # Collects the dependencies
            if not mod.name() in depen:
                depen[mod.name()] = dict()
            
            if dependencies and not options.brief == True:
                print('  depends on:')
                for dependsOn in mod.dependencies():
                    print('     %s (optional:%s)' % 
                          (dependsOn._name, dependsOn.is_optional())) 
                    depen[mod.name()][dependsOn._name]=  dependsOn.is_optional()
            elif not options.brief == True:
                print('  No dependencies!')
                
            
            if options.variables:
                self._show_variables(mod)

        if options.enabledTree and label=="enabled":
            print("\n-- Enabled modules dependency tree --")
            self.deptree(depen, depen, label, dict(), "", " ")
       
        return mod

    def showSystemDependencies(self, systemDependencies,config):
        """ Shows the System dependencies of the defined configuration. """
        
        if len(systemDependencies)<=0:
            return
        
        print ("\n-- System Dependencies --")
        
        (distribution, version, version_id) = distro.linux_distribution()

        if not distribution:
            distribution = 'darwin' # osName
        else:
            distribution = distribution.lower()

        missing=False
        returnValue=""
        depend_keys = systemDependencies.keys()
        depend_keys=sorted(depend_keys)

        # creates the environment        
        configuration = self._read_config(config)
        logger = StdoutModuleLogger()
        logger.set_verbose(0)
        env = ModuleEnvironment(logger, "","")
        
        for this_key in depend_keys:
            sysDep=systemDependencies[this_key]
            dependencyExists = False
        
            # We support one of the following three attributes:
            # file_test, executable_test, and dependency_test (deprecated)
            if (sysDep.attribute('file_test').value is not None):
                dependencyExists = sysDep._check_file_expression ( 
                  sysDep.attribute('file_test').value)
            elif (sysDep.attribute('executable_test').value is not None):
                dependencyExists = sysDep._check_executable_expression ( 
                  sysDep.attribute('executable_test').value)
            # XXX Deprecated attribute; will be removed in future
            elif (sysDep.attribute('dependency_test').value is not None):
                dependencyExists = sysDep._check_dependency_expression(env, 
                  sysDep.attribute('dependency_test').value)
            elif (sysDep.attribute('import_test').value is not None):
                dependencyExists = sysDep._check_import(
                  sysDep.attribute('import_test').value)

            if not dependencyExists:
                sys.stdout.write(" > " + this_key + " - ")
                ColorTool.cPrintln(ColorTool.FAIL, "Missing")
                print("   >> " + sysDep.attribute('more_information').value)
                command = sysDep._get_command(distribution)
                command = command.strip()
                if not command == '':
                    installerName = sysDep.attribute('name_' + command.split()[0]).value
            
                    # if didn't find the specific installer name uses the default one
                    if(not installerName):
                        installerName = this_key
                      
                    if (sysDep.attribute('import_test').value is None):
                        print('   >> Try: "sudo ' + command + ' ' + 
                              installerName + '", if you have sudo rights.')

                missing = True
            else:
                sys.stdout.write(" > " + this_key + " - ")
                ColorTool.cPrintln(ColorTool.OK, "OK")
                
            returnValue= returnValue + this_key

        # if there is a missing dependency the system error level is set to 1 
        if missing:
            sys.exit(1)
        
        return returnValue
        
    def deptree(self, fulldep, depen, key, has_passed, optionalModule, padding):
        """ Shows the dependency tree. """
        
        sys.stdout.write(padding[:-1] + '+-' + key + '/')
        color=ColorTool.FAIL
        if 'optional' in optionalModule:
            color=ColorTool.OK
        ColorTool.cPrintln(color, optionalModule)
        padding = padding + ' '
        
        # to avoid loops
        if key in has_passed:
            sys.stdout.write(padding)
            ColorTool.cPrintln(ColorTool.FAIL, "> Cyclic Dependency")
            return "> Cyclic Dependency."
        else:
            has_passed[key]=True
        
        depend_keys = depen.keys()
        depend_keys=sorted(depend_keys)
        
        # goes recursively over the list of keys reading the dictionaries with 
        # the dependencies
        count = 0
        listStr = ''
        for this_key in depend_keys:
            count += 1
            print (padding + '|')
            optional=""
            color=ColorTool.FAIL
            if this_key in depen and isinstance(depen[this_key],bool)>0:
                if depen[this_key]:
                    optional = " (optional)"
                    color=ColorTool.OK
                else:
                    optional = " (mandatory)"

            if this_key in fulldep and len(fulldep[this_key])>0:
                        
                if count == len(depend_keys):
                    listStr = listStr + self.deptree(fulldep, fulldep[this_key], this_key, has_passed, optional, padding + ' ')
                else:
                    listStr = listStr + self.deptree(fulldep, fulldep[this_key], this_key, has_passed, optional, padding + '|')
            else:
                if this_key in fulldep:
                    sys.stdout.write(padding + '+-' + this_key)
                    ColorTool.cPrintln(color, optional)
                listStr = this_key +'.'+ listStr
               
        del has_passed[key]
        return key +'/'+ listStr

    def _print_version(self):
        print(" > Bake Version 0.1")
        
    def _show(self, config, args):
        """Handles the show command line option."""
        
        parser = OptionParser(usage='usage: %prog show [options]')
        parser.add_option('-a', '--all', action='store_true', dest='all', 
                          default=False,
                          help='Display all known information about current configuration')
        parser.add_option('--enabled', action='store_true', dest='enabled',
                          default=False, help='Display information about existing enabled modules')
        parser.add_option('--disabled', action='store_true', dest='disabled',
                          default=False, help='Display information about existing disabled modules')
        parser.add_option('--available', action='store_true', dest='available',
                          default=False, help='Display information about available modules')
        parser.add_option('--variables', action='store_true', dest='variables', 
                          default=False,
                          help='Display information on the variables set for the modules selected')
        parser.add_option('--predefined', action='store_true', dest='predefined', 
                          default=False,
                          help='Display information on the items predefined')
        parser.add_option('--directories', action='store_true', dest='directories', 
                          default=False,
                          help='Display information about which directories have been configured')
        parser.add_option('--enabledTree', action='store_true', dest='enabledTree', 
                          default=False,
                          help='Shows the enabled modules dependency tree')
        parser.add_option('--showSystemDep', action='store_true', dest='showSystemDep', 
                          default=True,
                          help='Shows the system dependency of the enabled/disabled modules')
        parser.add_option('-b', '--brief', action='store_true', dest='brief', 
                          default=False,
                          help='Show only the module name')
        parser.add_option('-c', '--configured', action='store_true', dest='configured', 
                          default=False,
                          help='Show only the configured module')
        (options, args_left) = parser.parse_args(args)
        # adds a default value so that show will show something even if there is
        # no option 
        if not args:
            options.enabled = True
            options.showSystemDep = True
        else:
            if not options.disabled and not options.enabled and not options.configured:
                options.enabled=True

        config= self.check_configuration_file(config, True);

        import os
        if os.path.isfile(config):
            configuration = self._read_config(config)
        else:
            # try to get the default 
            print(" > Couldn't find the " + config + " configuration file. \n"
                  "   Call bake with -f [full path configuration file name].\n")
            return
        if options.all:
            options.enabled = True
            options.disabled = True
            options.directories = True
            options.variables = True
            options.predefined = True
            options.enabledTree = True
        elif options.available:
            options.enabled = True
            options.disabled = True
           
        if options.directories:
            print ('installdir   : ' + configuration.compute_installdir())
            print ('sourcedir    : ' + configuration.compute_sourcedir())


        enabled = []
        def _iterator(module):
            enabled.append(module)
            return True
        self._iterate(configuration, _iterator, configuration.enabled())
        disabled = filter(lambda module: not module in enabled, configuration.modules())

        if options.enabled:
            self.show_module(enabled, options, config, 'enabled')

        if options.configured:
            self.show_module(configuration.configured(), options, config, 'configured')

        if options.disabled:
            self.show_module(disabled, options, config, 'disabled')
            
        if options.showSystemDep:
            self.showSystemDependencies(self.systemDependencies, config)


    def check_configuration_file(self, configFile, considersTemplate=False):
        """ Checks if the configuration file exists, if not tries to use the
        one on the root bake directory."""
        
        # If the name is not the default one... do not interfere 
        if configFile != "bakeconf.xml" and configFile != "bakefile.xml":
            return configFile
        
        # If the file is the default, and exists on the local directory, fine
        if os.path.isfile(configFile):
            return configFile

        presentDir = os.path.dirname(sys.argv[0])
        if not presentDir:
            presentDir = "."
        # if the file does not exist on the local directory
        # tries the standard configuration file on the installation directory
        if os.path.isfile(os.path.join(presentDir, configFile)):
            return os.path.join(presentDir, configFile)
        
        # if the standard file does not exist 
        # tries the generic configuration file on the installation directory
        if  considersTemplate and os.path.isfile(os.path.join(presentDir,
                                                              "bakeconf.xml")):
            return os.path.join(presentDir,"bakeconf.xml")
        
        # if everything else fail.... returns the same name
        return configFile

    options = ""

    def checkPythonVersion(self):
        """ Checks the version  of the user's machine python. python-2.6 or
            2.7 supported"""

        if sys.hexversion < 0x02060000:
            print(">>> Old Python version detected, please use python2 >= version 2.6")
            sys.exit(1)

    def main(self, argv):
        """Main Bake function."""
       
        # catches Ctrl-c 
        signal.signal(signal.SIGINT, signal_handler)
        
        self.checkPythonVersion()
        
        parser = MyOptionParser(usage='usage: %prog [options] command [command options]',
                                description="""Where command is one of:
  deploy       : Downloads the configured modules AND makes the build in one step
  configure    : Setup the build configuration (source, build, install directory,
                 and per-module build options) from the module descriptions
  fix-config   : Update the build configuration from a newer module description
  search       : Search for modules from the AppStore
  getconf      : Downloads the bakefile for the module
  install      : Download and build the module
  download     : Download all modules enabled during configure
  update       : Update the source tree of all modules enabled during configure
  build        : Build all modules enabled during configure
  clean        : Cleanup the source tree of all modules built previously
  shell        : Start a shell and setup relevant environment variables
  uninstall    : Remove all files that were installed during build
  distclean    : Call the modules distclean option, if available
  fullclean    : Remove all the build AND source files
  show         : Report on build configuration
  show-builtin : Report on builtin source and build commands
  check        : Checks if all the required tools are available on the system

To get more help about each command, try:
  %s command --help
""")
        parser.add_option("-f", "--file", action="store", type="string",
                          dest="config_file", default="bakefile.xml",
                          help="The Bake file to use, and the target "
                          "configuration/reconfiguration. Default: %default.")
        parser.add_option("--debug", action="store_true",
                          dest="debug", default=False,
                          help="Prints out all the error messages and problems.")
        parser.add_option("--noColor", action="store_true",
                          dest="noColor", default=False, 
                          help='Print messages with no color')
        parser.add_option("-V", action="store_true",
                          dest="version", default=False, 
                          help='Prints the version of Bake' )
        parser.disable_interspersed_args()
        (options, args_left) = parser.parse_args(argv[1:])
        
        if options.version:
            self._print_version()
        

        Bake.main_options = options
        
        # if asked to not having collors, useful for non iteractive
        # use and users that do not have a color enabled terminal 
        if options.noColor:
            ColorTool.disable()
        else:
            has_colours = ColorTool.has_colours(sys.stdout)
            if not has_colours:
                ColorTool.disable()


        if len(args_left) == 0:
            if not options.version:
                parser.print_help()
            sys.exit(1)
        ops = [ ['deploy', self._deploy],
                ['configure', self._configure],
                ['fix-config', self._fix_config],
                ['search', self._search],
                ['getconf', self._getconf],
                ['install', self._install],
                ['download', self._download],
                ['update', self._update],
                ['build', self._build],
                ['clean', self._clean],
                ['shell', self._shell],
                ['uninstall', self._uninstall],
                ['distclean', self._distclean],
                ['fullclean', self._fullclean],
                ['show', self._show],
                ['show-builtin', self._show_builtin],
                ['check', self._check],
                ['list', self._list],
               ]
        recognizedCommand = False
        
        for name, function in ops: 
            if args_left[0].lower() == name:
                recognizedCommand = True
                if options.debug:
                    function(config=options.config_file, args=args_left[1:])
                else:
                    try:
                        function(config=options.config_file, args=args_left[1:])
                    except Exception as e:
                        print ('\n'+ str(e))
                        sys.exit(1)
                    except TaskError as e:
                        print ('\n'+e.reason)
                        sys.exit(1)
                        
        if not recognizedCommand:
            print (' >> Unrecognized option: ' + args_left[0])
            sys.exit(1)
           
