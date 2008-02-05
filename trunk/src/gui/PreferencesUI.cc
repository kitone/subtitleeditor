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
#include "Encodings.h"

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

		refGlade->get_widget("check-create-backup-copy", m_checkCreateBackupCopy);
		refGlade->get_widget("check-autosave", m_checkAutosave);
		refGlade->get_widget("spin-autosave", m_spinAutosave);

		WidgetToConfig::read_config_and_connect(m_checkUseDynamicKeyboardShortcuts, "interface", "use-dynamic-keyboard-shortcuts");
		WidgetToConfig::read_config_and_connect(m_checkMaximizeWindow, "interface", "maximize-window");
		WidgetToConfig::read_config_and_connect(m_checkAskToSaveOnExit, "interface", "ask-to-save-on-exit");

		WidgetToConfig::read_config_and_connect(m_checkCenterSubtitle, "subtitle-view", "property-alignment-center");
		WidgetToConfig::read_config_and_connect(m_checkShowCharacterPerLine, "subtitle-view", "show-character-per-line");
		WidgetToConfig::read_config_and_connect(m_checkEnableRubberbandSelection, "subtitle-view", "enable-rubberband-selection");
		
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

	Gtk::CheckButton* m_checkCreateBackupCopy;
	Gtk::CheckButton* m_checkAutosave;
	Gtk::SpinButton*	m_spinAutosave;
};

/*
 *	Encodings
 */
class PreferencesEncodingsUI : public Gtk::VBox
{
	class ColumnEncoding : public Gtk::TreeModel::ColumnRecord
	{
	public:
		ColumnEncoding()
		{
			add(use);
			add(name);
			add(charset);
		}
		Gtk::TreeModelColumn<Glib::ustring> use;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> charset;
	};

public:
	PreferencesEncodingsUI(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::VBox(cobject)
	{
		refGlade->get_widget("treeview-encodings", m_treeviewEncodings);
		refGlade->get_widget("button-add-charset", m_buttonAddCharset);
		refGlade->get_widget("button-remove-charset", m_buttonRemoveCharset);
		refGlade->get_widget("button-up-charset", m_buttonUpCharset);
		refGlade->get_widget("button-down-charset", m_buttonDownCharset);
		refGlade->get_widget("check-used-auto-detected", m_checkUsedAutoDetected);

		m_buttonAddCharset->signal_clicked().connect(
				sigc::mem_fun(*this, &PreferencesEncodingsUI::on_add_charset));
		m_buttonRemoveCharset->signal_clicked().connect(
				sigc::mem_fun(*this, &PreferencesEncodingsUI::on_remove_charset));
		m_buttonUpCharset->signal_clicked().connect(
				sigc::mem_fun(*this, &PreferencesEncodingsUI::on_up_charset));
		m_buttonDownCharset->signal_clicked().connect(
				sigc::mem_fun(*this, &PreferencesEncodingsUI::on_down_charset));

		WidgetToConfig::read_config_and_connect(m_checkUsedAutoDetected, "encodings", "used-auto-detected");

		createTreeView(m_treeviewEncodings);

		load_config();
	}

	Glib::RefPtr<Gtk::ListStore> createTreeView(Gtk::TreeView *view)
	{
		ColumnEncoding m_column;
		Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(m_column);

		view->set_model(model);

		Gtk::TreeViewColumn* column = NULL;
		Gtk::CellRendererText* name = NULL;
		Gtk::CellRendererText* charset = NULL;

		// column name
		column = manage(new Gtk::TreeViewColumn(_("Name")));
		//column->set_sort_indicator(true);
		//column->set_clickable(true);
		view->append_column(*column);
	
		name = manage(new Gtk::CellRendererText);
		column->pack_start(*name, false);
		column->add_attribute(name->property_text(), m_column.name);

		// column charset
		column = manage(new Gtk::TreeViewColumn(_("Charset")));
		//column->set_sort_indicator(true);
		//column->set_clickable(true);
		view->append_column(*column);

		charset = manage(new Gtk::CellRendererText);
		column->pack_start(*charset, false);
		column->add_attribute(charset->property_text(), m_column.charset);

		return model;
	}

	void load_config()
	{
		Glib::RefPtr<Gtk::ListStore> model = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());
		
		ColumnEncoding m_column;
		
		// add encodings pref 
		std::list<Glib::ustring> list_encodings;
		if(Config::getInstance().get_value_string_list("encodings", "encodings", list_encodings))
		{
			std::list<Glib::ustring>::const_iterator it;
			for(it = list_encodings.begin(); it!=list_encodings.end(); ++it)
			{
				EncodingInfo *info= Encodings::get_from_charset(*it);
				if(info)
				{
					Gtk::TreeIter it = model->append();

					(*it)[m_column.name] = info->name;
					(*it)[m_column.charset] = info->charset;
				}
			}
		}
	}

