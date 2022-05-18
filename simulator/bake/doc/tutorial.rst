How do you get started with bake ?
==================================

A typical user session to build everything:::

  wget http://www.nsnam.org/bakeconf.xml
  bake configure -a
  bake download
  bake build

The result:

* bakefile.xml: contains output of configuration step
* 'source' directory: contains downloaded code:
** one file per tarball downloaded
** one directory per module
** one directory/objdir per module which contains all object files built
* 'build' directory: contains the libraries, headers, and binaries installed by each module

A more advanced user:::

  wget http://www.nsnam.org/bakeconf.xml
  mkdir opt
  cd opt
  bake configure -c ../bakeconf.xml -a --sourcedir=../source --builddir=. --objdir=opt --set=CFLAGS=-O3
  bake download
  bake build
  # now, build debug version
  cd ..
  mkdir dbg
  cd dbg
  bake configure -c ../bakeconf.xml -a --sourcedir=../source --builddir=. --objdir=dbg --set=CFLAGS=-O0
  bake build

The result:

* opt/bakefile.xml contains optimized build configuration
* dbg/bakefile.xml contains debug build configuration
* source contains all source code:
** source/module/opt contains optimized object files
** source/module/dbg contains debug object files
* opt/ contains headers, libraries, binaries for optimized build
* dbg/ contains headers, libraries, binaries for debug build


