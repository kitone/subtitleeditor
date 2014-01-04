pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions

AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) $(GSTREAMER_CFLAGS)

pluginlib_LTLIBRARIES = \
	libwaveformmanagement.la

libwaveformmanagement_la_SOURCES = \
	mediadecoder.h \
	waveformgenerator.cc \
	waveformmanagement.cc

libwaveformmanagement_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libwaveformmanagement_la_LIBADD = \
	$(SUBTITLEEDITOR_LIBS) \
	$(GSTREAMER_LIBS) \
	-L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = waveformmanagement.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
