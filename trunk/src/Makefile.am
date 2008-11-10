bin_PROGRAMS = subtitleeditor

lib_LTLIBRARIES = libsubtitleeditor.la

AM_CFLAGS = -DDATA_DIR=\""$(datadir)"\" \
              -DLOCALE_DIR=\""$(datadir)/locale"\" \
              -DPIXMAPS_DIR=\""$(datadir)/pixmaps"\" 

INCLUDES = \
					 $(GTKMM_CFLAGS) $(LIBGLADEMM_CFLAGS) \
					 $(GSTREAMER_CFLAGS) $(LIBXML_CFLAGS) $(GTKGLEXT_CFLAGS) $(GL_CFLAGS) \
					 -I$(top_srcdir)/include \
					 -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
					 -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

FILES = \
				debug.h \
				debug.cc \
				utility.h \
				utility.cc \
				Config.h \
				Config.cc \
				Signal.h \
				Signal.cc \
				Color.h \
				Color.cc \
				SubtitleTime.h \
				SubtitleTime.cc \
				DocumentSystem.h \
				DocumentSystem.cc \
				Encodings.h \
				Encodings.cc \
				ScriptInfo.h \
				ScriptInfo.cc \
				StyleModel.h \
				StyleModel.cc \
				Error.h \
				SubtitleModel.h \
				SubtitleModel.cc \
				SubtitleView.h \
				SubtitleView.cc \
				SubtitleTags.h \
				SubtitleTags.cc \
				Waveform.h \
				Waveform.cc \
				Options.h \
				Options.cc \
				Default.h \
				Style.h \
				Style.cc \
				Styles.h \
				Styles.cc \
				Command.h \
				Command.cc \
				CommandSystem.h \
				CommandSystem.cc \
				Player.h \
				vp/GStreamerPlayer.h \
				vp/GStreamerPlayer.cc \
				vp/VideoPlayer.h \
				vp/VideoPlayer.cc \
				we/WaveformRenderer.h \
				we/WaveformRenderer.cc \
				we/WaveformRendererCairo.cc \
				we/WaveformRendererGL.cc \
				we/WaveformEditor.h \
				we/WaveformEditor.cc \
				we/WaveformGenerator.h \
				we/WaveformGenerator.cc \
				TimeUtility.h \
				TimeUtility.cc \
				FileReader.h \
				FileReader.cc \
				FileWriter.h \
				FileWriter.cc \
				SubtitleFormat.h \
				SubtitleFormat.cc \
				SubtitleFormatFactory.h \
				SubtitleFormatSystem.h \
				SubtitleFormatSystem.cc \
				Extension.h \
				Extension.cc \
				ExtensionInfo.h \
				ExtensionInfo.cc \
				ExtensionManager.h \
				ExtensionManager.cc \
				extension/Action.h \
				extension/Action.cc


FORMATS_FILES = \
				formats/AdobeEncoreDVD.h \
				formats/AdvancedSubStationAlpha.h \
				formats/MicroDVD.h \
				formats/MPL2.h \
				formats/MPsub.h \
				formats/SubRip.h \
				formats/SubStationAlpha.h \
				formats/SubtitleEditorProject.h \
				formats/SubViewer2.h \
				formats/TimedTextAuthoringFormat1.h

GUI_FILES = \
				gui/DialogUtility.h \
				gui/DialogUtility.cc \
				gui/DialogFileChooser.h \
				gui/DialogFileChooser.cc \
				gui/DialogCharacterCodings.h \
				gui/DialogCharacterCodings.cc \
				gui/ComboBoxEncoding.h \
				gui/ComboBoxEncoding.cc \
				gui/ComboBoxSubtitleFormat.h \
				gui/ComboBoxSubtitleFormat.cc \
				gui/ComboBoxNewLine.h \
				gui/ComboBoxNewLine.cc \
				gui/ComboBoxVideo.h \
				gui/ComboBoxVideo.cc \
				gui/SpinButtonTime.h \
				gui/SpinButtonTime.cc \
				gui/TreeViewExtensionManager.h \
				gui/TreeViewExtensionManager.cc


if	 ENABLE_UNITTEST

UNITTEST_FILES = \
				 unittest/unittest.cc \
				 unittest/testSubtitleTime.cc \
				 unittest/testColor.cc

EXTRA_DIST = \
				unittest/adobe.encore.dvd.ntsc.format \
				unittest/adobe.encore.dvd.ntsc.format.euc-jp \
				unittest/adobe.encore.dvd.pal.format \
				unittest/adobe.encore.dvd.pal.format.euc-jp \
				unittest/ass.format \
				unittest/ass.format.euc-jp \
				unittest/microdvd.format \
				unittest/microdvd.format.euc-jp \
				unittest/mpl2.format \
				unittest/mpl2.format.euc-jp \
				unittest/mpsub.format \
				unittest/mpsub.format.euc-jp \
				unittest/ssa.format \
				unittest/ssa.format.euc-jp \
				unittest/subrip.format \
				unittest/subrip.format.euc-jp \
				unittest/subtitle.editor.project.format \
				unittest/subtitle.editor.project.format.euc-jp \
				unittest/subviewer2.0.format \
				unittest/subviewer2.0.format.euc-jp \
				unittest/transcript

endif


## lib

LIB_FILES = \
					SubtitleEditorWindow.h \
					SubtitleEditorWindow.cc \
					Document.h \
					Document.cc \
					Subtitle.h \
					Subtitle.cc \
					Subtitles.h \
					Subtitles.cc

## TODO fixme
APPLICATION_FILES = \
				main.cc \
				gui/Application.h \
				gui/Application.cc \
				gui/MenuBar.h \
				gui/MenuBar.cc \
				gui/Statusbar.h \
				gui/Statusbar.cc

## libsubtitleeditor
libsubtitleeditor_la_LDFLAGS = -export-dynamic -no-undefined

libsubtitleeditor_la_SOURCES = $(FILES) $(LIB_FILES) $(FORMATS_FILES) $(GUI_FILES) 


## subttileeditor
subtitleeditor_SOURCES = $(APPLICATION_FILES) $(UNITTEST_FILES)

subtitleeditor_LDADD = $(GTKMM_LIBS) $(LIBGLADEMM_LIBS) $(GSTREAMER_LIBS) \
											 $(INTLLIBS) $(LIBXML_LIBS) \
											 $(GTKGLEXT_LIBS) $(GL_LIBS) $(CPPUNIT_LIBS) \
											 libsubtitleeditor.la


## clean
CLEANFILES = Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak

