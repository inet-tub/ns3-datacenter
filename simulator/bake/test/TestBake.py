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
import unittest
# hack to save ourselves from having to use PYTHONPATH
import sys
import os
import subprocess 
import re

from bake.Configuration import Configuration
from bake.ModuleSource import SystemDependency
from bake.ModuleEnvironment import ModuleEnvironment
from bake.ModuleLogger import StdoutModuleLogger
from bake.ModuleSource import ModuleSource
from bake.Exceptions import TaskError
from bake.Bake import Bake


sys.path.append(os.path.join (os.getcwd(), '..'))

def compensate_third_runner():
    """ Compensates the name of the file, if a third party program is
        inside eclipse."""
    fileName = sys.argv[0]
    if len(sys.argv) > 1:
        fileName = sys.argv[1]
    return fileName

class TestBake(unittest.TestCase):
    """Tests cases for the main Bake Class."""

           
    def setUp(self):
        """Common set Up environment, available for all tests."""
        pathname = os.path.dirname("/tmp/source/")  
        self._logger = StdoutModuleLogger();
        self._logger.set_verbose(1)
        self._env = ModuleEnvironment(self._logger, pathname, pathname, pathname)
#        testStatus = subprocess.getoutput('cp ' + pathname + '/bakefile.xml /tmp/.')
        testStatus = subprocess.getoutput('chmod 755 /tmp/source')
        testStatus = subprocess.getoutput('rm -rf /tmp/source')

        
    def tearDown(self):
        """Cleans the environment environment for the next tests."""
        self._env = None
        pathname = os.path.dirname("/tmp/source")  
