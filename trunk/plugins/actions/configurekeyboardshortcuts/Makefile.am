plugindir = $(libdir)/subtitleeditor/plugins/actions

INCLUDES = \
					 -I$(top_srcdir) \
					 -I$(top_srcdir)/src \
					 $(SUBTITLEEDITOR_CFLAGS)

plugin_LTLIBRARIES = libconfigurekeyboardshortcuts.la

libconfigurekeyboardshortcuts_la_SOURCES = \
	ConfigureKeyboardShortcuts.cc

libconfigurekeyboardshortcuts_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libconfigurekeyboardshortcuts_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugin_in_files = configurekeyboardshortcuts.se-plugin.desktop.in

%.se-plugin: %.se-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.se-plugin.desktop.in=.se-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

