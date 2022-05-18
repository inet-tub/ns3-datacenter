
Bake - Tutorial
---------------

Introduction
************

This tutorial will present a walk through over all the steps to set, configure, download and build a new module using Bake.

Bake is a tool for distributed software builds, developed for the ns-3 project, but it is general and could be used to help on the development of other open-source projects requiring integration of multiple projects.  First of all, Bake is developed in python, so you have to have python on your machine.  This walk-through will consider that the user is using a Linux, Fedora or Ubuntu distributions, with bash. 

Conventions:
************
|    During the tutorial we will use the following conventions:
|     **> [command]** - For now, bake is only Linux and Mac OS X compatible, the “>” means the prompt of the terminal, and what comes next, [command] , is the command that should be executed on a bash shell screen. It also means that the operation is distribution independent.
|     **Mac OS X >**  - Designates commands that are specific for Mac OS X distributions
|     **Ubuntu >**  - Designates commands that are specific for Ubuntu distributions
|     **Fedora >**  - Designates commands that are specific for Fedora distributions
|     **$BAKE_HOME** -  is the home directory for bake
|     **$HOME** - the user home directory, this tutorial considers that, by default, all the actions will be performed from this directory. Whenever one need a relative path, it will be given regarding the HOME directory.  For example, if  HOME=/home/username  $HOME/bake would be /home/username/bake.

Third party software
********************

Bake uses a series of third party tools that, not necessarily, come with standard Linux machines distributions, however, they are usually required.   Each given module has a specific set of tools that are required to build it, and each module is different. If you never builds a module that is stored, for example, in a mercurial repository, you do not need to have mercurial in your machine. However, if the module requires it and you do not have it installed... you will not be able to install that specific module.  Apart from python, no other third party software is mandatory, however, it is highly recommended to have all these tools installed on your machine.  To check if you have all the third party software installed on your machine you can run “> bake.py check”.  However, to do that you need to have Python installed.

To check if you have python installed you can do the following:
1. Open a shell window and type:

::
 
 > python -V

If as answer you receive the version of the installed python, preferably above 2.7.3, you do have python installed on your machine.   If you received a “command not found”  kind of message, you do not have python on your machine, or it is not on your path.  Try to talk to your administrator, or, if you have root rights,  you should try to:


::
 
 Ubuntu > sudo apt-get install python
 or
 Fedora > sudo yum install python

After solving any python related issues, you can check the third party tools bake relies on by  calling:

::
 
 > bake.py check

Again, just to remember, the tools you will really need are linked to the modules you will install.  For many users a small subset of those tools are more than enough.

Bake installation
*****************

1. Downloading Bake
###################

You can get the bake code from the ns-3 code repository. However, do do so you will 
need to have mercurial installed in your machine. 

::
 
 Mac OS X > sudo port install mercurial
 or
 Ubuntu > sudo apt-get install mercurial
 or
 Fedora > sudo yum install mercurial



::
 
  > hg clone http://code.nsnam.org/bake bake

This should create a bake repository on the directory you are in now

2. Putting bake on the path
###########################

bake will, by default, use a ``build`` directory to install components
locally (rather than trying to install them in system places).  This
``build`` directory will contain ``bin`` and ``lib`` directories.  These
should be put into your environment variables for PATH and PYTHONPATH:

Change directory into the ``bake`` directory (the directory including
the file ``bake.py``) and type these commands:

::
 
 > export BAKE_HOME=`pwd`
 > export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
 > export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib

For this whole documentation section, bake will be assumed to be found on 
the path.  These path extensions can also be adapted and added to a local
shell file (e.g. ~/.bashrc).

Now, from any place of your machine you can call bake.py.

