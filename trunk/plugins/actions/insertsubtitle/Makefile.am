actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libinsertsubtitle.la

libinsertsubtitle_la_SOURCES = \
	InsertSubtitle.cc

libinsertsubtitle_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libinsertsubtitle_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = insertsubtitle.se-plugin.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
