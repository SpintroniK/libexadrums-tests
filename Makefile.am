## Process this file with automake to produce Makefile.in

# Copyright (C) 2018-2019 Jérémy Oden <jerem.oden@gmail.com>
# Copyright (C) 2018-2019 Nicolas Boulenguez <nicolas@debian.org>

# Cache shared with autoconf.
ACLOCAL_AMFLAGS = -I m4

PACKAGE = tests

dist_pkgdata_DATA = 

# Good habits.
AM_CXXFLAGS = -Wall
AM_LDFLAGS = -Wl,--as-needed

bin_PROGRAMS = tests

tests_CXXFLAGS = $(AM_CXXFLAGS) \
  $(alsa_CFLAGS) $(tinyxml2_CFLAGS) $(exadrums_CFLAGS) \
  -std=c++17
tests_LDADD = $(AM_LDADD) \
  -lstdc++fs \
  $(alsa_LIBS) $(tinyxml2_LIBS) $(exadrums_LIBS)

tests_SOURCES = \
  tests.cpp
