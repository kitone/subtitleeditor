pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/actions
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/actions

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

pluginlib_LTLIBRARIES = \
	libjoindocument.la

libjoindocument_la_SOURCES = \
	JoinDocument.cc

libjoindocument_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libjoindocument_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = joindocument.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
