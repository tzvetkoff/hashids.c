# Headers to install.
include_HEADERS = hashids.h

# Libraries to build.
lib_LTLIBRARIES = libhashids.la
libhashids_la_SOURCES = hashids.c
libhashids_la_LIBADD = $(LIBM)
libhashids_la_LDFLAGS = -no-undefined -version-number 1:2:1

# Binaries to build.
bin_PROGRAMS = hashids
hashids_SOURCES = main.c
hashids_LDADD = $(LIBM) libhashids.la

# Test programs.
noinst_PROGRAMS = test
test_SOURCES = test.c
test_LDADD = $(LIBM) libhashids.la

# Tests.
TESTS=test
