actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libtimemodemanagement.la

libtimemodemanagement_la_SOURCES = \
	TimeModeManagement.cc

libtimemodemanagement_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtimemodemanagement_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = timemodemanagement.se-plugin.desktop.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.desktop.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
