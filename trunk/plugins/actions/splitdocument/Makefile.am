plugin_name = splitdocument
pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions
actionplugindevdir = plugins/actions/$(plugin_name)
gladedir = $(PACKAGE_PLUGIN_SHARE_DIR)/$(plugin_name)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	-DSE_PLUGIN_PATH_DEV=\"$(actionplugindevdir)\" \
	-DSE_PLUGIN_PATH_GLADE=\"$(gladedir)\"

pluginlib_LTLIBRARIES = \
	libsplitdocument.la

libsplitdocument_la_SOURCES = \
	SplitDocument.cc

libsplitdocument_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libsplitdocument_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = splitdocument.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

glade_DATA = dialog-split-document.glade

EXTRA_DIST = $(plugindescription_in_files) $(glade_DATA)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak *.in~