::
 
 > bake.py
 Usage: bake.py [options] command [command options]

 Where command is one of:
   deploy       : Downloads the configured modules AND makes the build in one step
   configure    : Setup the build configuration (source, build, install directory,
                  and per-module build options) from the module descriptions
   fix-config   : Update the build configuration from a newer module description
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
   bake.py command --help

 Options:
   -h, --help            show this help message and exit
   -f CONFIG_FILE, --file=CONFIG_FILE
                         The Bake file to use, and the target
                         configuration/reconfiguration. Default: bakefile.xml.
   --debug               Prints out all the error messages and problems.
   --noColor             Print messages with no color
   -V                    Prints the version of Bake

Basic Bake usage
****************

To run bake, first of all,  you need the configuration file, that describes how the modules should be built. By default this file is called bakefile.xml. Bake includes a generic configuration file, called bakeconf.xml that is shipped with bake ($HOME/bake/bakeconf.xml) or with the standard ns-3 distribution.  This file contains the generic information for all the available modules that bakes can handle.  We will see later how to create a new input to this generic configuration file. But for now it is important to understand that the bakeconf.xml will work as a template for your personal and specific configuration. On the bakeconf.xml we have all the available modules for ns-3 and more, you should choose a subset of these to be installed on your system.
 
To configure bake to build, for example, ns-3-dev, you can:

::
 
 > bake.py configure -e ns-3-dev 

This command will create a specific configuration file on the local directory called bakefile.xml. This file has ns-3-dev and all the optional modules enabled to download and build. By default the file will be configured to download the required source files at the “source” directory and install them at the “build” directory. Optionally one can do something like this 

::
 
 > bake.py -f nonStandardName.xml configure -c $BAKE_HOME/bakefile.xml -e ns-3-allinone --installdir=/tmp/installBake --sourcedir=/tmp/sourceBake

This will create the nonStandardName.xml configuration file on the local directory and will set the sources to be stored at “/tmp/sourceBake” and the installation directory to be “/tmp/installBake”.

To create the executable instance of ns-3 you can either call:

::
 
  > bake.py deploy

this will make the download and build of the selected modules in one step. Alternatively you can use bake to download all the required files and install ns-3 afterwards even in off-line mode.  To do this you should call:

::
 
 > bake.py download
 
This will download the modules to the configured target, e.g. /tmp/sourceBake, or by default, will create a source directory inside the current one.  After that, to perform the build and installation of the compiled modules, the user should call:

::
 
 > bake.py build

This will create a consistent version of ns-3 and its libraries in the configured target repository, e.g. /tmp/installBake or, by default, it creates a build directory inside the current one.   This, if you have no missing third party software, should be enough to have a full ns-3 version with all the configured modules working and waiting for your experiments.

To receive more information about the ongoing process one can call bake in verbose mode.  The verbose varies from -v to -vvv that is the maximum verbose level. If one call bake with -vvv all the messages from the third part tools will be showed.

::
   
 > bake.py build -vvv


System dependencies
*******************

Some modules may be dependent on third party packages. Such dependencies are expressed
as SystemDependencies, by default, such dependencies, if not installed in the machine
will show up as failures during the download process. 

The typical way to express a system dependency is to check for the existence
of a file such as an executable or a header file in a well-known system
place.  If the paths searched for the dependency do not include the
actual path, a false negative result may occur.  There exist other 
more platform-specific ways to check for system dependencies, such as
pkg-config or using the system package manager, or performing a 
configuration check by trying to compile a small test program, but bake does 
not presently support such additional checking.

The following snippet shows an example in which a ``file_test`` may be 
expressed, asking bake to check for the presence of a library at the
path location ``/usr/lib/debug/lib64/ld-linux-x86-64.so.2.debug``.

::

   <module name="libc-debug">
      <source type="system_dependency">
        <attribute name="file_test" value="/usr/lib/debug/lib64/ld-linux-x86-64.so.2.debug"/>
        <attribute name="name_apt-get" value="libc6-dbg"/>
        <attribute name="name_yum" value="glibc-debuginfo"/>
        <attribute name="more_information" value="Didn't find:   libc debug package; please install it."/>
      </source>
      <depends_on name="libc" optional="False"/>
      <build type="none" objdir="no">
      </build>
    </module>

