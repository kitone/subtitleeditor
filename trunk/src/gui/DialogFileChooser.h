#ifndef _DialogFileChooser_h
#define _DialogFileChooser_h

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

#include "DialogUtility.h"

#include <gtkmm.h>
#include <libglademm.h>
#include "SubtitleSystem.h"
#include "Encodings.h"
#include "utility.h"

/*
 * Internal class
 */
class ComboBoxSubtitleFormat;
class ComboBoxNewLine;
class ComboBoxEncoding;
class ComboBoxVideo;


/*
 *
 */
class DialogFileChooser : public Gtk::FileChooserDialog
{
public:

	/*
	 * Glade constructor
	 */
	DialogFileChooser(BaseObjectType* cobject, const Glib::ustring &name);

	/*
	 *
	 */
	virtual ~DialogFileChooser();

protected:
	Glib::ustring m_name;
};


/*
 * Dialog open file chooser with Encoding and Video options.
 */
class DialogOpenDocument : public DialogFileChooser
{
public:

	/*
	 *
	 */
	typedef std::auto_ptr<DialogOpenDocument> auto_ptr;
	
	/*
	 * Constructor
	 */
	DialogOpenDocument(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 * Returns the encoding value.
	 * Charset or empty string (Auto Detected)
	 */
	Glib::ustring get_encoding() const;

	/*
	 * Returns the video uri or empty string.
	 */
	Glib::ustring get_video_uri() const;

	/*
	 *
	 */
	void show_video(bool state);

	/*
	 * Create a instance of the dialog.
	 */
	static auto_ptr create();

protected:

	/*
	 * The current folder has changed, need to update the ComboBox Video
	 */
	void on_current_folder_changed();

	/*
	 * The file selection has changed, need to update the ComboBox Video
	 */
	void on_selection_changed();

protected:
	ComboBoxEncoding* m_comboEncodings;
	Gtk::Label* m_labelVideo;
	ComboBoxVideo* m_comboVideo;
};



/*
 * Dialog save file chooser with Format, Encoding and NewLine options.
 */
class DialogSaveDocument : public DialogFileChooser
{
public:

	/*
	 *
	 */
	typedef std::auto_ptr<DialogSaveDocument> auto_ptr;
	
	/*
	 * Constructor
	 */
	DialogSaveDocument(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 * Returns the subtitle format value.
	 */
	Glib::ustring get_format() const;

	/*
	 * Returns the encoding value. or empty string (Auto Detected).
	 */
	Glib::ustring get_encoding() const;

	/*
	 * Returns the newline value.
	 * Windows or Unix.
	 */
	Glib::ustring get_newline() const;

	/*
	 * Create a instance of the dialog.
	 */
	static auto_ptr create();


protected:
	ComboBoxSubtitleFormat* m_comboFormat;
	ComboBoxEncoding* m_comboEncodings;
	ComboBoxNewLine* m_comboNewLine;
};

/*
 * Dialog Import file chooser with Encoding option.
 */
class DialogImportText : public DialogFileChooser
{
public:

	/*
	 *
	 */
	typedef std::auto_ptr<DialogImportText> auto_ptr;
	
	/*
	 * Constructor
	 */
	DialogImportText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 * Returns the encoding value.
	 * Charset or empty string (Auto Detected)
	 */
	Glib::ustring get_encoding() const;

	/*
	 * Create a instance of the dialog.
	 */
	static auto_ptr create();

protected:
	ComboBoxEncoding* m_comboEncodings;
};


/*
 * Dialog export file chooser with Encoding and NewLine options.
 */
class DialogExportText : public DialogFileChooser
{
public:

	/*
	 *
	 */
	typedef std::auto_ptr<DialogExportText> auto_ptr;
	
	/*
	 * Constructor
	 */
	DialogExportText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 * Returns the encoding value. or empty string (Auto Detected).
	 */
	Glib::ustring get_encoding() const;

	/*
	 * Returns the newline value.
	 * Windows or Unix.
	 */
	Glib::ustring get_newline() const;

	/*
	 * Create a instance of the dialog.
	 */
	static auto_ptr create();


protected:
	ComboBoxEncoding* m_comboEncodings;
	ComboBoxNewLine* m_comboNewLine;
};


/*
 *
 */
class DialogOpenVideo : public Gtk::FileChooserDialog
{
public:
	DialogOpenVideo();
	~DialogOpenVideo();
};

/*
 * Waveform or Video/Audio
 */
class DialogOpenWaveform : public Gtk::FileChooserDialog
{
public:
	DialogOpenWaveform();
	~DialogOpenWaveform();
};

#endif//_DialogFileChooser_h

