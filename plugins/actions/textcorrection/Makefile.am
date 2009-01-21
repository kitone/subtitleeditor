plugin_name = textcorrection
pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions
patterndir = $(PACKAGE_PLUGIN_SHARE_DIR)/$(plugin_name)
actionplugindevdir = $(abs_srcdir)
gladedir = $(PACKAGE_PLUGIN_SHARE_DIR)/$(plugin_name)

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	$(LIBXML_CFLAGS) \
	-DSE_PLUGIN_PATH_DEV=\"$(actionplugindevdir)\" \
	-DSE_PLUGIN_PATH_GLADE=\"$(gladedir)\" \
	-DSE_PLUGIN_PATH_PATTERN=\"$(patterndir)\"

pluginlib_LTLIBRARIES = \
	libtextcorrection.la

libtextcorrection_la_SOURCES = \
	commonerrorpage.h \
	confirmationpage.h \
	hearingimpairedpage.h \
	page.h \
	pattern.cc \
	pattern.h \
	patternmanager.cc \
	patternmanager.h \
	patternspage.h \
	taskspage.h \
	textcorrection.cc

libtextcorrection_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libtextcorrection_la_LIBADD = $(SUBTITLEEDITOR_LIBS) $(LIBXML_LIBS) -L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = textcorrection.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

pattern_in_files = \
	Latn.common-error.se-pattern.in \
	Latn-en.common-error.se-pattern.in \
	Latn-en.hearing-impaired.se-pattern.in \
	Latn-en-US.common-error.se-pattern.in \
	Latn.hearing-impaired.se-pattern.in \
	Zyyy.common-error.se-pattern.in

pattern_DATA = $(pattern_in_files:.se-pattern.in=.se-pattern)
@INTLTOOL_SE_PATTERN_RULE@

glade_DATA = assistant-text-correction.glade

EXTRA_DIST = $(plugindescription_in_files) $(pattern_in_files) $(glade_DATA)

CLEANFILES = $(plugindescription_DATA) $(pattern_DATA) Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak *.in~