This states that libc-debug depends on a mandatory ``libc`` module, and has
no ``build type``; it must be installed on the system, usually by a package
manager.  Two source attributes, ``name_apt-get`` and ``name_yum``, provide
hints as to the names of packages that could be installed if the dependency
check fails.  The mandatory ``more_information`` attribute provides
additional hints to the user should the check fail.

The ``file_test`` and ``executable_test`` attributes may specify more than 
one possible paths to check, with possible values separated by the ``or`` 
keyword such as:

::

        <attribute name="file_test" value="/usr/lib/debug/lib64/ld-linux-x86-64.so.2.debug or /usr/local/lib/debug/lib64/ld-linux-x86-64.so.2.debug"/>

Presently, ``file_test`` and ``executable_test`` do not support wildcard
characters for path values.

A deprecated source attribute called ``dependency_test`` exists, which is 
similar to ``executable_test``, but doesn't distinguish between header files,
libraries, and executables.

Configuration
**************
Bake has a template file, where we store the configuration of all the modules bake is able to handle, and a personal configuration, created by the user, to inform bake about his/her personal requirements. The template file, that contains the configuration for all the modules Bake is able to handle, is called by default bakeconf.xml and  it is shipped with bake and ns-3 distributions.  By default Bake will use the one in the local directory, if there is none, it will use the one on $BAKE_HOME. The file can also be informed through the -c option.  One could use, for example:

::
 
 > bake.py -f myPersonalConfig.xml configure -c nonStandardName.xml -e ns-3-allinone -d netanim-dev

This command will use nonStandardName.xml as a template to create the user’s configuration file, myPersonalConfig.xml. To see the activated modules on myPersonalConfig.xml do the following:

::
 
  > bake.py -f myPersonalConfig.xml show
  module: python-dev (enabled)
    No dependencies!
  module: pygraphviz (enabled)
    No dependencies!
  module: pygoocanvas (enabled)
    No dependencies!
  module: gccxml-ns3 (enabled)
    No dependencies!
  module: nsc-dev (enabled)
    No dependencies!
  module: click-dev (enabled)
    No dependencies!
  module: libxml2-dev (enabled)
    No dependencies!
  module: pygccxml (enabled)
    depends on:
       gccxml-ns3 (optional:False)
  module: pyviz-prerequisites (enabled)
    depends on:
       python-dev (optional:True)
       pygraphviz (optional:True)
       pygoocanvas (optional:True)
  module: openflow-dev (enabled)
    depends on:
       libxml2-dev (optional:False)
  module: pybindgen-dev (enabled)
    depends on:
       pygccxml (optional:True)
       python-dev (optional:True)
  module: ns-3-allinone (enabled)
    depends on:
       netanim-dev (optional:True)
       nsc-dev (optional:True)
       pybindgen-dev (optional:True)
       pyviz-prerequisites (optional:True)
       click-dev (optional:True)
       openflow-dev (optional:True)

  -- System Dependencies --
   > libxml2-dev - OK
   > pygoocanvas - Missing
     >> The pygoocanvas is not installed, try to install it.
     >> Try: "sudo yum -y install pygoocanvas", if you have sudo rights.
   > pygraphviz - Missing
     >> The pygraphviz is not installed, try to install it.
     >> Try: "sudo yum -y install py27-pygraphviz", if you have sudo rights.
   > python-dev - OK

Pay attention that the results bake show for myPersonalConfig.xml and
nonStandardName.xml are different, since we disabled netanim-dev
(-d netanim-dev).  Some dependencies are missing (pygoocanvas and pygraphviz).
These are referred at pyviz-prerequisites, that is by its turn an optional module
of ns-3-allinone. To have a more visual description of the enabled module you can also
call:
 
