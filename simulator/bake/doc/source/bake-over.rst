BAKE - Introduction
*******************

About Bake
==========

Bake is not a make, autoconf, or automake replacement. Bake is not a replacement for the package management tool that can be found on your home system. In short, Bake is not quite like what you are used to see around. Instead, Bake is an integration tool which is used by software developers to automate the reproducible build of a number of projects which depend on each other and which might be developed, and hosted by unrelated parties.

Bake was developed to automate the reproducible build of ns-3 taking into account that this build may be composed of a number of interdependent projects. Bake was developed to simplify the assembly of these pieces of software in a coherent and useful way.  This tutorial will show how to use bake and how to perform small configurations. Bake is an open source tool implemented in python, a deeper documentation of it can be found at `Bake's main page <http://planete.inria.fr/software/bake/index.html>`_. 

Features
========

* Automatic handling of dependencies
* Automatic download of the required sources
* Automatic correct build of the required modules
* Off-line installation and build capabilities
* Transparency, from the user's point of view, of the methods/tools used to store and build the used modules
* Fully configurable: Possible to add new modules, create complex build tasks, create predefined builds, among others

Limitations
===========

* Currently it works under Linux like systems
* The required missing system tools have to be installed by the user
  - Even though a --sudo option exists, to try to install dependencies, but should be used carefully and at the user's own risk

Prerequisites
=============
 First of all Bake is implemented in Python, so Python is required. Bake wraps a series of commands to make the life of the user easier.  However, it is required to have installed in the machine the third party tools used to download and build the modules. The user can verify the missing tools by calling 'bake.py check'. 
| > bake.py check
 First of all, Bake is developed in python, so to run Bake you have to have python on your machine. 

To check if you have python installed you can open a shell window and call: 
:: 

   > python -V

If as answer you receive the version of the installed python, preferably above 2.6.0, you do have python installed on your machine.   If you received a "command not found"  kind of message, you do not have python on your machine, or it is not on your path.  Try to talk to your administrator, or, if you have root rights,  you should try to:

::

  ubuntu > sudo apt-get install python 
  or
  fedora > sudo yum install python

To check if you have mercurial installed you can open a shell window and call:

:: 

    > hg version

If as answer you receive the version of the installed mercurial, preferably above 1.9.3, you do have mercurial installed on your machine.   If you received a "command not found" try to talk to your administrator, or, if you have root rights try:

::
 
   ubuntu > sudo apt-get install mercurial 
   or
   fedora > sudo yum install mercurial


After solving any mercurial related issues, you can download and install Bake by:


| **1. Downloading Bake**
|   You can get the bake code from the |ns3| code repository. 

::

  > hg clone http://code.nsnam.org/bake bake

This should create a bake repository on the directory you are in now

| **2. Put bake on the path**

::

  > export BAKE_HOME=`pwd`/bake
  > export PATH=$PATH:$BAKE_HOME:$BAKE_HOME/build/bin
  > export PYTHONPATH=$PYTHONPATH:$BAKE_HOME:$BAKE_HOME/build/lib

Bake utilizes a series of third party tools,  you can check if they exist in your system by calling:

::

  > bake.py check
 
After configuring one specific building scenario, you can check the modules selected and if the system dependencies are present by calling:

::

  > bake.py show
|  > Python - OK
|  > GNU C++ compiler - OK
|  > Mercurial - OK
|  > CVS - OK
|  > GIT - OK
|  > Bazaar - OK
|  > Tar tool - OK
|  > Unzip tool - OK
|  > Unrar tool - OK
|  > 7z  data compression utility - OK
|  > XZ data compression utility - OK
|  > Make - OK
|  > cMake - OK
|  > patch tool - OK
|  > autoreconf tool - OK
|
|  > Path searched for tools: /usr/lib64/qt-3.3/bin /usr/lib64/ccache /usr/local/bin /usr/bin /bin /usr/local/sbin /usr/sbin /sbin 

You may use bake to check and install the requirements if you call:
::

  > bake.py configure -p set-bake-env
  > bake.py show
  > bake.py download <--sudo> # if you are in the sudoers group and you want bake to try, in a best effort way, to install it own dependencies. 


Basic usage
===========
A typical user session should be:::

  > wget http://code.nsnam.org/bake/raw-file/tip/bakeconf.xml
  > bake.py show --available
  > bake.py configure -e <one of the available modules>
  > bake.py deploy

The result:

* The wget command will download the last version of the bake configuration file
* **bake.py show available** will show all the available modules
* After configuring bake with **bake.py configure** a bakefile.xml, containing the output of configuration step should be created on the same directory the user called bake configure
* After calling **bake.py deploy** two directories, build and source should have been created. Source will have one directory for each module downloaded and the build will contains the installed object files for all the built modules. 

The installation process may be broken into download and build, in this way the user just need to be online to perform the download and the build may be done later, even offline. 

In this case the steps should be:::

  > wget http://code.nsnam.org/bake/raw-file/tip/bakeconf.xml
  > bake.py show --available
  > bake.py configure -e <one of the available modules>
  > bake.py download
    <-- Later, even if the user is offline -->
  > bake.py build

If something goes wrong:

* Sometimes the download, build or installation of some of the modules may fail in that case there are a --stop-on-error parameter may be passed so that the process stops on the first error it founds so that the user can analyze it and find a solution. The stop on error feature is best used  with -v or -vvv so that more information about the error.

