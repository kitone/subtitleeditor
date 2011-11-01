pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libdcsubtitle.la

libdcsubtitle_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	$(LIBXML_CFLAGS)

libdcsubtitle_la_SOURCES = \
	dcsubtitle.cc

libdcsubtitle_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdcsubtitle_la_LIBADD = \
	$(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor \
	$(LIBXML_LIBS)

plugindescription_in_files = dcsubtitle.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