	void save_config()
	{
		Glib::RefPtr<Gtk::ListStore> m_model = 
			Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());

		Config &cfg = Config::getInstance();

		std::list<Glib::ustring> list;
		
		ColumnEncoding m_column;
	
		Gtk::TreeNodeChildren rows = m_model->children();

		for(Gtk::TreeIter iter = rows.begin(); iter; ++iter)
			list.push_back((*iter)[m_column.charset]);

		cfg.set_value_string_list("encodings", "encodings", list);
	}

	void on_add_charset()
	{
		Gtk::Dialog *dialog = NULL;
		Gtk::TreeView* treeview = NULL;

		Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(get_share_dir("glade/dialog-encodings-chooser.glade"));
	
		refXml->get_widget("dialog-encodings-chooser", dialog);
		refXml->get_widget("treeviewEncodings", treeview);

		Glib::RefPtr<Gtk::ListStore> model = createTreeView(treeview);
		ColumnEncoding m_column;
		
		// add encodings
		for(unsigned int i=0; encodings_info[i].charset != NULL; ++i)
		{
			Gtk::TreeIter it = model->append();
		
			EncodingInfo *info= Encodings::get_from_charset(encodings_info[i].charset);
			if(info)
			{
				(*it)[m_column.name] = info->name;
				(*it)[m_column.charset] = info->charset;
			}
		}

		if( dialog->run() == Gtk::RESPONSE_OK)
		{
			Gtk::TreeIter iter = treeview->get_selection()->get_selected();
		
			if(iter)
			{
				Glib::ustring name = (*iter)[m_column.name];
				Glib::ustring charset = (*iter)[m_column.charset];
				// add in the list
				Glib::RefPtr<Gtk::ListStore> modelEncodings = 
					Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());
				
				Gtk::TreeIter it = modelEncodings->append();
				(*it)[m_column.name] = name;
				(*it)[m_column.charset] = charset;
			}
		}

		delete dialog;

		save_config();
	}

	void on_remove_charset()
	{
		Glib::RefPtr<Gtk::ListStore> modelEncodings = 
			Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());

		Gtk::TreeIter iter = m_treeviewEncodings->get_selection()->get_selected();

		if(iter)
		{
			modelEncodings->erase(iter);
			save_config();
		}
	}

	void on_up_charset()
	{
		Glib::RefPtr<Gtk::ListStore> m_model = 
			Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());

		Gtk::TreeIter it = m_treeviewEncodings->get_selection()->get_selected();
		if(it)
		{
			Gtk::TreePath path = m_model->get_path(it);

			if(path.prev())
			{
				Gtk::TreeIter prev = m_model->get_iter(path);
				if(prev)
				{
					m_model->move(it, prev);
					save_config();
				}
			}
		}
	}

	void on_down_charset()
	{
		Glib::RefPtr<Gtk::ListStore> m_model = 
			Glib::RefPtr<Gtk::ListStore>::cast_dynamic(m_treeviewEncodings->get_model());

		Gtk::TreeIter it = m_treeviewEncodings->get_selection()->get_selected();
		if(it)
		{
			Gtk::TreeIter next = it;
			++next;
			if(next)
			{
				m_model->move(next, it);
				save_config();
			}
		}
	}

