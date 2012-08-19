pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions
actionsharedir = $(PACKAGE_PLUGIN_SHARE_DIR)/actions

INCLUDES = \
	 -I$(top_srcdir) \
	 -I$(top_srcdir)/src \
	 $(SUBTITLEEDITOR_CFLAGS) \
	 $(GSTREAMER_CFLAGS)

pluginlib_LTLIBRARIES = \
	libkeyframesmanagement.la

libkeyframesmanagement_la_SOURCES = \
	keyframesgenerator.cc \
	keyframesgeneratorusingframe.cc \
	keyframesmanagement.cc \
	mediadecoder.h

libkeyframesmanagement_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libkeyframesmanagement_la_LIBADD = \
	$(SUBTITLEEDITOR_LIBS) \
	$(GSTREAMER_LIBS) \
	-L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = keyframesmanagement.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