#        pathname = os.path.dirname(compensate_third_runner())  
        testStatus = subprocess.getoutput('rm -f ' + pathname +'/bakefile.xml')
        testStatus = subprocess.getoutput('chmod 755 /tmp/source')
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        testStatus = subprocess.getoutput('mv bc.xml bakeconf.xml ')
        testStatus = subprocess.getoutput('mv bf.xml bakefile.xml ')
        testStatus = subprocess.getoutput('mv ~/.bakerc_saved ~/.bakerc')

    def test_simple_proceedings(self):
        """Tests a simple download and build of Bake. """

        mercurial = ModuleSource.create("mercurial")
        testResult = mercurial.check_version(self._env)
        self.assertTrue(testResult)
        mercurial.attribute("url").value = "http://code.nsnam.org/daniel/bake"
        self._env._module_name="bake"
        self._env._module_dir="bake"
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        self._logger.set_current_module(self._env._module_name)
        testResult = mercurial.download(self._env)
        self.assertFalse(testResult)

        testStatus = subprocess.getoutput('cd /tmp/source/bake;./bake.py configure -p ns3-min')
        self.assertEquals(testStatus, "", "Should have worked the download of the code")
        testStatus = subprocess.getoutput('cd /tmp/source/bake;./bake.py download')
        self.assertFalse("Problem" in testStatus, 
                        "Should have worked the download of the code")
        testStatus = subprocess.getoutput('cd /tmp/source/bake;./bake.py build')
        self.assertFalse("Problem" in testStatus, 
                        "Should have worked the build of the code")
  
 
    def test_read_resource_file(self):
        """Tests the _read_resource_file method of Class Bake."""
        
        configuration = Configuration("bakefile.xml")
        testStatus,testMessage = subprocess.getstatusoutput('mv ~/.bakerc ~/.bakerc_saved')
        self.assertTrue(testStatus==0 or testStatus==256,"Couldn't move the ressource file!")
        bake = Bake()
        
        testResult = bake._read_resource_file(configuration)
        self.assertFalse(testResult,"Shouldn't find a configuration!")
        
        testStatus = subprocess.getoutput('touch ~/.bakerc')
        testResult = bake._read_resource_file(configuration)
        self.assertFalse(testResult,"Configuration should be empty!")

        testStatus = subprocess.getoutput('cp ./bakeTest.rc ~/.bakerc')
        testResult = bake._read_resource_file(configuration)
        self.assertTrue(testResult[0].name=="my-ns3","Shouldn't find a configuration!")
        
        testStatus,testMessage = subprocess.getstatusoutput('mv ~/.bakerc_saved ~/.bakerc')
      
    def test_save_resource_configuration(self):
        """Tests the _save_resource_configuration method of Class Bake."""
        
        pathname = os.path.dirname(compensate_third_runner())  
        if not pathname:
            pathname="."
        testStatus = subprocess.getoutput('python ' + pathname + 
                                        '/../bake.py -f ./ttt.xml configure ' 
                                        '--enable=openflow-ns3 ' 
                                        '--sourcedir=/tmp/source ' 
                                        '--installdir=/tmp/source')

        configuration = Configuration("./ttt.xml")
        configuration.read()
        testStatus,testMessage = subprocess.getstatusoutput('mv ~/.bakerc ~/.bakerc_saved')
        self.assertTrue(testStatus==0 or testStatus==256,
                        "Couldn't move the resource file!")
        fileName = os.path.join(os.path.expanduser("~"), ".bakerc")
        testStatus,testMessage = subprocess.getstatusoutput('rm -rf ./ttt.xml')
        bake = Bake()
        
        testResult = bake._save_resource_configuration(configuration)
        self.assertFalse(testResult,"Should have write configuration!")
        self.assertTrue(os.path.isfile(fileName), "Didn't create the .bakerc file")
        fin = open(fileName, "r")
        string = fin.read()
        fin.close()
        self.assertTrue("last" in string, "Didn't save the last configuration")
        self.assertTrue("openflow-ns3" in string, 
                        "Didn't save the last configuration")
        
        testStatus = subprocess.getoutput('cp ./bakeTest.rc ~/.bakerc')
        fin = open(fileName, "r")
        string = fin.read()
        fin.close()
        self.assertFalse("last" in string, "Did find last in the configuration")
        
        testResult = bake._save_resource_configuration(configuration)
        self.assertFalse(testResult,"Should have write configuration!")
        self.assertTrue(os.path.isfile(fileName), "Didn't create the .bakerc file")
        fin = open(fileName, "r")
        string = fin.read()
        fin.close()
        self.assertTrue("<predefined name=\"last\">" in string, "Didn't save the last configuration")
        self.assertTrue("<enable name=\"openflow-ns3\"/>" in string, 
                        "Didn't save the last configuration")


        testStatus,testMessage = subprocess.getstatusoutput('mv ~/.bakerc_saved ~/.bakerc')
  
   
    def test_check_source_code(self):
        """Tests the _check_source_code method of Class Bake. """

        # Environment settings        
        mercurial = ModuleSource.create("mercurial")
        testResult = mercurial.check_version(self._env)
        self.assertTrue(testResult)
        
        pathname = os.path.dirname(compensate_third_runner())  
        if not pathname:
            pathname="."
        testStatus = subprocess.getoutput('python ' + pathname + 
                                        '/../bake.py configure ' 
                                        '--enable=openflow-ns3 ' 
                                        '--sourcedir=/tmp/source ' 
                                        '--installdir=/tmp/source')

        mercurial.attribute("url").value = "http://code.nsnam.org/bhurd/openflow"
        self._env._module_name="openflow-ns3"
        self._env._module_dir="openflow-ns3"
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        self._logger.set_current_module(self._env._module_name)
        testResult = mercurial.download(self._env)

        bake = Bake()
        config = "bakefile.xml" #bakefile.xml"
        args = []
        parser = bake._option_parser('build')
        parser.add_option('-j', '--jobs', 
                          help='Allow N jobs at once. Default is 1.',
                          type='int', action='store', dest='jobs', default=1)
        parser.add_option("--debug", action="store_true", 
                          dest="debug", default=True, 
                          help="Should we enable extra Bake debugging output ?")
        (options, args_left) = parser.parse_args(args)