protected:
	Gtk::TreeView* m_treeviewEncodings;
	Gtk::Button* m_buttonAddCharset;
	Gtk::Button* m_buttonRemoveCharset;
	Gtk::Button* m_buttonUpCharset;
	Gtk::Button* m_buttonDownCharset;
	Gtk::CheckButton* m_checkUsedAutoDetected;
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
	{_("OpenGL "), "glimagesink"},
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
		refGlade->get_widget("colorbutton-play-line", m_colorbuttonPlayLine);
		refGlade->get_widget("colorbutton-text", m_colorbuttonText);
		refGlade->get_widget("colorbutton-wave", m_colorbuttonWave);
		refGlade->get_widget("colorbutton-wave-fill", m_colorbuttonWaveFill);
		refGlade->get_widget("colorbutton-marker", m_colorbuttonMarker);
		refGlade->get_widget("colorbutton-marker-hightlight", m_colorbuttonMarkerHightlight);
		refGlade->get_widget("colorbutton-marker-invalid", m_colorbuttonMarkerInvalid);

		refGlade->get_widget("check-display-background", m_checkDisplayBackground);
		refGlade->get_widget("check-display-waveform-fill", m_checkDisplayWaveformFill);
		refGlade->get_widget("check-display-subtitle-text", m_checkDisplaySubtitleText);
		refGlade->get_widget("button-reset-to-defaults-waveform-color", m_buttonReset);

		WidgetToConfig::read_config_and_connect(m_colorbuttonBackground, "waveform", "color-background");
		WidgetToConfig::read_config_and_connect(m_colorbuttonPlayLine, "waveform", "color-play-line");
		WidgetToConfig::read_config_and_connect(m_colorbuttonText, "waveform", "color-text");
		WidgetToConfig::read_config_and_connect(m_colorbuttonWave, "waveform", "color-wave");
		WidgetToConfig::read_config_and_connect(m_colorbuttonWaveFill, "waveform", "color-wave-fill");
		WidgetToConfig::read_config_and_connect(m_colorbuttonMarker, "waveform", "color-marker");
		WidgetToConfig::read_config_and_connect(m_colorbuttonMarkerHightlight, "waveform", "color-marker-hightlight");
		WidgetToConfig::read_config_and_connect(m_colorbuttonMarkerInvalid, "waveform", "color-marker-invalid");
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
		if(cfg.set_default_value("waveform", key)) \
		{ \
			cfg.get_default_value("waveform", key, value); \
			Color color(value); \
			color.initColorButton(*w); \
		}

		RESET_COLOR(m_colorbuttonBackground, "color-background");
		RESET_COLOR(m_colorbuttonPlayLine, "color-play-line");
		RESET_COLOR(m_colorbuttonWave, "color-text");
		RESET_COLOR(m_colorbuttonWave, "color-wave");
		RESET_COLOR(m_colorbuttonWaveFill, "color-wave-fill");
		RESET_COLOR(m_colorbuttonMarker, "color-marker");
		RESET_COLOR(m_colorbuttonMarkerHightlight, "color-marker-hightlight");
		RESET_COLOR(m_colorbuttonMarkerInvalid, "color-marker-invalid");

#undef RESET_COLOR
	}

protected:
	Gtk::ColorButton*	m_colorbuttonBackground;
	Gtk::ColorButton*	m_colorbuttonPlayLine;
	Gtk::ColorButton*	m_colorbuttonText;
	Gtk::ColorButton*	m_colorbuttonWave;
	Gtk::ColorButton*	m_colorbuttonWaveFill;
	Gtk::ColorButton*	m_colorbuttonMarker;
	Gtk::ColorButton*	m_colorbuttonMarkerHightlight;
	Gtk::ColorButton*	m_colorbuttonMarkerInvalid;

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
	PreferencesEncodingsUI *encodingsUI = NULL;
	PreferenceWaveformUI *waveformUI = NULL;
	PreferenceVideoPlayerUI *videoplayerUI = NULL;
	PreferenceTimingUI *timingUI = NULL;
	PreferencePreviewUI *previewUI = NULL;

	refGlade->get_widget_derived("vbox-interface", interfaceUI);
	refGlade->get_widget_derived("vbox-encodings", encodingsUI);
	refGlade->get_widget_derived("vbox-waveform", waveformUI);
	refGlade->get_widget_derived("vbox-video-player", videoplayerUI);
	refGlade->get_widget_derived("vbox-timing", timingUI);
	refGlade->get_widget_derived("vbox-preview", previewUI);
}
