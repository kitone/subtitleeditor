/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2008, kitone
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "PreferencesUI.h"
#include "Config.h"
#include "utility.h"

/*
 *	Interface
 */
class PreferenceInterfaceUI : public Gtk::VBox
{
public:
	PreferenceInterfaceUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		refGlade->get_widget("check-use-dynamic-keyboard-shortcuts", m_checkUseDynamicKeyboardShortcuts);
		refGlade->get_widget("check-maximize-window", m_checkMaximizeWindow);
		refGlade->get_widget("check-ask-to-save-on-exit", m_checkAskToSaveOnExit);

		refGlade->get_widget("check-center-subtitle", m_checkCenterSubtitle);
		refGlade->get_widget("check-show-character-per-line", m_checkShowCharacterPerLine);
		refGlade->get_widget("check-enable-rubberband-selection", m_checkEnableRubberbandSelection);
		refGlade->get_widget("check-used-ctrl-enter-to-confirm-change", m_checkUsedCtrlEnterToConfirmChange);

		refGlade->get_widget("check-create-backup-copy", m_checkCreateBackupCopy);
		refGlade->get_widget("check-autosave", m_checkAutosave);
		refGlade->get_widget("spin-autosave", m_spinAutosave);

		WidgetToConfig::read_config_and_connect(m_checkUseDynamicKeyboardShortcuts, "interface", "use-dynamic-keyboard-shortcuts");
		WidgetToConfig::read_config_and_connect(m_checkMaximizeWindow, "interface", "maximize-window");
		WidgetToConfig::read_config_and_connect(m_checkAskToSaveOnExit, "interface", "ask-to-save-on-exit");

		WidgetToConfig::read_config_and_connect(m_checkCenterSubtitle, "subtitle-view", "property-alignment-center");
		WidgetToConfig::read_config_and_connect(m_checkShowCharacterPerLine, "subtitle-view", "show-character-per-line");
		WidgetToConfig::read_config_and_connect(m_checkEnableRubberbandSelection, "subtitle-view", "enable-rubberband-selection");
		WidgetToConfig::read_config_and_connect(m_checkUsedCtrlEnterToConfirmChange, "subtitle-view", "used-ctrl-enter-to-confirm-change");
		
		WidgetToConfig::read_config_and_connect(m_checkCreateBackupCopy, "interface", "create-backup-copy");
		WidgetToConfig::read_config_and_connect(m_checkAutosave, "interface", "used-autosave");
		WidgetToConfig::read_config_and_connect(m_spinAutosave, "interface", "autosave-minutes");

		m_checkCreateBackupCopy->set_sensitive(false);
	}


protected:
	Gtk::CheckButton* m_checkUseDynamicKeyboardShortcuts;
	Gtk::CheckButton* m_checkMaximizeWindow;
	Gtk::CheckButton* m_checkAskToSaveOnExit;

	Gtk::CheckButton* m_checkShowCharacterPerLine;
	Gtk::CheckButton* m_checkCenterSubtitle;
	Gtk::CheckButton* m_checkEnableRubberbandSelection;
	Gtk::CheckButton* m_checkUsedCtrlEnterToConfirmChange;

	Gtk::CheckButton* m_checkCreateBackupCopy;
	Gtk::CheckButton* m_checkAutosave;
	Gtk::SpinButton*	m_spinAutosave;
};


struct t_output
{
	const gchar *label;
	const gchar *name;
};

t_output m_audio_output[]={
	{_("Autodetect"), "autoaudiosink"},
	{_("ALSA - Advanced Linux Sound Architecture"), "alsasink"},
	{_("ESD - Enlightenment Sound Daemon"), "esdsink"},
	{_("OSS - Open Sound System"), "ossink"},
	{_("SDL - Simple DirectMedia Layer"), "sdlaudiosink"},
	{_("GConf"), "gconfaudiosink"},
	{NULL, NULL}
};