#        bake.setMainOptions(options)
        Bake.main_options = options
        
        # Tests typical case, the module is there
        testResult = bake._check_source_code(config, options);
        self.assertEqual(testResult, None)
 
        # if the user has no permission to see the file
        testStatus = subprocess.getoutput('chmod 000 /tmp/source')
        testResult=None
        try:
            testResult = bake._check_source_code(config, options);
        except SystemExit as e:
            self.assertTrue(e.code==1,"Wrong error code received")
            
        self.assertFalse(testResult, None)    
        
        testStatus = subprocess.getoutput('chmod 755 /tmp/source')
           
        # if the folder is not where it should be
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        testResult=None
        try:
            testResult = bake._check_source_code(config, options);
        except SystemExit as e:
            self.assertTrue(e.code==1,"Wrong error code received")
            
        self.assertFalse(testResult, None)    
             

    def test_check_build_version(self):
        """Tests the _check_source_code method of Class Bake. """

        # Environment settings        
        # Environment settings        
        mercurial = ModuleSource.create("mercurial")
        testResult = mercurial.check_version(self._env)
        self.assertTrue(testResult)
        
        self._env._debug = True
        pathname = os.path.dirname(compensate_third_runner()) 
        if not pathname:
            pathname="."
 
        testStatus = subprocess.getoutput('python ' + pathname + 
                                        '/../bake.py configure' 
                                        ' --enable=openflow-ns3' 
                                        ' --sourcedir=/tmp/source' 
                                        ' --installdir=/tmp/source')

        mercurial.attribute("url").value = "http://code.nsnam.org/bhurd/openflow"
        self._env._module_name="openflow-ns3"
        self._env._module_dir="openflow-ns3"
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        self._logger.set_current_module(self._env._module_name)
        testResult = mercurial.download(self._env)
#        try:
#            testResult = mercurial.download(self._env)
#            self.fail("The directory does not exist, this shouldn't work")
#        except TaskError as e:
#            self.assertNotEqual(e._reason, None)    
#            self.assertEqual(testResult, None)
        

        bake = Bake()
        config = "bakefile.xml" #bakefile.xml"
        args = []
        parser = bake._option_parser('build')
        parser.add_option('-j', '--jobs', help='Allow N jobs at once. Default is 1.',
                          type='int', action='store', dest='jobs', default=1)
        parser.add_option("--debug", action="store_true", 
                          dest="debug", default=False, 
                          help="Should we enable extra Bake debugging output ?")
        (options, args_left) = parser.parse_args(args)
#        bake.setMainOptions(options)
        Bake.main_options = options
        
        # Tests typical case, the module is there and the object directory is not
        self._env._installdir = self._env.srcdir+"/install_bake"
        testResult = bake._check_build_version(config, options);
        self.assertEqual(testResult, None)
 
        # if the user has no permission to see the file
        testStatus = subprocess.getoutput('chmod 000 /tmp/source')

        testResult=None
        try:
            testResult = bake._check_source_code(config, options);
        except SystemExit as e:
            self.assertTrue(e.code==1,"Wrong error code received")
            
        self.assertFalse(testResult, None)    
        
        testStatus = subprocess.getoutput('chmod 755 /tmp/source')
           
        # if the folder is not where it should be
        testStatus = subprocess.getoutput('rm -rf /tmp/source')
        testResult=None
        testResult=None
        try:
            testResult = bake._check_source_code(config, options);
        except SystemExit as e:
            self.assertTrue(e.code==1,"Wrong error code received")
            
        self.assertFalse(testResult, None)    

    def test_check_configuration_file(self):
        """Tests the check_configuration_file method of Class Bake. """

        bakeInstance = Bake() 

        testResult = bakeInstance.check_configuration_file("strangeName")
        self.assertEqual(testResult, "strangeName", "New name is not respected")

        testResult = bakeInstance.check_configuration_file("bakeconf.xml")
        self.assertEqual(testResult, "bakeconf.xml", "Default file should"
                         " exist but it changed the name.")
        
        testStatus = subprocess.getoutput('mv bakeconf.xml bc.xml')
        testResult = bakeInstance.check_configuration_file("bakeconf.xml")
        self.assertTrue(testResult.endswith("bakeconf.xml"), "Should have"
                        " returned the bakeconf.xml but returned " + testResult)

