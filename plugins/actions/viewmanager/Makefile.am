plugin_name = viewmanager
pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions
actionplugindevdir = $(abs_srcdir)
gladedir = $(PACKAGE_PLUGIN_SHARE_DIR)/$(plugin_name)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	-DSE_PLUGIN_PATH_DEV=\"$(actionplugindevdir)\" \
	-DSE_PLUGIN_PATH_GLADE=\"$(gladedir)\"

pluginlib_LTLIBRARIES = \
	libviewmanager.la

libviewmanager_la_SOURCES = \
	viewmanager.cc

libviewmanager_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libviewmanager_la_LIBADD = $(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = viewmanager.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

glade_DATA = dialog-view-manager.glade

EXTRA_DIST = $(plugindescription_in_files) $(glade_DATA)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak *.in~