t_output m_video_output[]={
	{_("Autodetect"), "autovideosink"},
	{_("X Window System (X11/XShm/Xv)"), "xvimagesink"},
	{_("X Window System (No Xv)"), "ximagesink"},
	{_("SDL - Simple DirectMedia Layer"), "sdlvideosink"},
	{_("GConf"), "gconfvideosink"},
	{_("OpenGL"), "glimagesink"},
	{NULL, NULL}
};

/*
 *	Video Player (gstreamer)
 *	Subtitle font, use shaded background, color blance
 */
class PreferenceVideoPlayerUI : public Gtk::VBox
{
public:
	PreferenceVideoPlayerUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		refGlade->get_widget("fontbutton-subtitle", m_fontbuttonSubtitle);
		refGlade->get_widget("check-use-shaded-background", m_checkUseShadedBackground);
		refGlade->get_widget("check-display-translated-subtitle", m_checkDisplayTranslatedSubtitle);

		refGlade->get_widget("check-force-aspect-ratio", m_checkForceAspectRatio);
		refGlade->get_widget("check-automatically-open-video", m_checkAutomaticallyOpenVideo);
		
		refGlade->get_widget_derived("combo-audio-output", m_comboAudioOutput);
		refGlade->get_widget_derived("combo-video-output", m_comboVideoOutput);
		
		for(unsigned int i=0; m_audio_output[i].label != NULL; ++i)
		{
			m_comboAudioOutput->append_text(m_audio_output[i].label);
		}

		for(unsigned int i=0; m_video_output[i].label != NULL; ++i)
		{
			m_comboVideoOutput->append_text(m_video_output[i].label);
		}

		WidgetToConfig::read_config_and_connect(m_checkUseShadedBackground, "video-player", "shaded-background");
		WidgetToConfig::read_config_and_connect(m_fontbuttonSubtitle, "video-player", "font-desc");
		WidgetToConfig::read_config_and_connect(m_checkDisplayTranslatedSubtitle, "video-player", "display-translated-subtitle");

		WidgetToConfig::read_config_and_connect(m_checkForceAspectRatio, "video-player", "force-aspect-ratio");
		WidgetToConfig::read_config_and_connect(m_checkAutomaticallyOpenVideo, "video-player", "automatically-open-video");

		Glib::ustring audiosink, videosink;
		Config::getInstance().get_value_string("video-player", "audio-sink", audiosink);
		Config::getInstance().get_value_string("video-player", "video-sink", videosink);

		m_comboAudioOutput->set_active_text(get_output_label_by_name(m_audio_output, audiosink));
		m_comboVideoOutput->set_active_text(get_output_label_by_name(m_video_output, videosink));

		m_comboAudioOutput->signal_changed().connect(
				sigc::mem_fun(*this, &PreferenceVideoPlayerUI::on_audio_output_changed));
		m_comboVideoOutput->signal_changed().connect(
				sigc::mem_fun(*this, &PreferenceVideoPlayerUI::on_video_output_changed));
	}

protected:

	Glib::ustring get_output_label_by_name(t_output output[], const Glib::ustring &name)
	{
		for(unsigned int i=0; output[i].name != NULL; ++i)
		{
			if(name == output[i].name)
				return output[i].label;
		}
		return Glib::ustring();
	}

	Glib::ustring get_output_name_by_label(t_output output[], const Glib::ustring &label)
	{
		for(unsigned int i=0; output[i].name != NULL; ++i)
		{
			if(label == output[i].label)
				return output[i].name;
		}
		return Glib::ustring();
	}

	void on_audio_output_changed()
	{
		Glib::ustring name = get_output_name_by_label(m_audio_output, m_comboAudioOutput->get_active_text());
		Config::getInstance().set_value_string("video-player", "audio-sink", name);
	}

	void on_video_output_changed()
	{
		Glib::ustring name = get_output_name_by_label(m_video_output, m_comboVideoOutput->get_active_text());
		Config::getInstance().set_value_string("video-player", "video-sink", name);
	}
protected:
	Gtk::FontButton* m_fontbuttonSubtitle;
	Gtk::CheckButton* m_checkUseShadedBackground;
	Gtk::CheckButton* m_checkDisplayTranslatedSubtitle;

	Gtk::CheckButton* m_checkForceAspectRatio;
	Gtk::CheckButton* m_checkAutomaticallyOpenVideo;

	ComboBoxText*	m_comboAudioOutput;
	ComboBoxText*	m_comboVideoOutput;
};


