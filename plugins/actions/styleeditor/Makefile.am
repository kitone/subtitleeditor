SE_PLUGIN_NAME = styleeditor

SE_PLUGIN_PATH_DEV = plugins/actions/$(SE_PLUGIN_NAME)
SE_PLUGIN_PATH_LIB = $(PACKAGE_PLUGIN_DIR)/actions/
SE_PLUGIN_PATH_GLADE = $(PACKAGE_SHARE_DIR)/plugins/$(SE_PLUGIN_NAME)

INCLUDES = \
					 -I$(top_srcdir) \
					 -I$(top_srcdir)/src \
					 $(SUBTITLEEDITOR_CFLAGS) \
					 -DSE_PLUGIN_PATH_DEV=\"$(SE_PLUGIN_PATH_DEV)\" \
					 -DSE_PLUGIN_PATH_GLADE=\"$(SE_PLUGIN_PATH_GLADE)\"

plugindir = $(SE_PLUGIN_PATH_LIB)

plugin_LTLIBRARIES = libstyleeditor.la

libstyleeditor_la_SOURCES = \
	StyleEditor.h \
	StyleEditor.cc

libstyleeditor_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libstyleeditor_la_LIBADD = $(SUBTITLEEDITOR_LIBS)


gladedir = $(SE_PLUGIN_PATH_GLADE)
glade_DATA = dialog-style-editor.glade


plugin_in_files = styleeditor.se-plugin.desktop.in

%.se-plugin: %.se-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.se-plugin.desktop.in=.se-plugin)


EXTRA_DIST = $(glade_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

