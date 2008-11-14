pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions

INCLUDES = \
	 -I$(top_srcdir) \
	 -I$(top_srcdir)/src \
	 $(SUBTITLEEDITOR_CFLAGS)

pluginlib_LTLIBRARIES = \
	libapplytranslation.la

libapplytranslation_la_SOURCES = \
	ApplyTranslation.cc

libapplytranslation_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libapplytranslation_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = applytranslation.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
