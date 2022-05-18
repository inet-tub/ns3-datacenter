#!/usr/bin/env python
from setuptools import setup

with open('README.rst') as file_:
    LONG_DESCRIPTION = file_.read()


setup(name='PyBindGen',
      use_scm_version={"version_scheme": "post-release",
                       "write_to": "pybindgen/version.py"},
      setup_requires=['setuptools_scm'],
      description='Python Bindings Generator',
      author='Gustavo Carneiro',
      author_email='gjcarneiro@gmail.com',
      url='https://github.com/gjcarneiro/pybindgen',
      packages=['pybindgen',
                'pybindgen.typehandlers',
                'pybindgen.typehandlers.ctypeparser',
                ],
      long_description=LONG_DESCRIPTION,
      classifiers=[
          'Development Status :: 4 - Beta',
          'Intended Audience :: Developers',
          'Topic :: Software Development :: Build Tools',
          'License :: OSI Approved :: GNU Lesser General Public License v2 or later (LGPLv2+)',
          'Programming Language :: Python :: 2',
          'Programming Language :: Python :: 3',
      ],
)