/*
 *	Waveform Preference
 *	Color, Waveform Generator
 */
class PreferenceWaveformUI : public Gtk::VBox
{
public:
	PreferenceWaveformUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		se_debug(SE_DEBUG_APP);

		refGlade->get_widget("colorbutton-background", m_colorbuttonBackground);
		refGlade->get_widget("colorbutton-text", m_colorbuttonText);
		refGlade->get_widget("colorbutton-wave", m_colorbuttonWave);
		refGlade->get_widget("colorbutton-wave-fill", m_colorbuttonWaveFill);
		refGlade->get_widget("colorbutton-subtitle", m_colorbuttonSubtitle);
		refGlade->get_widget("colorbutton-subtitle-selected", m_colorbuttonSubtitleSelected);
		refGlade->get_widget("colorbutton-subtitle-invalid", m_colorbuttonSubtitleInvalid);
		refGlade->get_widget("colorbutton-player-position", m_colorbuttonPlayerPosition);

		refGlade->get_widget("check-display-background", m_checkDisplayBackground);
		refGlade->get_widget("check-display-waveform-fill", m_checkDisplayWaveformFill);
		refGlade->get_widget("check-display-subtitle-text", m_checkDisplaySubtitleText);
		refGlade->get_widget("button-reset-to-defaults-waveform-color", m_buttonReset);

		WidgetToConfig::read_config_and_connect(m_colorbuttonBackground, 
				"waveform-renderer", "color-background");
		WidgetToConfig::read_config_and_connect(m_colorbuttonWave, 
				"waveform-renderer", "color-wave");
		WidgetToConfig::read_config_and_connect(m_colorbuttonWaveFill, 
				"waveform-renderer", "color-wave-fill");
		WidgetToConfig::read_config_and_connect(m_colorbuttonSubtitle, 
				"waveform-renderer", "color-subtitle");
		WidgetToConfig::read_config_and_connect(m_colorbuttonSubtitleSelected, 
				"waveform-renderer", "color-subtitle-selected");
		WidgetToConfig::read_config_and_connect(m_colorbuttonSubtitleInvalid, 
				"waveform-renderer", "color-subtitle-invalid");
		WidgetToConfig::read_config_and_connect(m_colorbuttonText, 
				"waveform-renderer", "color-text");
		WidgetToConfig::read_config_and_connect(m_colorbuttonPlayerPosition, 
				"waveform-renderer", "color-player-position");


		WidgetToConfig::read_config_and_connect(m_checkDisplayBackground, "waveform", "display-background");
		WidgetToConfig::read_config_and_connect(m_checkDisplayWaveformFill, "waveform", "display-waveform-fill");
		WidgetToConfig::read_config_and_connect(m_checkDisplaySubtitleText, "waveform", "display-subtitle-text");

		m_buttonReset->signal_clicked().connect(
				sigc::mem_fun(*this, &PreferenceWaveformUI::on_reset));
	}

protected:

	void on_reset()
	{
		se_debug(SE_DEBUG_APP);

		Glib::ustring value;
		Config &cfg = Config::getInstance();

#define RESET_COLOR(w, key) \
		if(cfg.set_default_value("waveform-renderer", key)) \
		{ \
			cfg.get_default_value("waveform-renderer", key, value); \
			Color color(value); \
			color.initColorButton(*w); \
		}

		RESET_COLOR(m_colorbuttonBackground, "color-background");
		RESET_COLOR(m_colorbuttonPlayerPosition, "color-player-position");
		RESET_COLOR(m_colorbuttonWave, "color-text");
		RESET_COLOR(m_colorbuttonWave, "color-wave");
		RESET_COLOR(m_colorbuttonWaveFill, "color-wave-fill");
		RESET_COLOR(m_colorbuttonSubtitle, "color-marker");
		RESET_COLOR(m_colorbuttonSubtitleSelected, "color-subtitle-selected");
		RESET_COLOR(m_colorbuttonSubtitleInvalid, "color-subtitle-invalid");

#undef RESET_COLOR
	}