::
 
  > bake.py -f myPersonalConfig.xml show
  module: click-dev (enabled)
    No dependencies!
  module: libxml2-dev (enabled)
    No dependencies!
  module: gccxml-ns3 (enabled)
    No dependencies!
  module: python-dev (enabled)
    No dependencies!
  module: pygraphviz (enabled)
    No dependencies!
  module: pygoocanvas (enabled)
    No dependencies!
  module: nsc-dev (enabled)
    No dependencies!
  module: openflow-dev (enabled)
    depends on:
       libxml2-dev (optional:False)
  module: pygccxml (enabled)  
    depends on:
       gccxml-ns3 (optional:False)
  module: pyviz-prerequisites (enabled)
    depends on:
       python-dev (optional:True)
       pygraphviz (optional:True)
       pygoocanvas (optional:True)
  module: pybindgen-dev (enabled)
    depends on:
       pygccxml (optional:True)
       python-dev (optional:True)
  module: ns-3-allinone (enabled)
    depends on:
       netanim-dev (optional:True)  
       nsc-dev (optional:True)
       pybindgen-dev (optional:True)
       pyviz-prerequisites (optional:True)
       click-dev (optional:True)
       openflow-dev (optional:True)
  
  -- Enabled modules dependency tree --
  +-enabled/
    |
    +-click-dev
    |
    +-gccxml-ns3
    |
    +-libxml2-dev
    |
    +-ns-3-allinone/  
    | |
    | +-click-dev (optional)
    | |
    | |
    | +-nsc-dev (optional)
    | |
    | +-openflow-dev/ (optional)
    | | |
    | | +-libxml2-dev (mandatory)
    | |
    | +-pybindgen-dev/ (optional)
    | | |
    | | +-pygccxml/ (optional)
    | | | |
    | | | +-gccxml-ns3 (mandatory)
    | | |
    | | +-python-dev (optional)
    | |  
    | +-pyviz-prerequisites/ (optional)
    |   |
    |   +-pygoocanvas (optional)
    |   |
    |   +-pygraphviz (optional)
    |   |  
    |   +-python-dev (optional)
    |
    +-nsc-dev
    |
    +-openflow-dev/
    | |
    | +-libxml2-dev (mandatory)
    |
    +-pybindgen-dev/
    | |
    | +-pygccxml/ (optional)
    | | |
    | | +-gccxml-ns3 (mandatory)
    | |
    | +-python-dev (optional)
    |
    +-pygccxml/
    | |
    | +-gccxml-ns3 (mandatory)
    |  
    +-pygoocanvas
    |
    +-pygraphviz
    |
    +-python-dev
    |
    +-pyviz-prerequisites/
    |
    +-pygoocanvas (optional)
    |
    +-pygraphviz (optional)
    |
    +-python-dev (optional)
  
  -- System Dependencies --
   > libxml2-dev - OK
   > pygoocanvas - Missing
     >> The pygoocanvas is not installed, try to install it.
     >> Try: "sudo yum -y install pygoocanvas", if you have sudo rights.
   > pygraphviz - Missing
     >> The pygraphviz is not installed, try to install it.
     >> Try: "sudo yum -y install py27-pygraphviz", if you have sudo rights.
   > python-dev - OK


The configuration file
**********************

If you are a developer, and you want to add your module to bake how should you proceed to create the configuration input?

First you need to create a new xml entry on the configuration file so that bake can understand the actions it should perform. Lets take DCE as an example of configuration for a new module:


