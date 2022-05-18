******************
BAKE Documentation
******************

The bake documentation was written using Sphinx (http://sphinx-doc.org/) 
and Pygments (http://pygments.org/). To generate the documentation, 
first please be sure you have Sphinx and Pygments on your 
machine. If it is not installed you can, for example, call:

 > sudo yum install python-sphinx 
   or 
 > sudo apt-get install python-sphinx
 
and
 
 > easy_install Pygments
   or
 > sudo yum install python-pygments
   or
 > sudo apt-get install python-pygments
   or
 > hg clone http://bitbucket.org/birkenfeld/pygments-main pygments
 > cd pygments
 > sudo python setup.py develop
 
depending on the system you are using. After that, just call make 
on the doc directory, choosing one the of the available formats: 

  html       to make standalone HTML files
  dirhtml    to make HTML files named index.html in directories
  singlehtml to make a single large HTML file
  pickle     to make pickle files
  json       to make JSON files
  htmlhelp   to make HTML files and a HTML help project
  qthelp     to make HTML files and a qthelp project
  devhelp    to make HTML files and a Devhelp project
  epub       to make an epub
  latex      to make LaTeX files, you can set PAPER=a4 or PAPER=letter
  latexpdf   to make LaTeX files and run them through pdflatex
  text       to make text files
  man        to make manual pages
  changes    to make an overview of all changed/added/deprecated items

For example: 
   [~/bake/doc]> make html
 
