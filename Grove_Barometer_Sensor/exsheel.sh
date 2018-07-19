#!/bin/sh
swig -c\+\+ -python BMP085.i
python setup.py build_ext --inplace



