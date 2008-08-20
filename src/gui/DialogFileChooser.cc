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

#include "DialogFileChooser.h"
#include "utility.h"
#include "RegEx.h"
#include "DialogCharacterCodings.h"


/*
 * Used to define the separator.
 * label = "<separator>"
 */
static bool on_row_separator_func(const Glib::RefPtr<Gtk::TreeModel> &model, const Gtk::TreeModel::iterator &it, const Gtk::TreeModelColumn<Glib::ustring> &col)
{
	Glib::ustring text = (*it)[col];
	if(text == "<separator>")
		return true;
	return false;
}

/*
 * ComboBox for choosing the format of subtitle.
 */
class ComboBoxSubtitleFormat : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxSubtitleFormat(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::ComboBoxText(cobject)
	{
		std::list<Glib::ustring> formats = SubtitleSystem::getInstance().get_formats();

		std::list<Glib::ustring>::const_iterator it;

		for(it = formats.begin(); it != formats.end(); ++it)
			append_text(*it);

		set_active(0);
	}

	/*
	 * Returns the subtitle format selected.
	 */
	Glib::ustring get_value() const
	{
		return get_active_text();
	}
};

/*
 * ComboBox for choosing the NewLine type. 
 * Windows or Unix.
 */
class ComboBoxNewLine : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxNewLine(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::ComboBoxText(cobject)
	{
		append_text("Unix");
		append_text("Windows");

		set_active_text("Unix");
	}

	/*
	 * Returns the NewLine type.
	 * Windows or Unix.
	 */
	Glib::ustring get_value() const
	{
		return get_active_text();
	}
};

/*
 * ComboBox for choosing the encoding.
 * Only the encoding preferences are displayed.
 * The support of "Auto Detected" is enable by default.
 */
class ComboBoxEncoding : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxEncoding(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::ComboBoxText(cobject), m_with_auto_detected(true)
	{
		init_encodings();

		// separator function
		Gtk::ComboBoxText::set_row_separator_func(
			sigc::bind(sigc::ptr_fun(on_row_separator_func), m_text_columns.m_column));

		// m_connection_changed is need to disable the signal when the combobox is rebuild.
		m_connection_changed = signal_changed().connect(
				sigc::mem_fun(*this, &ComboBoxEncoding::on_combo_changed));
	}

	/*
	 * Returns only the charset value.
	 * ex: "UTF-8", "ISO-8859-15" ...
	 * Return empty charset if it's "Auto Detected".
	 */
	Glib::ustring get_value() const
	{
		if(m_with_auto_detected)
		{
			if(get_active_row_number() == 0)
				return Glib::ustring(); // "None"
		}

		// extract the charset 
		// ex:
		// "Unicode (UTF-8)"
		Glib::ustring text = get_active_text();

		Glib::ustring::size_type a = text.find('(');
		Glib::ustring::size_type b = text.find(')', a);

		if(a != Glib::ustring::npos && b != Glib::ustring::npos)
			return text.substr(a+1, b-a-1);

		return Glib::ustring();
	}

	/*
	 * Enable or disable the auto detected mode.
	 */
	void show_auto_detected(bool value)
	{
		m_with_auto_detected = value;

		bool state = is_sensitive();
		set_sensitive(false);

		init_encodings();

		set_sensitive(state);
	}

protected:

	/*
	 * Rebuild the combobox with encoding user preferences.
	 */
	void init_encodings()
	{
		m_connection_changed.block();

		clear();

		bool used_auto_detected = Config::getInstance().get_value_bool("encodings", "used-auto-detected");

		if(m_with_auto_detected)
		{
			append_text(_("Auto Detected"));
			append_text("<separator>");
		}

		std::list<Glib::ustring> encodings = 
			Config::getInstance().get_value_string_list("encodings", "encodings");

		if(encodings.empty())
		{
			std::string charset;
			Glib::get_charset(charset);

			Glib::ustring item;
			item += _("Current Locale");
			item += " (";
			item += charset;
			item += ")";

			append_text(item);
		}
		else
		{
			std::list<Glib::ustring>::const_iterator it;
			for(it = encodings.begin(); it != encodings.end(); ++it)
			{
				EncodingInfo *info = Encodings::get_from_charset(*it);
				if(info)
				{
					Glib::ustring item;
					item+= info->name;
					item+= " (";
					item+= info->charset;
					item+= ")";

					append_text(item);
				}
			}
		}
		// configure
		append_text("<separator>");
		append_text(_("Add or Remove..."));

		if(m_with_auto_detected)
		{
			if(used_auto_detected)
				set_active(0);
			else
				set_active(2); // auto detected (0), separator (1), first charset (2)
		}
		else
			set_active(0);

		m_connection_changed.unblock();
	}

	/*
	 * Gtk::ComboBox::on_changed
	 * Used for intercepte "Add or Remove..."
	 */
	void on_combo_changed()
	{
		unsigned int size = get_model()->children().size();
		unsigned int activated = get_active_row_number();

		if(activated == size-1)
		{
			std::auto_ptr<DialogCharacterCodings> dialog = DialogCharacterCodings::create();
			if(dialog->run() == Gtk::RESPONSE_OK)
			{
				init_encodings();
			}
			else if(m_with_auto_detected)
			{
				if(Config::getInstance().get_value_bool("encodings", "used-auto-detected"))
					set_active(0);
				else
					set_active(2);
			}
			else
				set_active(0);
		}
	}

protected:
	bool m_with_auto_detected;
	sigc::connection m_connection_changed;
};

