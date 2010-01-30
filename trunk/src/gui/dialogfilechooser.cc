/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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

#include "dialogfilechooser.h"
#include "utility.h"
#include "gtkmm_utility.h"
#include "dialogcharactercodings.h"
#include "comboboxencoding.h"
#include "comboboxsubtitleformat.h"
#include "comboboxnewline.h"
#include "comboboxvideo.h"
#include "subtitleformatsystem.h"

/*
 * Init dialog filter with from SubtitleFormatSystem.
 */
void init_dialog_subtitle_filters(Gtk::FileChooserDialog *dialog)
{
	g_return_if_fail(dialog);

	std::list<SubtitleFormatInfo>::const_iterator it;
	std::list<SubtitleFormatInfo> infos = SubtitleFormatSystem::instance().get_infos();

	Gtk::FileFilter *all=manage(new Gtk::FileFilter), *supported = manage(new Gtk::FileFilter);
	// all files
	{
		all->set_name(_("All files (*.*)"));
		all->add_pattern("*");
		dialog->add_filter(*all);
	}

	// all supported formats
	{
		supported->set_name(_("All supported formats (*.ass, *.ssa, *.srt, ...)"));
		for(it = infos.begin(); it != infos.end(); ++it)
		{
			supported->add_pattern("*." + (*it).extension);
		}

		dialog->add_filter(*supported);
	}

	// by format
	{
		for(it = infos.begin(); it != infos.end(); ++it)
		{
			Glib::ustring name = (*it).name;
			Glib::ustring ext = (*it).extension;
	
			Gtk::FileFilter *filter = manage(new Gtk::FileFilter);
			filter->set_name(name + " (" + ext + ")");
			filter->add_pattern("*." + ext);
			dialog->add_filter(*filter);
		}
	}

	// select by default
	dialog->set_filter(*supported);
}

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
 * Define the current file filter.
 * ex: 'Subtitle Editor Project', 'SubRip', 'MicroDVD' ...
 */
void DialogFileChooser::set_current_filter(const Glib::ustring &subtitleformat_name)
{
	std::list<Gtk::FileFilter*> filters = list_filters();
	for(std::list<Gtk::FileFilter*>::const_iterator it = filters.begin(); it != filters.end(); ++it)
	{
		if((*it)->get_name().find(subtitleformat_name) == Glib::ustring::npos)
			continue;
		
		set_filter(*(*it));
		return;
	}
}


/*
 * DialogOpenDocument
 * Dialog open file chooser with Encoding and Video options.
 */

/*
 * Constructor
 */