#        testStatus = subprocess.getoutput('mv bakefile.xml bf.xml')
#        testStatus = subprocess.getoutput('mv bc.xml bakeconf.xml ')
#        testResult = bakeInstance.check_configuration_file("bakefile.xml", True)
#        self.assertTrue(testResult.endswith("bakeconf.xml"), "Should have"
#                        " returned the bakeconf but returned " + testResult)

        testStatus = subprocess.getoutput('mv bakeconf.xml bc.xml')
        testResult = bakeInstance.check_configuration_file("bakefile.xml")
        self.assertEqual(testResult, "bakefile.xml", "Default file should"
                         " be returned in the last case.")
        
        testStatus = subprocess.getoutput('mv bc.xml bakeconf.xml ')
        testStatus = subprocess.getoutput('mv bf.xml bakefile.xml ')

#     xmlVar = '<configuration> <modules> <module name="test1"> 
#    
#      <source type="mercurial">
#    <attribute name="url" value="http://code.nsnam.org/bake"/>
#      </source>
#      <build type="waf" objdir="yes">
#    <attribute name="configure_arguments" value="configure --prefix=$INSTALLDIR --enable-examples --enable-tests"/>
#      </build>
#  </module>
#
#    <module name="test2">
#      <source type="bazaar">
#    <attribute name="url" value="https://launchpad.net/pybindgen"/>
#    <attribute name="revision" value="revno:809"/>
#      </source>
#      <build type="waf" objdir="yes">
#    <attribute name="configure_arguments" value="configure --prefix=$INSTALLDIR"/>
#    <attribute name="build_arguments" value="--generate-version"/>
#      </build>
#      <depends_on name="test1" optional="False"/>
#    </module>
#
#    <module name="test3">
#      <source type="bazaar">
#    <attribute name="url" value="https://launchpad.net/pybindgen"/>
#    <attribute name="revision" value="revno:809"/>
#      </source>
#      <build type="waf" objdir="yes">
#    <attribute name="configure_arguments" value="configure --prefix=$INSTALLDIR"/>
#    <attribute name="build_arguments" value="--generate-version"/>
#      </build>
#      <depends_on name="test2" optional="True"/>
#      <depends_on name="pygccxml" optional="True"/>
#    </module>
#
#  </modules>
#
#</configuration>'

       
#    def test_dependencies(self):
#        """Tests the Dependencies mechanism of Bake. """
#        
#        configuration = Configuration("bakefile")
##        et = ET.parse(filename)
##        self._read_metadata(et)
#
#
#        testStatus = subprocess.getoutput('cp bakeconf.xml bc.xml ')
#        testStatus = subprocess.getoutput('cp bakefile.xml bf.xml ')
#
#        bake = Bake() 
#        config = "bakefile.xml" #bakefile.xml"
#        args = []
#        parser = bake._option_parser('install')
#        parser.add_option('-j', '--jobs', help='Allow N jobs at once. Default is 1.',
#                          type='int', action='store', dest='jobs', default=1)
#        parser.add_option("--debug", action="store_true", 
#                          dest="debug", default=False, 
#                          help="Should we enable extra Bake debugging output ?")
#        (options, args_left) = parser.parse_args(args)
#        Bake.main_options = options
#        
#        # Tests typical case, the module is there and the object directory is not
#        self._env._installdir = self._env.srcdir+"/install_bake"
#        testResult = bake._check_build_version(config, options);
#        self.assertEqual(testResult, None)
#
#
#        
#        testStatus = subprocess.getoutput('mv bc.xml bakeconf.xml ')
#        testStatus = subprocess.getoutput('mv bf.xml bakefile.xml ')
#       

    def test_systemReturnValues(self):
        """Tests the values get from the system point of view. """

        self._env._debug = True
        pathname = os.path.dirname(compensate_third_runner()) 
        if not pathname:
            pathname="."
 
        commandTmp = ('%s/../bake.py -f /tmp/myconf.xml configure -c %s/../bakeconf.xml --enable=openflow-ns3' 
        ' --sourcedir=/tmp/source' 
        ' --installdir=/tmp/source' %(pathname, pathname))
        (status, output) = subprocess.getstatusoutput(commandTmp)    

        (status, output) = subprocess.getstatusoutput('python ' + pathname + 
                                        '/../bake.py --debug download -vvv')
        self.assertFalse(status==0, 'Wrong system status return.')
        
