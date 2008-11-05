plugindir = $(libdir)/subtitleeditor/plugins/actions

INCLUDES = \
					 -I$(top_srcdir) \
					 -I$(top_srcdir)/src \
					 $(SUBTITLEEDITOR_CFLAGS)

plugin_LTLIBRARIES = libpreferences.la

libpreferences_la_SOURCES = \
	ExtensionPage.h \
	DocumentPage.h \
	InterfacePage.h \
	PreferencePage.h  \
	PreviewPage.h  \
	TimingPage.h  \
	VideoPlayerPage.h  \
	WaveformPage.h \
	PreferencesPlugin.cc

libpreferences_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libpreferences_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugin_in_files = preferences.se-plugin.desktop.in

%.se-plugin: %.se-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.se-plugin.desktop.in=.se-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

