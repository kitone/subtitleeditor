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


/*
 *	Open Document
 */
DialogOpenDocument::DialogOpenDocument()
:DialogFileChooser(
		_("Open Document"), 
		"dialog-open-document", 
		Gtk::FILE_CHOOSER_ACTION_OPEN, 
		(EXTRA_ENCODING_WITH_AUTO_DETECTED))
{
	set_select_multiple(true);

	show_filter();
}


/*
 *	Save Document
 */
DialogSaveDocument::DialogSaveDocument()
:DialogFileChooser(
		_("Save Document"), 
		"dialog-save-document", 
		Gtk::FILE_CHOOSER_ACTION_SAVE, 
		(EXTRA_ENCODING | EXTRA_FORMAT | EXTRA_NEWLINE))
{
	show_filter();
}

/*
 *	Open Movie
 */
DialogOpenVideo::DialogOpenVideo()
:Gtk::FileChooserDialog(_("Open Video"), Gtk::FILE_CHOOSER_ACTION_OPEN)
{
	Gtk::FileFilter m_filter;
	m_filter.set_name(_("Video"));

	m_filter.add_pattern("*.avi");
	m_filter.add_pattern("*.wma");
	m_filter.add_pattern("*.mkv");
	m_filter.add_pattern("*.mpg");
	m_filter.add_pattern("*.mpeg");

	m_filter.add_mime_type("video/*");
	//...
	add_filter(m_filter);

	Gtk::FileFilter m_filterAll;
	m_filterAll.set_name("ALL");
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
	// waveform filter
	Gtk::FileFilter m_filterWaveform;
	m_filterWaveform.set_name("Waveform (*.wf)");
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
	m_filterAll.set_name("ALL");
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

