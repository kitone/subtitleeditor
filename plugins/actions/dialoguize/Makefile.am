actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libdialoguize.la

libdialoguize_la_SOURCES = \
	Dialoguize.cc

libdialoguize_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libdialoguize_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = dialoguize.se-plugin.desktop.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.desktop.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
