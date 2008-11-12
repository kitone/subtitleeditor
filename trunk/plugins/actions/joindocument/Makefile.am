actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libjoindocument.la

libjoindocument_la_SOURCES = \
	JoinDocument.cc

libjoindocument_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libjoindocument_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = joindocument.se-plugin.desktop.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.desktop.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
