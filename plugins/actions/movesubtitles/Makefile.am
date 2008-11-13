se_plugin_name = movesubtitles
actionplugindir = $(PACKAGE_PLUGIN_DIR)/actions
actionplugindevdir = plugins/actions/$(se_plugin_name)
gladedir = $(PACKAGE_SHARE_DIR)/plugins/$(se_plugin_name)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	-DSE_PLUGIN_PATH_DEV=\"$(actionplugindevdir)\" \
	-DSE_PLUGIN_PATH_GLADE=\"$(gladedir)\"

actionplugin_LTLIBRARIES = \
	libmovesubtitles.la

libmovesubtitles_la_SOURCES = \
	MoveSubtitles.cc

libmovesubtitles_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libmovesubtitles_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

actionplugin_in_files = movesubtitles.se-plugin.in
actionplugin_DATA = $(actionplugin_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

glade_DATA = dialog-move-subtitles.glade

EXTRA_DIST = $(actionplugin_in_files) $(glade_DATA)

CLEANFILES = $(actionplugin_DATA) Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak *.in~
