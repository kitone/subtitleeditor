pluginlibdir = $(PACKAGE_PLUGIN_LIB_DIR)/subtitleformats
plugindescriptiondir = $(PACKAGE_PLUGIN_DESCRIPTION_DIR)/subtitleformats

pluginlib_LTLIBRARIES = \
	libadobeencoredvdntsc.la \
	libadobeencoredvdpal.la

## ntsc
libadobeencoredvdntsc_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

libadobeencoredvdntsc_la_SOURCES = \
	adobeencoredvd.h \
	adobeencoredvdntsc.cc

libadobeencoredvdntsc_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libadobeencoredvdntsc_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

## pal
libadobeencoredvdpal_la_CXXFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(SUBTITLEEDITOR_CFLAGS)

libadobeencoredvdpal_la_SOURCES = \
	adobeencoredvd.h \
	adobeencoredvdpal.cc

libadobeencoredvdpal_la_LDFLAGS = $(PLUGIN_LIBTOOL_FLAGS)
libadobeencoredvdpal_la_LIBADD = $(SUBTITLEEDITOR_LIBS)

plugindescription_in_files = \
	adobeencoredvdntsc.se-plugin.in \
	adobeencoredvdpal.se-plugin.in

plugindescription_DATA = $(plugindescription_in_files:.se-plugin.in=.se-plugin)

@INTLTOOL_SE_PLUGIN_RULE@

EXTRA_DIST = $(plugindescription_in_files)

CLEANFILES = $(plugindescription_DATA) Makefile.am~ *.cc~ *.h~ *.in~
