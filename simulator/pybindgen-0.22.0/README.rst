About
=====
PyBindGen is (surprise!) a python bindings generator.  The main features are:

 * Generates clean C or C++ code, nearly as readable as code written
   manually by a developer;
 * Generated code is self contained and does not require any external
   libraries or macros; after generating the python module only python
   header files are required, nothing else;
 * Does not require Boost.Python (no C++ template magic, also works in C);
 * The interface for code generation is a simple Python API, not some
   obscure interface definition language.  Additionally PyBindGen can
   parse header files with gccxml, if gccxml and pygccxml are
   installed in the system (note: pygccxml has not been ported to Python 3 yet);
 * Can be easily extended with new type handlers;
 * Type handlers can allocate memory and register cleanup code to free;
 * Supports in, out, and inout parameters (e.g. with pointers or C++ references);
 * Supports multiple return values (e.g. due to out/inout parameters);
 * Supports wrapping of simple C++ classes;
 * Supports virtual methods in classes;
 * Supports reference counted classes and, to some extent, smart pointers;
 * Multiple inheritance;
 * Wrapping templated classes;

Notable features NOT implemented:

 * Converting exceptions from Python to C++ (only from C++ to Python is currently supported);
 * Callbacks.

Supported Python versions
=========================

See the `documentation <http://pybindgen.readthedocs.org/en/latest/tutorial/#supported-python-versions>`_.

Installation
============

NOTE: if checking out pybindgen from bazaar, do not forget to see the
file HACKING for additional instructions.

Before proceeding make sure the system requirements are met.  PyBindGen requires:

 1. Python (http://www.python.org)
 2. Python development files (the `python-dev` package in Ubuntu/Debian, for example)
 3. A C/C++ Compilation tool-chain (`apt-get install build-essential`)
 4. (optional) GCCXML and `PyGCCXML <http://www.language-binding.net/pygccxml/download.html>`_

You can install PyBindGen using either the `setup.py` or WAF.  Note: to
be able to run all tests, which involve code generation, and
subsequent compilation, you need to use WAF.

Installation using setup.py:
----------------------------

	python setup.py install

Installation using WAF
----------------------

PyBindGen uses `WAF <http://code.google.com/p/waf/>`_ as main build system.
However, WAF does not have to be previously installed, as it is
shipped with PyBindGen in a single `waf` script.

To install PyBindGen, issue the following commands (win32 users should omit the ./ prefix in commands):

1. ./waf configure
   - optionally you may add the option --prefix /foo/bar. To select a non-defaul python version, use the PYTHON environment variable, e.g.::

        PYTHON=/usr/bin/python2.4 ./waf configure

2. ./waf
    - possible options: `-jN` for parallel build, `-p` for progress bar
3. ./waf check
    - optional step, runs the unit tests
4. ./waf --examples
    - optional step, compiles the examples
5. ./waf install
    - may require sudo


Windows specific notes
----------------------

WAF concurrency bugs
++++++++++++++++++++


WAF automatically detects the number of cores and tries to activate
multiple build threads accordingly.  However, this concurrency support
appears to be buggy on Windows, therefore you should disable it with the
-j1 option, if you have multiple CPUs:

	waf check -j1

Compiler selection
++++++++++++++++++

Note that if you are compiling on win32, WAF will look for MSVC
(MicroSoft Visual C) by default and give up on finding a C/C++
compiler if not found.  If you do not have MSVC installed but instead
have MinGW or CygWin GCC, you have to tell WAF to look for GCC in the
configure stage:

	waf configure --check-c-compiler=gcc --check-cxx-compiler=g++

Installation
++++++++++++

On win32, `waf install` installs to a Temp folder by default.  To have
it install for a certain Python version, use the --prefix option to
waf configure.  For instance:
   
	waf configure --prefix C:\Python26
	waf install



Installation failsafe
---------------------

If by any chance you have trouble with WAF and are just looking to
install PyBindGen, you should know that PyBindGen is entirely
self-contained in the `pybindgen` directory.  You can simply
recursively copy the entire pybindgen folder into Python's
site-packages directory, and that's it!  PyBindGen is a pure Python
package and does not actually require a C/C++ compiler; a C++ compiler is only
used for code generation unit tests and compiling the example modules, and it is not needed to generate code.


Documentation
=============

The following documentation is available:

 1. `API docs (with introductory tutorial) <http://pybindgen.readthedocs.org/en/latest/>`_ 
 2. Many simple examples in the `examples` directory
 3. Advanced examples in the unit tests (`tests/`)
 4. The source code!

.. image:: https://travis-ci.org/gjcarneiro/pybindgen.svg?branch=master
    :target: https://travis-ci.org/gjcarneiro/pybindgen

