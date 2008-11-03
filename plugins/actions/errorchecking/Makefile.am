plugindir = $(libdir)/subtitleeditor/plugins/actions

INCLUDES = \
					 -I$(top_srcdir) \
					 -I$(top_srcdir)/src \
					 $(SUBTITLEEDITOR_CFLAGS)

plugin_LTLIBRARIES = liberrorchecking.la

liberrorchecking_la_SOURCES = \
				ErrorChecking.h \
				ErrorCheckingPreferences.h \
				ErrorCheckingPlugin.cc \
				MaxCharactersPerLine.h \
				MaxLinePerSubtitle.h \
				MinGapBetweenSubtitles.h \
				Overlapping.h \
				MinDisplayTime.h \
				TooLongDisplayTime.h \
				TooShortDisplayTime.h

liberrorchecking_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
liberrorchecking_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugin_in_files = errorchecking.se-plugin.desktop.in

%.se-plugin: %.se-plugin.desktop.in $(INTLTOOL_MERGE) $(wildcard $(top_srcdir)/po/*po) ; $(INTLTOOL_MERGE) $(top_srcdir)/po $< $@ -d -u -c $(top_builddir)/po/.intltool-merge-cache

plugin_DATA = $(plugin_in_files:.se-plugin.desktop.in=.se-plugin)

EXTRA_DIST = $(plugin_in_files)

CLEANFILES = $(plugin_DATA)
DISTCLEANFILES = $(plugin_DATA)

