pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions

INCLUDES = \
	 -I$(top_srcdir) \
	 -I$(top_srcdir)/src \
	 $(SUBTITLEEDITOR_CFLAGS)

pluginlib_LTLIBRARIES = \
	libadjusttime.la

libadjusttime_la_SOURCES = \
	AdjustTime.cc

libadjusttime_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libadjusttime_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = adjusttime.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
