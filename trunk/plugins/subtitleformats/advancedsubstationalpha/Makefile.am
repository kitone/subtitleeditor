pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libadvancedsubstationalpha.la

libadvancedsubstationalpha_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

libadvancedsubstationalpha_la_SOURCES = \
	advancedsubstationalpha.cc

libadvancedsubstationalpha_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libadvancedsubstationalpha_la_LIBADD = $(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = advancedsubstationalpha.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
