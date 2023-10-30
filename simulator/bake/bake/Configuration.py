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
 Configuration.py

 The main purpose of this file is to store all the classes related
 to the configuration of Bake. 
''' 

import os
import re
import sys
import xml.etree.ElementTree as ET
try:
 from xml.etree.ElementTree import ParseError
except ImportError:
 from xml.parsers.expat import ExpatError as ParseError
from bake.Module import Module, ModuleDependency
from bake.ModuleSource import ModuleSource, InlineModuleSource
from bake.ModuleBuild import ModuleBuild, InlineModuleBuild
from bake.Exceptions import MetadataError
from bake.Exceptions import TaskError 

class MetadataFile:
    """Stores the meta information of a given file."""
    
    def __init__(self, filename, h=''):
        self._filename = os.path.realpath(filename)
        self._h = h

    def filename(self):
        return self._filename

    def h(self):
        import hashlib
        m = hashlib.sha256()
        try:
            f = open(self._filename)
            m.update(f.read())
            f.close()
            return m.hexdigest()
        except:
            return ''

    def is_hash_ok(self):
        """Verifies if the hash of the configuration file is OK, to avoid 
        manual and transmission changes.
        """
        
        return self.h() == self._h

class PredefinedConfiguration:
    """Stores the information of predefined options."""
    
    def __init__(self, name, enable, disable, variables_set, variables_append,
                 directories):
        self.name = name
        self.enable = enable
        self.disable = disable
        self.variables_set = variables_set
        self.variables_append = variables_append
        self.directories = directories       

class Configuration:
    """Main configuration class."""
    
    def __init__(self, bakefile, relative_directory_root=None):
        self._enabled = []
        self._disabled = []
        self._modules = []
        self._configured = []
        self._installdir = None
        self._sourcedir = None
        self._metadata_file = None
#        self._bakefile = os.path.abspath(bakefile)
        if bakefile.startswith(os.sep):
            self._bakefile = os.path.abspath(bakefile)
        else:   
            self._bakefile = os.getcwd()+os.sep+bakefile
        if relative_directory_root is None:
            self._relative_directory_root = os.path.relpath(os.getcwd(),
                                                            os.path.dirname(self._bakefile))
        else:
            self._relative_directory_root = relative_directory_root
        
    def read_metadata(self, filename):
        """ Reads the list of meta-data defined in the XML config file"""
   
        if not os.path.exists(filename):
            self._error('Could not find "%s"' % filename)
            
        self._metadata_file = MetadataFile(filename)
        et = ET.parse(filename)
        self._read_metadata(et)
        
    def read_predefined(self, filename):
        """ Creates the list of predefined entries defined in the XML 
        configuration file
        """
        predefined = []

        try:
            et = ET.parse(filename)
        except ParseError:
            return predefined
        
        for pred_node in et.getroot().findall('predefined'):
            name = pred_node.get('name', None)
            
            if not name:
                self._error('<predefined> must define a "name" attribute.')
                
            enable = []
            for enable_node in pred_node.findall('enable'):
                enable_name = enable_node.get('name', None)
                
                if not enable_name:
                    self._error('<enable> must define a "name" attribute.')
                enable.append(enable_name)
                
            disable = []
            for disable_node in pred_node.findall('disable'):
                disable_name = disable_node.get('name', None)
                
                if not disable_name:
                    self._error('<disable> must define a "name" attribute.')
                disable.append(disable_name)
                
            variables_set = []
            for set_node in pred_node.findall('set'):
                set_name = set_node.get('name', None)
                set_value = set_node.get('value', None)
                set_module = set_node.get('module', None)
                
                if not set_name or not set_value:
                    self._error('<set> must define a "name" and a "value" attribute.')
                variables_set.append((set_module, set_name, set_value))
                
            variables_append = []
            for append_node in pred_node.findall('append'):
                append_name = append_node.get('name', None)
                append_value = append_node.get('value', None)
                append_module = append_node.get('module', None)
                
                if not append_name or not append_value:
                    self._error('<append> must define a "name" and a "value" attribute.')
                variables_append.append((append_module, append_name, append_value))
                
            directories = {}
            for config_node in pred_node.findall('configuration'):
                installdir = config_node.get('installdir', None)
                sourcedir = config_node.get('sourcedir', None)
                
                if installdir:
                    directories['installdir'] = installdir
                    
                if sourcedir:
                    directories['sourcedir'] = sourcedir
                    
            predefined.append(PredefinedConfiguration(name, enable, disable,
                                                      variables_set,
                                                      variables_append,
                                                      directories))
        return predefined

    def _error(self, string):
        """ Handles the exceptions """
        raise Exception(string)

    def _check_mandatory_attributes(self, attribute_base, node, type_string, 
                                    module_string):
        """ Checks the existence of the mandatory attributes for each 
        configuration.
        """
        
        # get list of names in <attribute name="" value=""> tags
        attributes_present = [child.get('name') for child in node.findall('attribute')]
        # get list of names in <type_string name="value"> attributes
        attributes_present = attributes_present + list(node.attrib)
        
        for attribute in attribute_base.attributes():
            if attribute.is_mandatory and not attribute.name in attributes_present:
                sys.stderr.write('Error: mandatory attribute "%s" is missing from '
                                 'module "%s" in node "%s"\n' % (attribute.name,
                                                                 module_string,
                                                                 type_string))
                sys.exit(1)

    def _read_attributes(self, obj, node, type_string, module_string):
        """ Reads the list of attributes on the configuration configuration."""
        
        # read <type_string><attribute name="" value=""></type_string> tags
        for attribute_node in node.findall('attribute'):
            attr_name = attribute_node.get('name')
            attr_value = attribute_node.get('value', None)
            if obj.attribute(attr_name) is None:
                sys.stderr.write('Error: attribute "%s" is not supported by' 
                                 ' %s node of type "%s"\n' % 
                                 (attr_name, type_string, node.get('type')))
                sys.exit(1)
            obj.attribute(attr_name).value = attr_value
            
        # as a fallback, read <type_string name="value"> attributes
        # note: this will not generate errors upon invalid attribute names
        # because certain kinds of <foo name="value"/> XML attributes are
        # not handled as bake attributes.
        for attr_name in node.attrib.keys():
            if not obj.attribute(attr_name) is None:
                obj.attribute(attr_name).value = node.get(attr_name)

    def _write_attributes(self, attribute_base, obj_node):
        """ Creates the XML elements, reflecting the listed attributes."""
        
        # generate <attribute name="" value=""/> tags
        for attribute in attribute_base.attributes():
            if not attribute.value is None:
                attribute_node = ET.Element('attribute', {'name' : attribute.name,
                                                          'value' : attribute.value})
                obj_node.append(attribute_node)

    def _create_obj_from_node(self, node, classBase, node_string, module_name):
        """ Translates the XML elements on the correct bake object."""
        
        # read <node_string type=""> tag: handle type="inline" specially by
        # looking up a child node <code></code>
        if node.get('type') == 'inline':
            code_node = node.find('code')
            if code_node is None:
                sys.stderr.write('Error: no code tag in inline module\n')
                sys.exit(1)
                
            classname = node.get('classname')
            import codeop
            exec(code_node.text, globals(), locals())
            obj = eval(classname + '()')
            obj.__hidden_source_code = code_node.text
        else:
            obj = classBase.create(node.get('type'))

        self._check_mandatory_attributes(obj, node, node_string, module_name)
        self._read_attributes(obj, node, node_string, module_name)
        
        # if <type_string> has <child> nodes, look them up.
        for child_node in node.findall('child'):
            child_name = child_node.get('name')
            child = self._create_obj_from_node(child_node, classBase, 'child', 
                                               module_name)
            obj.add_child(child, child_name)

        return obj

    def _create_node_from_obj(self, obj, node_string):
        """ Generates the XML node based on the XML object passed as parameter"""

        # inline is when one uses Python as build configuration to create a 
        # small build script    
        if obj.__class__.name() == 'inline':
            node = ET.Element(node_string, {'type' : 'inline',
                                            'classname' : obj.__class__.__name__})
            code = ET.Element('code')
            code.text = obj.__hidden_source_code
            node.append(code)
        else:
            node = ET.Element(node_string, {'type' : obj.__class__.name()})

        self._write_attributes(obj, node)

        for child, child_name in obj.children():
            child_node = self._create_node_from_obj(child, 'child')
            child_node.attrib['name'] = child_name
            node.append(child_node)

        return node

    def _read_installed(self, node):
        """ Reads the installed modules from the XML."""
        
        installed = []
        for installed_node in node.findall('installed'):
            installed.append(installed_node.get('value', None))
        return installed

    def _write_installed(self, node, installed):
        """ Generates the XML nodes to register the installed modules."""
        
        for installed in installed:
            installed_node = ET.Element('installed', {'value' : installed})
            node.append(installed_node)

    
    def _read_metadata(self, et):
        """ Reads the elements from the xml configuration files and add it to 
        the internal list of modules.
        """

        # function designed to be called on two kinds of xml files.
        modules = et.findall('modules/module')
        for module_node in modules:
            name = module_node.get('name')
            mtype = module_node.get('type')
            min_ver = module_node.get('min_version')
            max_ver = module_node.get('max_version')
            installed = self._read_installed(module_node)

            source_node = module_node.find('source')
            source = self._create_obj_from_node(source_node, ModuleSource, 
                                                'source', name)

            build_node = module_node.find('build')
            build = self._create_obj_from_node(build_node, ModuleBuild, 
                                               'build', name)
#            self._read_libpath(build_node, build)

            dependencies = []
            for dep_node in module_node.findall('depends_on'):                                                        
                dependencies.append(self._create_obj_from_node(dep_node,ModuleDependency,'depends_on',name))
                            
            module = Module(name, source, build, mtype, min_ver, max_ver, dependencies=dependencies,
                            built_once=bool(module_node.get('built_once', '').upper()=='TRUE'),
                            installed=installed)
            self._modules.append(module)

    def _write_metadata(self, root):
        """ Saves modules data to the XML configuration file."""
        
        modules_node = ET.Element('modules')
        root.append(modules_node)
        
        for module in self._modules:
            module_attrs = {'name' : module.name()}
            if module.mtype():
                module_attrs['type'] = module.mtype()
            if module.minver():
                module_attrs['min_version'] = module.minver()
            if module.is_built_once():
                module_attrs['built_once'] = 'True'
            module_node = ET.Element('module', module_attrs)
            self._write_installed(module_node, module.installed)

            # registers the values, possible changed ones, from the source and
            # build XML tags of each module
            source_node = self._create_node_from_obj(module.get_source(), 
                                                     'source')
            module_node.append(source_node)

            build_node = self._create_node_from_obj(module.get_build(), 'build')
            module_node.append(build_node)
#            self._write_libpath(build_node, module.get_build())
            
            # handles the dependencies for the module and register them 
            # into module node
            for dependency in module.dependencies():                           
                dep_node = self._create_node_from_obj(dependency, 'depends_on')                                
                module_node.append(dep_node)
            modules_node.append(module_node)

    def defineXml(self):
        """ Creates the basic XML structure for the configuration file."""
        
        root = ET.Element('configuration', {'installdir':self._installdir,
                'sourcedir':self._sourcedir,
                'relative_directory_root':self._relative_directory_root,
                'bakefile':self._bakefile})
        
        if not self._metadata_file is None:
            metadata = ET.Element('metadata', 
                                  {'filename':self._metadata_file.filename(),
                                   'hash':self._metadata_file.h()})
            root.append(metadata)
            
        # write enabled nodes
        for e in self._enabled:
            enable_node = ET.Element('enabled', {'name':e.name()})
            root.append(enable_node)
        
        # write disabled nodes
        for e in self._disabled:
            disable_node = ET.Element('disabled', {'name':e.name()})
            root.append(disable_node)
        
        # add modules information
        self._write_metadata(root)
        et = ET.ElementTree(element=root)
        return et

    def write(self):
        """ Creates the target configuration XML file."""
        
        et = self.defineXml()
        
        try:
            et.write(self._bakefile)
        except IOError as e:
            raise TaskError('Problems writing the file, error: %s' % e)

    def read(self):
        """ Reads the XML customized configuration file."""
         
        try:
            et = ET.parse(self._bakefile)
        except IOError as e:
            err = re.sub(r'\[\w+ \w+\]+', ' ', str(e)).strip()
            raise TaskError('>> Problems reading the configuration file, verify if'
                            ' it exists or try calling bake.py configure. \n'
                            '   Error: %s' % err)

        self._read_metadata(et)
        root = et.getroot()
        self._installdir = root.get('installdir')
        self._sourcedir = root.get('sourcedir')
        self._relative_directory_root = root.get('relative_directory_root')
        original_bakefile = root.get('bakefile')
        metadata = root.find('metadata')
        
        if metadata is not None: 
            self._metadata_file = MetadataFile (metadata.get('filename'),
                                            h=metadata.get('hash'))

        # read which modules are enabled
        modules = root.findall('enabled')
        for module in modules:
            self._configured.append(self.lookup(module.get('name')))
            enabled = self.lookup(module.get('name'))
            self.enable(enabled)

        # read which modules are disabled
        modules = root.findall('disabled')
        for module in modules:
            disabled = self.lookup(module.get('name'))
            self.disable(disabled)

        if metadata  is not None: 
            return self._metadata_file.is_hash_ok() #and original_bakefile == self._bakefile
        else :
            return True

    def set_installdir(self, installdir):
        self._installdir = installdir

    def get_installdir(self):
        return self._installdir

    def set_sourcedir(self, sourcedir):
        self._sourcedir = sourcedir

    def get_sourcedir(self):
        return self._sourcedir

    def get_relative_directory_root(self):
        return self._relative_directory_root

    def _compute_path(self, p):
        """Returns the full path"""
        
        if os.path.isabs(p):
            return p
        else:
            tmp = os.path.join(os.path.dirname(self._bakefile), 
                               self._relative_directory_root, p)
            return os.path.normpath(tmp)

    def compute_sourcedir(self):
        return self._compute_path(self._sourcedir)

    def compute_installdir(self):
        return self._compute_path(self._installdir)

    def enable(self, module):
        """ Set the module as enabled, but if it is disabled, simply removes 
        it from the disabled list.
        """
        
        if module in self._disabled:
            self._disabled.remove(module)
        elif module not in self._enabled:
            self._enabled.append(module)
 
    def disable(self, module):
        """ Set the module as disabled, but if it is enabled, simply removes 
        it from the enabled list.
        """
        
        if module in self._enabled:
            self._enabled.remove(module)
        else:
            self._disabled.append(module)

    def lookup(self, name):
        """ Finds the module in the modules list."""
        
        for module in self._modules:
            if module.name() == name:
                return module
        return None

    def enabled(self):
        return self._enabled

    def disabled(self):
        return self._disabled

    def modules(self):
        return self._modules

    def configured(self):
        return self._configured
