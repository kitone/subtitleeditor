pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libtimedtextauthoringformat1.la

libtimedtextauthoringformat1_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	$(LIBXML_CFLAGS)

libtimedtextauthoringformat1_la_SOURCES = \
	timedtextauthoringformat1.cc

libtimedtextauthoringformat1_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtimedtextauthoringformat1_la_LIBADD = \
	$(SUBTITLEEDITOR_LIBS) \
	$(LIBXML_LIBS)

plugindescription_in_files = timedtextauthoringformat1.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
