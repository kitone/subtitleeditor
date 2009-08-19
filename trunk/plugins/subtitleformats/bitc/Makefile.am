plugin_name = bitc
pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats
subtitleformatplugindevdir = $(abs_srcdir)
uidir = $(PACKAGE_PLUGIN_SHARE_DIR)/$(plugin_name)

pluginlib_LTLIBRARIES = \
	libbitc.la

libbitc_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS) \
	-DSE_PLUGIN_PATH_DEV=\"$(subtitleformatplugindevdir)\" \
	-DSE_PLUGIN_PATH_UI=\"$(uidir)\"

libbitc_la_SOURCES = \
	bitc.cc

libbitc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libbitc_la_LIBADD = $(SUBTITLEEDITOR_LIBS) -L$(top_srcdir)/src -lsubtitleeditor

plugindescription_in_files = bitc.se-plugin.in
plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

ui_DATA = dialog-bitc.ui

EXTRA_DIST = $(plugindescription_in_files) $(ui_DATA)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
