pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libsubtitleeditorproject.la

libsubtitleeditorproject_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	$(LIBXML_CFLAGS)

libsubtitleeditorproject_la_SOURCES = \
	subtitleeditorproject.cc

libsubtitleeditorproject_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libsubtitleeditorproject_la_LIBADD = \
	$(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor \
	$(LIBXML_LIBS)

plugindescription_in_files = subtitleeditorproject.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