.. code-block:: xml
   :linenos:

    <module name="dce-ns3">
       <source type="mercurial">
    <attribute name="url" value="http://code.nsnam.org/furbani/ns-3-dce"/>
     <attribute name="module_directory" value="dce"/>
       </source>
       <depends_on name="ns-3-dev-dce" optional="False"/>
       <depends_on name="iperf" optional="True"/>
       <depends_on name="ccnx" optional="True"/>
       <build type="waf" objdir="build">
    <attribute name="configure_arguments"
                   value="configure --prefix=$INSTALLDIR --verbose"/>
    <attribute name="post_installation"
                  value="export DCE_PATH=$INSTALLDIR/bin:$INSTALLDIR/bin_dce;
                  export DCE_ROOT=$INSTALLDIR;
                  export LD_LIBRARY_PATH=$INSTALLDIR/lib:$INSTALLDIR/bin:
                                         $SRCDIR/../ns-3-dev-dce/build;
                                         $INSTALLDIR/bin/ns3test-dce --verbose"/>
        <attribute name="v_PATH" value="$SRCDIR;$SRCDIR/../build/bin" />
        <attribute name="v_LD_LIBRARY"
                   value="$OBJDIR/..;
                          $SRCDIR;$SRCDIR/../ns-3-dev-dce/build;$INSTALLDIR" />
        <attribute name="v_PKG_CONFIG"
                   value="$SRCDIR/../ns-3-dev-dce/build/lib/pkgconfig;
                   $OBJDIR/../../ns-3-dev-" />
        <attribute name="new_variable"
                   value="DCE_PATH=$INSTALLDIR/bin:$INSTALLDIR/bin_dce;
                          DCE_ROOT=$INSTALLDIR;
                          LD_LIBRARY_PATH=$INSTALLDIR/lib:$INSTALLDIR/bin:$SRCDIR/../ns-3-dev-dce/build" />
     </build>
   </module>

This defines how bake should download and build the code of DCE and which are the dependencies of DCE code. Now lets take a look on the code step by step so that we can understand what is going on and how we can do the same for a new code:.

.. code-block:: xml
   :linenos:
 
   <module name="dce-ns3">
   <!-- This defines that a new module will be created and its name -->

     <source type="mercurial">
     <!-- The type of the source code repository tool in use.
          Can be CVS, mercurial, SVN, Bazaar and archive. -->

         <attribute name="url" value="http://code.nsnam.org/furbani/ns-3-dce"/>
         <!-- The address of the target repository. -->

         <attribute name="module_directory" value="dce"/>
         <!-- The target directory the source should be downloaded to -->
     </source> <!-- End of the source handling data -->

     <!-- Here we treat the dependencies of dce. The dependencies may be optional
          or not, the lack of an optional dependency decreases the functionality
          of the module, but do not prevent its successful installation, mandatory
          dependencies on the other hand, do.  The dependencies below are, in the
          order, ns-3, iperf and ccnx. The ns-3 code is mandatory since DCE can
          not work without it. Iperf and CCNx are applications that can be used
          together DCE and are used as examples of DCE power, but are not
          essential for DCE. -->
      <depends_on name="ns-3-dev-dce" optional="False"/>
      <depends_on name="iperf" optional="True"/>
      <depends_on name="ccnx" optional="True"/>

      <!-- Here we set how bake should perform the build of the system. -->
      <build type="waf" objdir="build">
      <!-- The build tool used for building DCE is waf, and it will be stored
           on the directory build. -->

          <attribute name="configure_arguments" value="configure --prefix=$INSTALLDIR --verbose"/>
          <!-- These are the set of parameters that should be passed as
               parameter to the waf configure --prefix=$INSTALLDIR --verbose. -->

          <attribute name="post_installation" value="export
          DCE_PATH=$INSTALLDIR/bin:$INSTALLDIR/bin_dce;export DCE_ROOT=$INSTALLDIR;
          export LD_LIBRARY_PATH=$INSTALLDIR/lib:$INSTALLDIR/bin:$SRCDIR/../ns-3-dev-dce/build;
          $INSTALLDIR/bin/ns3test-dce --verbose"/>
        <!-- These are the list of shell commands Bake should execute
               after performing the build. -->

         <!-- These are the overload of the standard PATH, LD_LIBRARYPATH,
              PKG_CONFIG_PATH and a list of new variables that are
              required by DCE. -->
          <attribute name="v_PATH" value="$SRCDIR;$SRCDIR/../build/bin" />
          <attribute name="v_LD_LIBRARY" value="$OBJDIR/..;
                           $SRCDIR;$SRCDIR/../ns-3-dev-dce/build;$INSTALLDIR" />
          <attribute name="v_PKG_CONFIG" value="$SRCDIR/../ns-3-dev-dce/build/lib/pkgconfig;
                           $OBJDIR/../../ns-3-dev-" />
          <attribute name="new_variable" value="DCE_PATH=$INSTALLDIR/bin:$INSTALLDIR/bin_dce;
                           DCE_ROOT=$INSTALLDIR;
                           LD_LIBRARY_PATH=$INSTALLDIR/lib:$INSTALLDIR/bin:$SRCDIR/../ns-3-dev-dce/build" />
       </build> <!-- End of the build handling data -->
    </module> <!-- End of the module -->

