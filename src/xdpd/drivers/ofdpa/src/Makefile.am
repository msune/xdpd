MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = hal-imp ofdpa pipeline-imp

noinst_LTLIBRARIES = libxdpd_driver_ofdpa_src.la

libxdpd_driver_ofdpa_src_la_SOURCES =
libxdpd_driver_ofdpa_src_la_LIBADD = \
		hal-imp/libxdpd_driver_ofdpa_hal_imp.la\
		ofdpa/libxdpd_driver_ofdpa_ofdpa.la\
		pipeline-imp/libxdpd_driver_ofdpa_pipeline_imp.la\
		-lrpc_client

libxdpd_driver_ofdpa_src_la_LDFLAGS=-static
