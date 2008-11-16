plugin_name = errorchecking
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
	liberrorchecking.la

liberrorchecking_la_SOURCES = \
	errorchecking.h \
	errorcheckingpreferences.h \
	errorcheckingplugin.cc \
	maxcharactersperline.h \
	maxlinepersubtitle.h \
	mingapbetweensubtitles.h \
	overlapping.h \
	mindisplaytime.h \
	toolongdisplaytime.h \
	tooshortdisplaytime.h

liberrorchecking_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
liberrorchecking_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = errorchecking.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

glade_DATA = \
	dialog-error-checking.glade \
	dialog-error-checking-preferences.glade

EXTRA_DIST = $(plugindescription_in_files) $(glade_DATA)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak *.in~