Considering that the standard configuration file has the inputs for the dependencies, this is enough to teach Bake how to build DCE.  After adding this to the standard configuration file, or to the **~/.bakerc** file, one can simply call bake configure passing the name of the new module ‘’dce-ns3’’ as parameter.  Each configuration entry has to have a name, a source section and a build section.  

Predefined configurations
*************************

Tasks that are often made can be automatized with Bake by creating predefined entries.  After creating a predefined entry either on the configuration or in the ~\.bakerc file, the predefined configuration tag may be called with:

::

 > bake.py configure -p <name_of_the_predefined_tag>

A predefined configuration entry may looks like:

.. code-block:: xml
   :linenos:
 
    <predefined name="dce-min-tmp"> <!-- name of the predefined tag -->
      <enable name="dce-ns3"/>  <!-- module to enable -->

      <!-- disables optional modules -->
      <disable name="iperf"/>
      <disable name="ccnx"/>

      <!-- Changes the bin and source directories to /tmp -->
      <configuration installdir="/tmp/tmpBin" sourcedir="/tmp/tmpSource"/>

      <!-- Configures ns-3 to enable the modules  core,network and WiFi,
           it appends so it does not change the default configuration -->
     <append module="ns-3-dev-dce" name="configure_arguments"
             value=" --enable-modules=core,network,WiFi"/>
    </predefined> <!-- End of the predefined configuration -->

Thus, after adding this to the end of the bakeconf.xml file we can configure bake to download DCE without any optional module by calling:

::

 > bake.py configure -p dce-min-tmp
 >bake.py show
 module: ns-3-dev-dce (enabled)
   No dependencies!
 module: dce-ns3 (enabled)
   depends on:
      ns-3-dev-dce (optional:True)
      iperf (optional:True)
      ccnx (optional:True)

We can observe that only the **ns-3-dev-dce** and **dce-ns3** modules are enabled.

~/.bakerc configuration file
****************************

The last configuration made is stored in a file called **.bakerc** that is automatically
created in the user's home directory. One can also use this file to extend the
configuration file by creating personalized predefine actions. This way actions
that are repeated often can be codified in the **.bakerc** for future use. The last
 configuration command issued by the user is stored as last in the **.bakerc**, i.e.
it is possible to call:

::

  > bake.py configure -p last

To have the last configuration action repeated. One example of .bashrc:

.. code-block:: xml
   :linenos:

    <?xml version="1.0" ?>
    <configuration>
      <predefined name="dce-min">
        <enable name="dce-meta-dev"/>
        <disable name="elf-loader"/>
        <disable name="iperf"/>
        <disable name="ccnx"/>
        <disable name="wget"/>
        <disable name="thttpd"/>
        <configuration installdir="/tmp/tmpBin" sourcedir="/tmp/tmpSource"/>
        <append module="ns-3-dev" name="configure_arguments" value=" --enable-modules=core,network,wifi"/>
      </predefined>
      <predefined name="my-ns3">
        <enable name="ns-3-dev"/>
        <append name="configure_arguments" value=" --enable-modules=core,network,wifi"/>
      </predefined>
      <predefined name="last">
        <enable name="thttpd"/>
      </predefined>
    </configuration>


