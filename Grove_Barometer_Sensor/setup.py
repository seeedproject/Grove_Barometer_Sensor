#!/usr/bin/env python

""""
setup.py file for SWIG C\+\+/Python MutichannelGasSensor
"""

from distutils.core import setup,Extension
BMP085_module = Extension('_BMP085',
                          sources=['BMP085.cpp','i2crw.cpp','BMP085_wrap.cxx',],
                          )

setup (name = 'BMP085',
        version = '0.1',
        author = "www.seeed.com",
        description = """ C\+\+/Python BMP085""",
        ext_modules = [BMP085_module],
        py_modules = ["BMP085"],
        )