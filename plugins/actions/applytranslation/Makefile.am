actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions

INCLUDES = \
	 -I$(top_srcdir) \
	 -I$(top_srcdir)/src \
	 $(SUBTITLEEDITOR_CFLAGS)

actionplugin_LTLIBRARIES = \
	libapplytranslation.la

libapplytranslation_la_SOURCES = \
	ApplyTranslation.cc

libapplytranslation_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libapplytranslation_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = applytranslation.se-plugin.desktop.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.desktop.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(actionplugin_in_files)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.in~
