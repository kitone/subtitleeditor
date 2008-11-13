actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libsplitsubtitle.la

libsplitsubtitle_la_SOURCES = \
	SplitSubtitle.cc

libsplitsubtitle_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libsplitsubtitle_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = splitsubtitle.se-plugin.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