protected:
	Gtk::ColorButton*	m_colorbuttonBackground;
	Gtk::ColorButton*	m_colorbuttonPlayerPosition;
	Gtk::ColorButton*	m_colorbuttonText;
	Gtk::ColorButton*	m_colorbuttonWave;
	Gtk::ColorButton*	m_colorbuttonWaveFill;
	Gtk::ColorButton*	m_colorbuttonSubtitle;
	Gtk::ColorButton*	m_colorbuttonSubtitleSelected;
	Gtk::ColorButton*	m_colorbuttonSubtitleInvalid;

	Gtk::CheckButton* m_checkDisplayBackground;
	Gtk::CheckButton* m_checkDisplayWaveformFill;
	Gtk::CheckButton* m_checkDisplaySubtitleText;

	Gtk::Button*			m_buttonReset;
};


/*
 *	Timing/Synchro
 */
class PreferenceTimingUI : public Gtk::VBox
{
public:
	PreferenceTimingUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		refGlade->get_widget("spin-min-characters-per-second", m_spinMinCharactersPerSecond);
		refGlade->get_widget("spin-max-characters-per-second", m_spinMaxCharactersPerSecond);
		refGlade->get_widget("spin-min-gap-between-subtitles", m_spinMinGapBetweenSubtitles);
		refGlade->get_widget("spin-min-display", m_spinMinDisplay);
		refGlade->get_widget("spin-max-characters-per-line", m_spinMaxCharactersPerLine);
		refGlade->get_widget("spin-max-line-per-subtitle", m_spinMaxLinePerSubtitle);

		WidgetToConfig::read_config_and_connect(m_spinMinCharactersPerSecond, "timing", "min-characters-per-second");
		WidgetToConfig::read_config_and_connect(m_spinMaxCharactersPerSecond, "timing", "max-characters-per-second");
		WidgetToConfig::read_config_and_connect(m_spinMinGapBetweenSubtitles, "timing", "min-gap-between-subtitles");
		WidgetToConfig::read_config_and_connect(m_spinMinDisplay, "timing", "min-display");
		WidgetToConfig::read_config_and_connect(m_spinMaxCharactersPerLine, "timing", "max-characters-per-line");
		WidgetToConfig::read_config_and_connect(m_spinMaxLinePerSubtitle, "timing", "max-line-per-subtitle");
	}

protected:
	Gtk::SpinButton* m_spinMinCharactersPerSecond;
	Gtk::SpinButton* m_spinMaxCharactersPerSecond;
	Gtk::SpinButton* m_spinMinGapBetweenSubtitles;
	Gtk::SpinButton* m_spinMinDisplay;
	Gtk::SpinButton* m_spinMaxCharactersPerLine;
	Gtk::SpinButton* m_spinMaxLinePerSubtitle;
};

/*
 *
 */
class PreferencePreviewUI : public Gtk::VBox
{
public:
	PreferencePreviewUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		refGlade->get_widget("entry-video-player-command", m_entryVideoPlayerCommand);
		
		WidgetToConfig::read_config_and_connect(m_entryVideoPlayerCommand, "external-video-player", "command");
	}
protected:
	Gtk::Entry* m_entryVideoPlayerCommand;
};


/*
 *
 */
PreferencesUI::PreferencesUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Dialog(cobject)
{
	PreferenceInterfaceUI *interfaceUI = NULL;
	PreferenceWaveformUI *waveformUI = NULL;
	PreferenceVideoPlayerUI *videoplayerUI = NULL;
	PreferenceTimingUI *timingUI = NULL;
	PreferencePreviewUI *previewUI = NULL;

	refGlade->get_widget_derived("vbox-interface", interfaceUI);
	refGlade->get_widget_derived("vbox-waveform", waveformUI);
	refGlade->get_widget_derived("vbox-video-player", videoplayerUI);
	refGlade->get_widget_derived("vbox-timing", timingUI);
	refGlade->get_widget_derived("vbox-preview", previewUI);
}
