AC_INIT(xdpd - Example driver, m4_esyscmd_s(cat VERSION), rofl@bisdn.de,  xdpd_example_fm)

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_GNU_SOURCE

AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LD
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AUTOCONF_ENV="SHELL=/bin/sh"
LT_INIT
link_all_deplibs=yes
link_all_deplibs_CXX=yes
AC_ENABLE_STATIC
LIBTOOL="$LIBTOOL --preserve-dup-deps"
AC_SUBST(LIBTOOL)dnl

# default flags
CFLAGS="-Werror -Wall $CFLAGS"
CXXFLAGS="-Werror -Wall $CXXFLAGS"
LDFLAGS=" -Wl,-export-dynamic"
AC_DEFINE(__STDC_FORMAT_MACROS)

#Driver enviorment. Must go first
m4_include([../../../../config/driver.m4])

#ROFL (Must always be included *before* no_pipeline_inline.m4)
m4_include([../../../../config/rofl.m4])

#Debug
m4_include([../../../../config/debug.m4])

# Check for profiling mode
m4_include([../../../../config/profiling.m4])

# Ensure rofl-pipeline platform functions are NOT inlined 
m4_include([../../../../config/no_pipeline_inline.m4])

AC_DEFINE(DRIVER_NAME,"example_platform")

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/hal-imp/Makefile
	src/hal-imp/openflow/openflow1x/Makefile
	src/pipeline-imp/Makefile
])

AC_OUTPUT
