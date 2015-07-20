AC_INIT(xdpd - OF-DPPA driver, m4_esyscmd_s(cat VERSION), xdpd-devel@xdpd.org, xdpd-ofdpa, http://www.xdpd.org)

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
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
CFLAGS="-Werror -Wall -Wl,-export-dynamic $CFLAGS"
CXXFLAGS="-Werror -Wall -Wl,-export-dynamic $CXXFLAGS"
AC_DEFINE(__STDC_FORMAT_MACROS)

# Driver
m4_include([../../../../config/driver.m4])

# ROFL
m4_include([../../../../config/rofl.m4])

# Debug
m4_include([../../../../config/debug.m4])

# Check for profiling mode
m4_include([../../../../config/profiling.m4])

# Ensure rofl-pipeline platform functions are NOT inlined 
m4_include([../../../../config/no_pipeline_inline.m4])

# Add OFDPA to LDFLAGS
CPPFLAGS="-I$XDPD_SRCDIR/libs/ofdpa/src/include/ $CPPFLAGS"
#LDFLAGS="-L$XDPD_SRCDIR/libs/ofdpa/bin/ly2-trident-brl20/ $LDFLAGS"
LDFLAGS="-L$XDPD_SRCDIR/libs/ofdpa/bin/im-x86-trident-ub12/ $LDFLAGS"

AC_DEFINE(DRIVER_NAME,"ofdpa")

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/hal-imp/Makefile
	src/hal-imp/openflow/openflow1x/Makefile
	src/ofdpa/Makefile
	src/pipeline-imp/Makefile
])

AC_OUTPUT