/*
 * The combobox display all videos on the current folder.
 * Try to found the good video from the selected subtitle.
 */
class ComboBoxVideo : public Gtk::ComboBoxText
{
public:

	/*
	 * Constructor
	 */
	ComboBoxVideo(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::ComboBoxText(cobject)
	{
		// separator function
		Gtk::ComboBoxText::set_row_separator_func(
			sigc::bind(sigc::ptr_fun(on_row_separator_func), m_text_columns.m_column));
	}

	/*
	 * Search all videos in the folder.
	 */
	bool set_current_folder(const Glib::ustring &path)
	{
		clear();

		Glib::Dir dir(path);
		std::vector<std::string> files(dir.begin(), dir.end());

		if(files.empty())
		{
			set_sensitive(false);
			return false;
		}

		std::string f, e;

		RegEx exp("^(.*)\\.((avi)|(wma)|(mkv)|(mpg)|(mpeg)|(ogg)|(mov)|(mp4)|(xvid))$");
		
		for(unsigned int i=0; i<files.size(); ++i)
		{
			if(exp.FullMatch(files[i], &f, &e))
				append_text(files[i]);
		}

		if(get_model()->children().size() == 0)
		{
			set_sensitive(false);
			return false;
		}
		
		set_sensitive(true);

		prepend_text("<separator>");
		prepend_text(_("None"));

		return true;
	}

	/*
	 * Try to select the good video from the subtitle.
	 * Only if the option "automatically-open-video" is enabled.
	 */
	bool auto_select_video(const Glib::ustring &subtitle)
	{
		if(get_model()->children().empty())
			return false;

		if(Config::getInstance().get_value_bool("video-player", "automatically-open-video") == false)
		{
			set_active(0);
			return false;
		}

		RegEx exp("^(.*)\\.((avi)|(wma)|(mkv)|(mpg)|(mpeg)|(ogg)|(mov)|(mp4)|(xvid))$");

		Gtk::TreeIter it = get_model()->children().begin();

		for(; it; ++it)
		{
			std::string p, e;
			Glib::ustring video = (*it)[m_text_columns.m_column];

			if(exp.FullMatch(video.c_str(), &p, &e))
			{
				if(subtitle.find(p) != Glib::ustring::npos)
				{
					set_active_text(video);
					return true;
				}
			}
		}

		// not found, active "None"
		set_active(0);

		return false;
	}

	/*
	 * Return the video selected or a empty string.
	 */
	Glib::ustring get_value() const
	{
		if(is_sensitive() == false || get_active_row_number() == 0)
			return Glib::ustring();

		return get_active_text();
	}
};

/*
 * Init dialog filter with from SubtitleSystem.
 */
void init_dialog_subtitle_filters(Gtk::FileChooserDialog *dialog)
{
	g_return_if_fail(dialog);

	std::list<Glib::ustring>::const_iterator it;
	std::list<Glib::ustring> formats = SubtitleSystem::getInstance().get_formats();

	// all supported formats
	{
		Gtk::FileFilter all;

		all.set_name(_("All supported formats (*.ass, *.ssa, *.srt, ...)"));
		for(it = formats.begin(); it != formats.end(); ++it)
		{
			Glib::ustring ext = SubtitleSystem::getInstance().get_extension(*it);
			all.add_pattern("*."+ext);
		}

		dialog->add_filter(all);
	}

	// by format
	{
		for(it = formats.begin(); it != formats.end(); ++it)
		{
			Glib::ustring ext = "*." + SubtitleSystem::getInstance().get_extension(*it);

			Gtk::FileFilter filter;
			filter.set_name((*it) + " (" + ext + ")");
			filter.add_pattern(ext);
			dialog->add_filter(filter);
		}
	}

	// all files
	{
		Gtk::FileFilter all;
		all.set_name(_("All files (*.*)"));
		all.add_pattern("*");
		dialog->add_filter(all);
	}
}


#include "SubtitleEditorWindow.h"

/*
 * DialogFileChooser
 */

/*
 *
 */
DialogFileChooser::DialogFileChooser(BaseObjectType* cobject, const Glib::ustring &name)
:Gtk::FileChooserDialog(cobject), m_name(name)
{
	Glib::ustring last;
	if(Config::getInstance().get_value_string("dialog-last-folder", m_name, last))
		set_current_folder_uri(last);

	utility::set_transient_parent(*this);
}

/*
 *
 */
DialogFileChooser::~DialogFileChooser()
{
	Glib::ustring last = get_current_folder_uri();
	Config::getInstance().set_value_string("dialog-last-folder", m_name, last);
}



/*
 * DialogOpenDocument
 * Dialog open file chooser with Encoding and Video options.
 */

/*
 * Constructor
 */
DialogOpenDocument::DialogOpenDocument(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:DialogFileChooser(cobject, "dialog-open-document")
{
	refGlade->get_widget_derived("combobox-encodings", m_comboEncodings);
	refGlade->get_widget("label-video", m_labelVideo);
	refGlade->get_widget_derived("combobox-video", m_comboVideo);

	signal_current_folder_changed().connect(
			sigc::mem_fun(*this, &DialogOpenDocument::on_current_folder_changed));

	signal_selection_changed().connect(
			sigc::mem_fun(*this, &DialogOpenDocument::on_selection_changed));

	init_dialog_subtitle_filters(this);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	
	set_default_response(Gtk::RESPONSE_OK);
}

/*
 * Returns the encoding value.
 * Charset or empty string (Auto Detected)
 */
Glib::ustring DialogOpenDocument::get_encoding() const
{
	return m_comboEncodings->get_value();
}

/*
 * Returns the video uri or empty string.
 */
Glib::ustring DialogOpenDocument::get_video_uri() const
{
	Glib::ustring video = m_comboVideo->get_value();
	if(video.empty())
		return Glib::ustring();

	return Glib::build_filename(get_current_folder_uri(), video);
}

/*
 *
 */
void DialogOpenDocument::show_video(bool state)
{
	if(state)
	{
		m_labelVideo->show();
		m_comboVideo->show();
	}
	else
	{
		m_labelVideo->hide();
		m_comboVideo->hide();
	}
}

/*
 * Create a instance of the dialog.
 */
DialogOpenDocument::auto_ptr DialogOpenDocument::create()
{
	auto_ptr ptr( 
			utility::get_widget_derived<DialogOpenDocument>("dialog-open-document.glade", "dialog-open-document") );

	return ptr;
}

/*
 * The current folder has changed, need to update the ComboBox Video
 */
void DialogOpenDocument::on_current_folder_changed()
{
	m_comboVideo->set_current_folder(get_current_folder());
}

/*
 * The file selection has changed, need to update the ComboBox Video
 */
void DialogOpenDocument::on_selection_changed()
{
	std::list<Glib::ustring> selected = get_filenames();

	if(selected.size() == 1)
		m_comboVideo->auto_select_video(selected.front());
	else
		m_comboVideo->auto_select_video("");
}



/*
 * DialogSaveDocument
 * Dialog save file chooser with Format, Encoding and NewLine options.
 */

/*
 * Constructor
 */
DialogSaveDocument::DialogSaveDocument(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:DialogFileChooser(cobject, "dialog-save-document")
{
	refGlade->get_widget_derived("combobox-format", m_comboFormat);
	refGlade->get_widget_derived("combobox-encodings", m_comboEncodings);
	refGlade->get_widget_derived("combobox-newline", m_comboNewLine);

	init_dialog_subtitle_filters(this);

	m_comboEncodings->show_auto_detected(false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
	
	set_default_response(Gtk::RESPONSE_OK);
}

/*
 * Returns the subtitle format value.
 */
Glib::ustring DialogSaveDocument::get_format() const
{
	return m_comboFormat->get_value();
}

/*
 * Returns the encoding value or empty string (Auto Detected).
 */
Glib::ustring DialogSaveDocument::get_encoding() const
{
	return m_comboEncodings->get_value();
}

/*
 * Return the newline value.
 * Windows or Unix.
 */
Glib::ustring DialogSaveDocument::get_newline() const
{
	return m_comboNewLine->get_value();
}

/*
 * Create a instance of the dialog.
 */
DialogSaveDocument::auto_ptr DialogSaveDocument::create()
{
	auto_ptr ptr( 
			utility::get_widget_derived<DialogSaveDocument>("dialog-save-document.glade", "dialog-save-document") );

	return ptr;
}

/*
 * DialogExportText
 * Dialog open file chooser with Encoding option.
 */

/*
 * Constructor
 */
DialogImportText::DialogImportText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:DialogFileChooser(cobject, "dialog-import-text")
{
	refGlade->get_widget_derived("combobox-encodings", m_comboEncodings);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
	
	set_default_response(Gtk::RESPONSE_OK);
}

/*
 * Returns the encoding value.
 * Charset or empty string (Auto Detected)
 */
Glib::ustring DialogImportText::get_encoding() const
{
	return m_comboEncodings->get_value();
}

/*
 * Create a instance of the dialog.
 */
DialogImportText::auto_ptr DialogImportText::create()
{
	auto_ptr ptr( 
			utility::get_widget_derived<DialogImportText>("dialog-import-text.glade", "dialog-import-text") );

	return ptr;
}



/*
 * DialogExportText
 * Dialog save file chooser with Encoding and NewLine options.
 */

/*
 * Constructor
 */
DialogExportText::DialogExportText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:DialogFileChooser(cobject, "dialog-export-text")
{
	refGlade->get_widget_derived("combobox-encodings", m_comboEncodings);
	refGlade->get_widget_derived("combobox-newline", m_comboNewLine);

	m_comboEncodings->show_auto_detected(false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
	
	set_default_response(Gtk::RESPONSE_OK);
}

/*
 * Returns the encoding value or empty string (Auto Detected).
 */
Glib::ustring DialogExportText::get_encoding() const
{
	return m_comboEncodings->get_value();
}

/*
 * Return the newline value.
 * Windows or Unix.
 */
Glib::ustring DialogExportText::get_newline() const
{
	return m_comboNewLine->get_value();
}

/*
 * Create a instance of the dialog.
 */
DialogExportText::auto_ptr DialogExportText::create()
{
	auto_ptr ptr( 
			utility::get_widget_derived<DialogExportText>("dialog-export-text.glade", "dialog-export-text") );

	return ptr;
}



/*
 *	Open Movie
 */
DialogOpenVideo::DialogOpenVideo()
:Gtk::FileChooserDialog(_("Open Video"), Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	utility::set_transient_parent(*this);

	// video filter
	Gtk::FileFilter m_filterVideo;
	m_filterVideo.set_name(_("Video"));
	m_filterVideo.add_pattern("*.avi");
	m_filterVideo.add_pattern("*.wma");
	m_filterVideo.add_pattern("*.mkv");
	m_filterVideo.add_pattern("*.mpg");
	m_filterVideo.add_pattern("*.mpeg");
	m_filterVideo.add_mime_type("video/*");
	add_filter(m_filterVideo);

	// audio filter
	Gtk::FileFilter m_filterAudio;
	m_filterAudio.set_name(_("Audio"));
	m_filterAudio.add_pattern("*.mp3");
	m_filterAudio.add_pattern("*.ogg");
	m_filterAudio.add_pattern("*.wav");
	m_filterAudio.add_mime_type("audio/*");
	add_filter(m_filterAudio);

	Gtk::FileFilter m_filterAll;
	m_filterAll.set_name(_("ALL"));
	m_filterAll.add_pattern("*.*");
	add_filter(m_filterAll);
	
	
	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);


	Config &cfg = Config::getInstance();

	Glib::ustring floder;
	if(cfg.get_value_string("dialog-last-folder", "dialog-open-video", floder))
		set_current_folder_uri(floder);
}

/*
 *
 */
DialogOpenVideo::~DialogOpenVideo()
{
	Glib::ustring floder = get_current_folder_uri();

	Config::getInstance().set_value_string("dialog-last-folder", "dialog-open-video", floder);
}

/*
 * Waveform or Audio/Video
 */
DialogOpenWaveform::DialogOpenWaveform()
:Gtk::FileChooserDialog(_("Open Waveform"), Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	utility::set_transient_parent(*this);

	// waveform filter
	Gtk::FileFilter m_filterWaveform;
	m_filterWaveform.set_name(_("Waveform (*.wf)"));
	m_filterWaveform.add_pattern("*.wf");
	add_filter(m_filterWaveform);

	// movies filter
	Gtk::FileFilter m_filterMovie;
	m_filterMovie.set_name(_("Video"));
	m_filterMovie.add_pattern("*.avi");
	m_filterMovie.add_pattern("*.wma");
	m_filterMovie.add_pattern("*.mkv");
	m_filterMovie.add_pattern("*.mpg");
	m_filterMovie.add_pattern("*.mpeg");
	m_filterMovie.add_mime_type("video/*");
	add_filter(m_filterMovie);

	// audio filter
	Gtk::FileFilter m_filterAudio;
	m_filterAudio.set_name(_("Audio"));
	m_filterAudio.add_pattern("*.mp3");
	m_filterAudio.add_pattern("*.ogg");
	m_filterAudio.add_pattern("*.wav");
	m_filterAudio.add_mime_type("audio/*");
	add_filter(m_filterAudio);

	// all filter
	Gtk::FileFilter m_filterAll;
	m_filterAll.set_name(_("ALL"));
	m_filterAll.add_pattern("*.*");
	add_filter(m_filterAll);
	
	
	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
	set_default_response(Gtk::RESPONSE_OK);


	Config &cfg = Config::getInstance();

	Glib::ustring floder;
	if(cfg.get_value_string("dialog-last-folder", "dialog-open-waveform", floder))
		set_current_folder_uri(floder);
}

/*
 *
 */
DialogOpenWaveform::~DialogOpenWaveform()
{
	Glib::ustring floder = get_current_folder_uri();

	Config::getInstance().set_value_string("dialog-last-folder", "dialog-open-waveform", floder);
}

