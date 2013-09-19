#!/usr/bin/env python
from setuptools import setup, Extension


with open("README.rst", "r") as f:
    LONG_DESCRIPTION = f.read().strip()

with open("VERSION", "r") as f:
    VERSION = f.read().strip()

SOURCES = ['jellyfishmodule.c', 'jaro.c', 'hamming.c', 'levenshtein.c',
           'nysiis.c', 'damerau_levenshtein.c', 'mra.c', 'soundex.c',
           'metaphone.c', 'porter.c']

COMPILE_ARGS = ["-O3", "-std=c11", "-pg", "-fprofile-arcs", "-ftest-coverage"]

setup(name="jellyfish",
      version=VERSION,
      platforms=["any"],
      description=("a library for doing approximate and "
                   "phonetic matching of strings."),
      url="http://github.com/achernet/jellyfish",
      long_description=LONG_DESCRIPTION,
      classifiers=["Development Status :: 4 - Beta",
                   "Intended Audience :: Developers",
                   "License :: OSI Approved :: BSD License",
                   "Natural Language :: English",
                   "Operating System :: OS Independent",
                   "Programming Language :: Python",
                   "Topic :: Text Processing :: Linguistic"],
      ext_modules=[Extension(name="jellyfish",
                             sources=SOURCES,
                             extra_compile_args=COMPILE_ARGS,
                             extra_link_args=["-lgcov"])])
