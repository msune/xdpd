MAINTAINERCLEANFILES = Makefile.in

#Defines driver lib
FM_LIB= drivers/$(PLATFORM)/libxdpd_driver_$(PLATFORM).la
FM_LIB_a= drivers/$(PLATFORM)/.libs/libxdpd_driver_$(PLATFORM).a

SUBDIRS = management openflow drivers

#Wrap
noinst_LTLIBRARIES = libxdpd_wrap.la
libxdpd_wrap_la_SOURCES = 
libxdpd_wrap_la_LIBADD = \
	management/libxdpd_mgmt.la \
	-lrofl_datapath \
	openflow/libxdpd_openflow.la\
	$(FM_LIB)
libxdpd_wrap_la_LDFLAGS=-static

#Main program
sbin_PROGRAMS = xdpd
xdpd_SOURCES = cmm.cc xdpd.cc
xdpd_LDADD = \
	libxdpd_wrap.la \
	openflow/libxdpd_openflow.la \
	-lrofl_datapath \
	-lrofl_common \
	$(FM_LIB)

xdpd_LDFLAGS = $(xdpd_HW_LDFLAGS) -static
