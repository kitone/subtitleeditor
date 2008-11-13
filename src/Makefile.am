bin_PROGRAMS = subtitleeditor

lib_LTLIBRARIES = libsubtitleeditor.la

AM_CFLAGS = -DDATA_DIR=\""$(datadir)"\" \
              -DLOCALE_DIR=\""$(datadir)/locale"\" \
              -DPIXMAPS_DIR=\""$(datadir)/pixmaps"\" 

INCLUDES = \
					 $(GTKMM_CFLAGS) $(LIBGLADEMM_CFLAGS) \
					 $(GSTREAMER_CFLAGS) $(LIBXML_CFLAGS) $(GTKGLEXT_CFLAGS) $(GL_CFLAGS) \
					 -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
					 -DPACKAGE_SHARE_DIR=\""$(PACKAGE_SHARE_DIR)"\" \
					 -DPACKAGE_SHARE_DIR_DEV=\""share"\" \
					 -DPACKAGE_GLADE_DIR=\""$(PACKAGE_SHARE_DIR)/glade"\" \
					 -DPACKAGE_GLADE_DIR_DEV=\""share/glade"\" \
					 -DPACKAGE_PLUGIN_DIR=\""$(PACKAGE_PLUGIN_DIR)"\" \
					 -DPACKAGE_PLUGIN_DIR_DEV=\""plugins"\"

FILES = \
				debug.h \
				debug.cc \
				utility.h \
				utility.cc \
				gstreamer_utility.h \
				gstreamer_utility.cc \
				gtkmm_utility.h \
				gtkmm_utility.cc \
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
subtitleeditor_SOURCES = $(APPLICATION_FILES)

subtitleeditor_LDADD = $(GTKMM_LIBS) $(LIBGLADEMM_LIBS) $(GSTREAMER_LIBS) \
											 $(INTLLIBS) $(LIBXML_LIBS) \
											 $(GTKGLEXT_LIBS) $(GL_LIBS) \
											 libsubtitleeditor.la


## clean
CLEANFILES = Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak

