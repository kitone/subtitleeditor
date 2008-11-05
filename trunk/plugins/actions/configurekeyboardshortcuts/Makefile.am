SE_PLUGIN_NAME = configurekeyboardshortcuts

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

plugin_LTLIBRARIES = libconfigurekeyboardshortcuts.la

libconfigurekeyboardshortcuts_la_SOURCES = \
	ConfigureKeyboardShortcuts.cc

libconfigurekeyboardshortcuts_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libconfigurekeyboardshortcuts_la_LIBADD = $(SUBTITLEEDITOR_LIBS)


gladedir = $(SE_PLUGIN_PATH_GLADE)
glade_DATA = dialog-configure-keyboard-shortcuts.glade

plugin_in_files = configurekeyboardshortcuts.se-plugin.desktop.in

%.se-plugin: %.se-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.se-plugin.desktop.in=.se-plugin)

EXTRA_DIST = $(glade_DATA) $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