DialogOpenDocument::DialogOpenDocument(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:DialogFileChooser(cobject, "dialog-open-document")
{
	builder->get_widget_derived("combobox-encodings", m_comboEncodings);
	builder->get_widget("label-video", m_labelVideo);
	builder->get_widget_derived("combobox-video", m_comboVideo);

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
			gtkmm_utility::get_widget_derived<DialogOpenDocument>(
				SE_DEV_VALUE(PACKAGE_UI_DIR, PACKAGE_UI_DIR_DEV),
				"dialog-open-document.ui", 
				"dialog-open-document") );

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
DialogSaveDocument::DialogSaveDocument(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:DialogFileChooser(cobject, "dialog-save-document")
{
	builder->get_widget_derived("combobox-format", m_comboFormat);
	builder->get_widget_derived("combobox-encodings", m_comboEncodings);
	builder->get_widget_derived("combobox-newline", m_comboNewLine);

	init_dialog_subtitle_filters(this);

	m_comboEncodings->show_auto_detected(false);

	add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
	
	set_default_response(Gtk::RESPONSE_OK);

	m_comboFormat->signal_changed().connect(
			sigc::mem_fun(*this, &DialogSaveDocument::on_combo_format_changed));
}

/*
 *
 */
void DialogSaveDocument::set_format(const Glib::ustring &format)
{
	m_comboFormat->set_value(format);
}

/*
 * Returns the subtitle format value.
 */
Glib::ustring DialogSaveDocument::get_format() const
{
	return m_comboFormat->get_value();
}

/*
 *
 */
void DialogSaveDocument::set_encoding(const Glib::ustring &encoding)
{
	m_comboEncodings->set_value(encoding);
}

/*
 * Returns the encoding value or empty string (Auto Detected).
 */
Glib::ustring DialogSaveDocument::get_encoding() const
{
	return m_comboEncodings->get_value();
}

/*
 *
 */
void DialogSaveDocument::set_newline(const Glib::ustring &newline)
{
	m_comboNewLine->set_value(newline);
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
 * Update the extension of the current filename.
 */
void DialogSaveDocument::on_combo_format_changed()
{
	Glib::ustring filename = get_filename();
	if(filename.empty())
		return;

	// Only work with the name of the file
	Glib::ustring basename = Glib::path_get_basename(filename);

	// Try to get the extension from the format
	SubtitleFormatInfo sfinfo;
	if(SubtitleFormatSystem::instance().get_info(get_format(), sfinfo) == false)
		return;
	
	// Test if the name already have an extension then replace the 
	// previous extension by the new one or simply add the extension to the name.
	Glib::RefPtr<Glib::Regex> re = Glib::Regex::create("^(.*)(\\.)(.*)$");
	if(re->match(basename))
		basename = re->replace(basename, 0, "\\1." + sfinfo.extension, Glib::RegexMatchFlags(0));
	else
		basename = basename + "." + sfinfo.extension;

	// Update only the current name
	set_current_name(basename);
}

/*
 * Create a instance of the dialog.
 */
DialogSaveDocument::auto_ptr DialogSaveDocument::create()
{
	auto_ptr ptr( 
			gtkmm_utility::get_widget_derived<DialogSaveDocument>(
				SE_DEV_VALUE(PACKAGE_UI_DIR, PACKAGE_UI_DIR_DEV), 
				"dialog-save-document.ui", 
				"dialog-save-document") );

	return ptr;
}

/*
 * DialogExportText
 * Dialog open file chooser with Encoding option.
 */

/*
 * Constructor
 */
DialogImportText::DialogImportText(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:DialogFileChooser(cobject, "dialog-import-text")
{
	builder->get_widget_derived("combobox-encodings", m_comboEncodings);

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
			gtkmm_utility::get_widget_derived<DialogImportText>(
				SE_DEV_VALUE(PACKAGE_UI_DIR, PACKAGE_UI_DIR_DEV), 
				"dialog-import-text.ui", 
				"dialog-import-text") );

	return ptr;
}



/*
 * DialogExportText
 * Dialog save file chooser with Encoding and NewLine options.
 */

/*
 * Constructor
 */
DialogExportText::DialogExportText(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:DialogFileChooser(cobject, "dialog-export-text")
{
	builder->get_widget_derived("combobox-encodings", m_comboEncodings);
	builder->get_widget_derived("combobox-newline", m_comboNewLine);

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
			gtkmm_utility::get_widget_derived<DialogExportText>(
				SE_DEV_VALUE(PACKAGE_UI_DIR, PACKAGE_UI_DIR_DEV),
				"dialog-export-text.ui", 
				"dialog-export-text") );

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

	// waveform, video and audio filter
	Gtk::FileFilter m_filterSupported;
	m_filterSupported.set_name(_("Waveform & Media"));
	m_filterSupported.add_pattern("*.wf");
	m_filterSupported.add_mime_type("video/*");
	m_filterSupported.add_pattern("*.avi");
	m_filterSupported.add_pattern("*.wma");
	m_filterSupported.add_pattern("*.mkv");
	m_filterSupported.add_pattern("*.mpg");
	m_filterSupported.add_pattern("*.mpeg");
	m_filterSupported.add_mime_type("audio/*");
	m_filterSupported.add_pattern("*.mp3");
	m_filterSupported.add_pattern("*.ogg");
	m_filterSupported.add_pattern("*.wav");
	add_filter(m_filterSupported);

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


/*
 * Keyframes or Video
 */
DialogOpenKeyframe::DialogOpenKeyframe()
:Gtk::FileChooserDialog(_("Open Keyframe"), Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	utility::set_transient_parent(*this);

	// keyframes and video filter
	Gtk::FileFilter m_filterSupported;
	m_filterSupported.set_name(_("Keyframe & Media"));
	m_filterSupported.add_pattern("*.kf");
	m_filterSupported.add_mime_type("video/*");
	m_filterSupported.add_pattern("*.avi");
	m_filterSupported.add_pattern("*.wma");
	m_filterSupported.add_pattern("*.mkv");
	m_filterSupported.add_pattern("*.mpg");
	m_filterSupported.add_pattern("*.mpeg");
	add_filter(m_filterSupported);

	// keyframe filter
	Gtk::FileFilter m_filterKeyframe;
	m_filterKeyframe.set_name(_("Keyframe (*.kf)"));
	m_filterKeyframe.add_pattern("*.kf");
	add_filter(m_filterKeyframe);

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
	if(cfg.get_value_string("dialog-last-folder", "dialog-open-keyframe", floder))
		set_current_folder_uri(floder);
}

/*
 */
DialogOpenKeyframe::~DialogOpenKeyframe()
{
	Glib::ustring floder = get_current_folder_uri();

	Config::getInstance().set_value_string("dialog-last-folder", "dialog-open-keyframe", floder);
}