#        (status, output) = subprocess.getstatusoutput('python ' + pathname + 
#                                        '/../bake.py --debug download -vvv --sudo')
#        self.assertTrue(status==0, 'Wrong system status return.')
    

        (status, output) = subprocess.getstatusoutput('python ' + pathname + 
                                        '/../bake.py build')
        self.assertTrue(status!=0, 'Wrong system status return.')    
        

    def test_printTree(self):
        """Tests tree visualization mechanism. """
        
        first = dict()
        second = dict()
        third = dict()
        bake = Bake()
        
        mainDep = {'first': first, 'second':second, 'third': third}
        
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys=='main/third.second.first.', 'Error during tree processing')

        first={'second':True}
        second={'third':True}
        mainDep = {'first': first, 'second':second, 'third': third}
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys=='main/third.first/second/third.second/third.')

        # One more level
        fourth = dict()
        second={'third':True, 'fourth':True}
        mainDep = {'first': first, 'second':second, 'third': third,'fourth':fourth};
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys=='main/third.fourth.first/second/third.fourth.second/third.fourth.')

        # the loops
        # simple to self
        first={'first':True}
        mainDep = {'first': first}
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys== 'main/first/> Cyclic Dependency.')


        # two levels loop
        first={'second':True}
        second={'first':True}
        mainDep = {'first': first, 'second':second};
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys== 'main/first/second/> Cyclic Dependency.'
                        'second/first/> Cyclic Dependency.')

        # the same as before, but now with loop
        second={'third':True, 'first':first}
        mainDep = {'first': first, 'second':second, 'third': third}
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys==
                        'main/third.first/second/third.> Cyclic Dependency.'
                        'second/third.first/> Cyclic Dependency.')

        # multyLayer loop
        fourth = {'first':True}
        third = {'second':True}
        second={'third':True, 'fourth':True}
        mainDep = {'first': first, 'second':second, 'third': third,'fourth':fourth}
        keys = bake.deptree(mainDep, mainDep, 'main', dict(), ' ')
        self.assertTrue(keys!=None, 'Error during tree processing')    
        self.assertTrue(len(mainDep)>0, 'Error during tree processing')
        self.assertTrue(keys=='main/first/second/fourth/> Cyclic'
                        ' Dependency.third/> Cyclic Dependency.fourth/first/second/>'
                        ' Cyclic Dependency.third/> Cyclic '
                        'Dependency.second/fourth/first/> Cyclic '
                        'Dependency.third/> Cyclic Dependency.third/second/fourth/first/>'
                        ' Cyclic Dependency.> Cyclic Dependency.')
        
    def test_printDependencies(self):
        """Tests the print of dependencies mechanism. """
        
        bake = Bake()
        first = SystemDependency()
        first.attribute('dependency_test').value ='passwd'
        first.attribute('more_information').value ='Missing the required dependency'
        first.attribute('name_apt-get').value = 'libpcap-dev'
        first.attribute('name_yum').value = 'libpcap-devel'
       
        mainDep = {'passwd': first};
        returnValue = bake.showSystemDependencies(mainDep, 'bakeconf.xml')
        self.assertTrue(returnValue!=None, 'Error during dependencies processing')    
        self.assertTrue(returnValue=='passwd')
        
        first.attribute('dependency_test').value ='nononononononononon'
        
        mainDep = {'nononononononononon': first}
        returnValue=None
        try:
            returnValue = bake.showSystemDependencies(mainDep, 'bakeconf.xml')
            self.fail("Should have stoped")
        except SystemExit as e: 
            self.assertTrue(returnValue==None, 'Error during dependencies processing')    
         
        second = SystemDependency()
        second.attribute('dependency_test').value ='passwd'
        second.attribute('more_information').value ='Has not the required dependency'
        mainDep = {'nononononononononon': first, 'passwd':second}
        returnValue=None
        try:
            returnValue = bake.showSystemDependencies(mainDep, 'bakeconf.xml')
            self.fail("Should have stoped")
        except SystemExit as e: 
            self.assertTrue(returnValue==None, 'Error during dependencies processing')    
 


# main call for the tests        
if __name__ == '__main__':
    unittest.main()
