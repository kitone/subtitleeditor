plugin_name = avidds
pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libavidds.la

libavidds_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

libavidds_la_SOURCES = \
	avidds.cc

libavidds_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libavidds_la_LIBADD = $(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = avidds.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
