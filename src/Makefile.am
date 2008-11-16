PACKAGE_DIRECTORY = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SHARE_DIR=\""$(PACKAGE_SHARE_DIR)"\" \
	-DPACKAGE_SHARE_DIR_DEV=\""share"\" \
	-DPACKAGE_GLADE_DIR=\""$(PACKAGE_SHARE_DIR)/glade"\" \
	-DPACKAGE_GLADE_DIR_DEV=\""share/glade"\" \
	-DPACKAGE_PLUGIN_LIB_DIR=\""$(PACKAGE_PLUGIN_LIB_DIR)"\" \
	-DPACKAGE_PLUGIN_DESCRIPTION_DIR=\""$(PACKAGE_PLUGIN_DESCRIPTION_DIR)"\" \
	-DPACKAGE_PLUGIN_DIR_DEV=\""plugins"\"


## libsubtitleeditor
LIB_SUBTITLEEDITOR_FILES = \
	Color.cc \
	Color.h \
	Command.cc \
	Command.h \
	CommandSystem.cc \
	CommandSystem.h \
	cfg.cc \
	cfg.h \
	debug.cc \
	debug.h \
	Document.cc \
	Document.h \
	DocumentSystem.cc \
	DocumentSystem.h \
	Encodings.cc \
	Encodings.h \
	Error.h \
	extension/Action.cc \
	extension/Action.h \
	extension/SubtitleFormat.h \
	Extension.cc \
	Extension.h \
	ExtensionInfo.cc \
	ExtensionInfo.h \
	ExtensionManager.cc \
	ExtensionManager.h \
	FileReader.cc \
	FileReader.h \
	FileWriter.cc \
	FileWriter.h \
	gtkmm_utility.cc \
	gtkmm_utility.h \
	i18n.h \
	Player.h \
	ScriptInfo.cc \
	ScriptInfo.h \
	Style.cc \
	Style.h \
	StyleModel.cc \
	StyleModel.h \
	Styles.cc \
	Styles.h \
	Subtitle.cc \
	SubtitleEditorWindow.cc \
	SubtitleEditorWindow.h \
	SubtitleFormatIO.cc \
	SubtitleFormatIO.h \
	SubtitleFormatSystem.cc \
	SubtitleFormatSystem.h \
	Subtitle.h \
	SubtitleModel.cc \
	SubtitleModel.h \
	Subtitles.cc \
	Subtitles.h \
	SubtitleTags.cc \
	SubtitleTags.h \
	SubtitleTime.cc \
	SubtitleTime.h \
	SubtitleView.cc \
	SubtitleView.h \
	TimeUtility.cc \
	TimeUtility.h \
	utility.cc \
	utility.h \
	Waveform.cc \
	Waveform.h \
	WaveformManager.h \
	widget_config_utility.cc \
	widget_config_utility.h

LIB_GUI_FILES = \
	gui/ComboBoxEncoding.cc \
	gui/ComboBoxEncoding.h \
	gui/ComboBoxNewLine.cc \
	gui/ComboBoxNewLine.h \
	gui/ComboBoxSubtitleFormat.cc \
	gui/ComboBoxSubtitleFormat.h \
	gui/ComboBoxVideo.cc \
	gui/ComboBoxVideo.h \
	gui/DialogCharacterCodings.cc \
	gui/DialogCharacterCodings.h \
	gui/DialogFileChooser.cc \
	gui/DialogFileChooser.h \
	gui/DialogUtility.cc \
	gui/DialogUtility.h \
	gui/SpinButtonTime.cc \
	gui/SpinButtonTime.h \
	gui/TreeViewExtensionManager.cc \
	gui/TreeViewExtensionManager.h

lib_LTLIBRARIES = libsubtitleeditor.la

libsubtitleeditor_la_LDFLAGS = -export-dynamic -no-undefined

libsubtitleeditor_la_SOURCES = \
	$(LIB_SUBTITLEEDITOR_FILES) \
	$(LIB_GUI_FILES)

libsubtitleeditor_la_CXXFLAGS = \
	$(GTKMM_CFLAGS) \
	$(LIBGLADEMM_CFLAGS) \
	$(PACKAGE_DIRECTORY)


## subtitleeditor
APPLICATION_FILES = \
	gui/Application.cc \
	gui/Application.h \
	gui/MenuBar.cc \
	gui/MenuBar.h \
	gui/Statusbar.cc \
	gui/Statusbar.h \
	gstreamer_utility.cc \
	gstreamer_utility.h \
	main.cc \
	Options.cc \
	Options.h \
	vp/GStreamerPlayer.cc \
	vp/GStreamerPlayer.h \
	vp/VideoPlayer.cc \
	vp/VideoPlayer.h \
	we/WaveformEditor.cc \
	we/WaveformEditor.h \
	we/WaveformGenerator.cc \
	we/WaveformGenerator.h \
	we/WaveformRendererCairo.cc \
	we/WaveformRenderer.cc \
	we/WaveformRendererGL.cc \
	we/WaveformRenderer.h

bin_PROGRAMS = subtitleeditor

subtitleeditor_SOURCES = \
	$(APPLICATION_FILES)

subtitleeditor_LDADD = \
	$(GTKMM_LIBS) \
	$(LIBGLADEMM_LIBS) \
	$(GSTREAMER_LIBS) \
	$(GTKGLEXT_LIBS) \
	$(GL_LIBS) \
	$(LIBXML_LIBS) \
	libsubtitleeditor.la

subtitleeditor_CXXFLAGS = \
	$(GTKMM_CFLAGS) \
	$(LIBGLADEMM_CFLAGS) \
	$(GSTREAMER_CFLAGS) \
	$(LIBXML_CFLAGS) \
	$(GTKGLEXT_CFLAGS) \
	$(GL_CFLAGS) \
	$(PACKAGE_DIRECTORY)


CLEANFILES = Makefile.am~ *.cc~ *.h~ *.in~
