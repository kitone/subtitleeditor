
AM_CFLAGS = -DDATA_DIR=\""$(datadir)"\" \
              -DLOCALE_DIR=\""$(datadir)/locale"\" \
              -DPIXMAPS_DIR=\""$(datadir)/pixmaps"\" 

INCLUDES = \
					 $(GTKMM_CFLAGS) $(LIBGLADEMM_CFLAGS) \
					 $(GSTREAMER_CFLAGS) $(LIBXML_CFLAGS) $(ENCHANT_CFLAGS) $(LIBPCRE_CFLAGS) \
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
				RegEx.h \
				RegEx.cc \
				ScriptInfo.h \
				ScriptInfo.cc \
				StyleModel.h \
				StyleModel.cc \
				SubtitleFormat.h \
				SubtitleFormat.cc \
				SubtitleModel.h \
				SubtitleModel.cc \
				SubtitleSystem.h \
				SubtitleSystem.cc \
				SubtitleView.h \
				SubtitleView.cc \
				Action.h \
				Action.cc \
				ActionSystem.h \
				ActionSystem.cc \
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
				PluginSystem.h \
				PluginSystem.cc


FORMATS_FILES = \
				formats/SubtitleEditorProject.h \
				formats/SubtitleEditorProject.cc \
				formats/SubtitleASS.cc \
				formats/SubtitleASS.h \
				formats/SubtitleMicroDVD.cc \
				formats/SubtitleMicroDVD.h \
				formats/SubtitleMPL2.cc \
				formats/SubtitleMPL2.h \
				formats/SubtitleMPsub.cc \
				formats/SubtitleMPsub.h \
				formats/SubtitleSSA.cc \
				formats/SubtitleSSA.h \
				formats/SubtitleSubRip.cc \
				formats/SubtitleSubRip.h \
				formats/SubtitleSubViewer2.cc \
				formats/SubtitleSubViewer2.h \
				formats/SubtitleText.cc \
				formats/SubtitleText.h \
				formats/SubtitleTTXT.cc \
				formats/SubtitleTTXT.h \
				formats/SubtitleEncoreNTSC.h \
				formats/SubtitleEncoreNTSC.cc \
				formats/SubtitleEncorePAL.h \
				formats/SubtitleEncorePAL.cc

ACTIONS = \
				actions/DocumentManagement.cc \
				actions/UndoRedo.cc \
				actions/AdjustTime.cc \
				actions/ApplyTranslation.cc \
				actions/DuplicateSelectedSubtitles.cc \
				actions/ExtendLength.cc \
				actions/TryToExtendToPerfect.cc \
				actions/MoveSubtitles.cc \
				actions/MoveAfterPrecedingSubtitle.cc \
				actions/DeleteSubtitleSelect.cc \
				actions/InsertSubtitle.cc \
				actions/Selection.cc \
				actions/CombineSubtitles.cc \
				actions/SplitSubtitle.cc \
				actions/Italicize.cc \
				actions/ScaleSubtitles.cc \
				actions/ChangeFramerate.cc \
				actions/Dialoguize.cc \
				actions/SpellChecking.h \
				actions/SpellChecking.cc \
				actions/ReverseTextAndTranslation.cc \
				actions/SplitDocument.cc \
				actions/JoinDocument.cc \
				actions/StyleEditor.h \
				actions/StyleEditor.cc \
				actions/FindAndReplace.cc \
				actions/Transcript.cc \
				actions/ExternalVideoPlayer.cc \
				actions/About.cc \
				actions/ViewManager.cc


ACTIONS_FILES = $(ACTIONS) \
				actions/CheckErrors.h \
				actions/CheckErrors.cc

GUI_FILES = \
				gui/DialogUtility.h \
				gui/DialogUtility.cc \
				gui/DialogFileChooser.h \
				gui/DialogFileChooser.cc \
				gui/Application.h \
				gui/Application.cc \
				gui/MenuBar.h \
				gui/MenuBar.cc \
				gui/Statusbar.h \
				gui/Statusbar.cc \
				gui/CheckErrorsUI.h \
				gui/CheckErrorsUI.cc \
				gui/VideoPlayer.h \
				gui/VideoPlayer.cc \
				gui/PreferencesUI.h \
				gui/PreferencesUI.cc \
				gui/WaveformGeneratorUI.h \
				gui/WaveformGeneratorUI.cc \
				gui/Player.h \
				gui/Player.cc \
				gui/MPlayer.h \
				gui/MPlayer.cc \
				gui/GStreamerPlayer.h \
				gui/GStreamerPlayer.cc


if	 ENABLE_UNITTEST

UNITTEST_FILES = \
				 unittest/unittest.cc \
				 unittest/testSubtitleTime.cc \
				 unittest/testRegEx.cc \
				 unittest/testSubtitleLoaders.cc \
				 unittest/testColor.cc

EXTRA_DIST = \
					unittest/ass.format \
					unittest/microdvd.format \
					unittest/mpl2.format \
					unittest/ssa.format \
					unittest/subrip.format \
					unittest/subviewer2.0.format

endif


## lib

LIB_FILES = \
					SubtitleEditorWindow.h \
					SubtitleEditorWindow.cc \
					Plugin.h \
					Plugin.cc \
					Document.h \
					Document.cc \
					Subtitle.h \
					Subtitle.cc \
					Subtitles.h \
					Subtitles.cc

#noinst_LTLIBRARIES = libsubtitleeditor.la

#libsubtitleeditor_la_LDFAGS = #-export-dynamic -no-undefined -export-symbols-regex "^[[^_]].*"

#libsubtitleeditor_la_LIBADD = ##$(SUBTITLEEDITOR_LIBS)

#libsubtitleeditor_la_SOURCES = $(LIB_FILES)

## subtitleeditor
bin_PROGRAMS = subtitleeditor

subtitleeditor_SOURCES = main.cc $(FILES) $(LIB_FILES) $(FORMATS_FILES) $(GUI_FILES) $(ACTIONS_FILES) $(UNITTEST_FILES)


subtitleeditor_LDADD = $(GTKMM_LIBS) $(LIBGLADEMM_LIBS) $(GSTREAMER_LIBS) \
											 $(INTLLIBS) $(LIBXML_LIBS) $(ENCHANT_LIBS) $(LIBPCRE_LIBS) -lpcrecpp \
											 $(CPPUNIT_LIBS) ##libsubtitleeditor.la



##

CLEANFILES = Makefile.am~ *.cc~ *.h~ *.glade~ *.gladep *.gladep.bak

