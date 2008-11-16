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
 

#include "document.h"
#include "cfg.h"
#include <iostream>
#include "utility.h"
#include "gui/comboboxencoding.h"
#include "gui/dialogutility.h"
#include "encodings.h"
#include <gtkmm.h>
#include "error.h"
#include <ctime>
#include <memory>
#include "subtitleformatsystem.h"

/*
 *	constructeur
 *	s'il y a une option dans le fichier config [encodings] default=xxxx
 *	on l'utilise pour choisir le charset, sinon on utilise UTF-8
 */
Document::Document()
:CommandSystem(*this), m_subtitles(*this), m_styles(*this), m_subtitleView(NULL)
{
	m_timing_mode = TIME;
	m_edit_timing_mode = TIME;
	m_framerate = FRAMERATE_25;

	m_document_changed = false;

	// sets default values
	Config &cfg = Config::getInstance();

	// sets default encoding value
	Glib::ustring default_encoding = cfg.get_value_string("encodings", "default");
	
	m_charset = (default_encoding.empty()) ? "UTF-8" : default_encoding;

	// sets default document format
	Glib::ustring default_format = cfg.get_value_string("document", "format");

	m_format = (SubtitleFormatSystem::instance().is_supported(default_format)) ? default_format : "SubRip";

	// sets default newline
	Glib::ustring default_newline = cfg.get_value_string("document", "newline");

	m_newline = (default_newline.empty()) ? "Unix" : default_newline;

	// create models
	m_subtitleModel = Glib::RefPtr<SubtitleModel>(new SubtitleModel(this));

	m_styleModel = Glib::RefPtr<StyleModel>(new StyleModel);

	//m_nameModel = Glib::RefPtr<NameModel>(new NameModel);
	CommandSystem::signal_changed().connect(
			sigc::mem_fun(*this, &Document::make_document_changed));
}

/*
 *	constructeur par copy
 */
Document::Document(Document &src)
:CommandSystem(*this), m_subtitles(*this), m_styles(*this), m_subtitleView(NULL)
{
	m_timing_mode = src.m_timing_mode;
	m_edit_timing_mode = src.m_edit_timing_mode;
	m_framerate = src.m_framerate;

	m_document_changed = false;

	m_subtitleModel = Glib::RefPtr<SubtitleModel>(new SubtitleModel(this));
	m_styleModel = Glib::RefPtr<StyleModel>(new StyleModel);

	g_return_if_fail(m_subtitleModel);
	g_return_if_fail(m_styleModel);

	m_format = src.m_format;
	m_charset = src.m_charset;
	m_newline = src.m_newline;

	m_scriptInfo = src.m_scriptInfo;
	
	m_name = src.m_name;
	m_filename = src.m_filename;

	m_uriMovie = src.m_uriMovie;
	
	// model
	m_subtitleModel->copy(src.get_subtitle_model());

	m_styleModel->copy(src.get_style_model());

	m_data = src.m_data;

	CommandSystem::signal_changed().connect(
			sigc::mem_fun(*this, &Document::make_document_changed));
}

/*
 *	destructeur
 */
Document::~Document()
{
}

/*
 *
 */
Gtk::Widget* Document::widget()
{
	return get_subtitle_view();
}

/*
 *	le nom complet du document 
 *	ex: /home/toto/subtitle05.ass
 */
void Document::setFilename(const Glib::ustring &filename)
{
	m_filename = utility::create_full_path(filename);

	m_name = Glib::path_get_basename(m_filename);

	emit_signal("document-property-changed");
}

/*
 *	le nom complet du document 
 *	ex: /home/toto/subtitle05.ass
 */
Glib::ustring Document::getFilename()
{
	return m_filename;
}

/*
 *	le nom du document
 *	si filename = /home/toto/subtitle05.ass
 *	name = subtitle05.ass
 */
void Document::setName(const Glib::ustring &name)
{
	m_name = name;

	emit_signal("document-property-changed");
}

/*
 *	le nom du document
 *	si filename = /home/toto/subtitle05.ass
 *	name = subtitle05.ass
 */
Glib::ustring Document::getName()
{
	return m_name;
}

/*
 *
 */
void Document::setFormat(const Glib::ustring &format)
{
	m_format = format;
}

/*
 *
 */
Glib::ustring Document::getFormat()
{
	return m_format;
}

/*
 *
 */
void Document::setCharset(const Glib::ustring &charset)
{
	m_charset = charset;
}

/*
 *	retourne le charset (encodage) du document
 *	ça permet de sauvegarder dans le meme encodage qu'a la lecture
 *	...
 */
Glib::ustring Document::getCharset()
{
	return m_charset;
}

/*
 *	"Unix", "Windows"
 */
void Document::setNewLine(const Glib::ustring &name)
{
	m_newline = name;
}

/*
 *	"Unix", "Windows"
 */
Glib::ustring Document::getNewLine()
{
	return m_newline;
}

/*
 *	efface les informations :
 *	 - script info
 *	 - subtitle model
 *	 - style model
 *	
 *	reinitialize le charset
 */
void Document::clear()
{
	Config &cfg = Config::getInstance();
	Glib::ustring encoding;

	if(cfg.get_value_string("encodings", "default", encoding))
		m_charset = encoding;
	else
		m_charset = "UTF-8";

	//m_format = ASS;
	
	m_scriptInfo = ScriptInfo();
	
	m_filename.clear();
	m_uriMovie.clear();
	
	m_subtitleModel->clear();
	m_styleModel->clear();
	//m_nameModel->clear();
}

/*
 *
 */
bool Document::open(const Glib::ustring &_filename)
{
	Glib::ustring uri = Glib::filename_to_uri(_filename);
	Glib::ustring charset = getCharset();

	SubtitleFormatSystem::instance().open(this, uri, charset);

	return true;
}



/*
 *	encodings ""=auto, "UTF-8", ...
 *	format "SubRip", "MicroDVD", "MPL2", ...
 *	rename, change le nom du document (filename)
 */
bool Document::save(const Glib::ustring &_filename)
{
	Glib::ustring filename = _filename;
	Glib::ustring basename = Glib::path_get_basename(filename);
	Glib::ustring format = getFormat();
	Glib::ustring charset = getCharset();
	Glib::ustring newline = getNewLine();

	try
	{
		SubtitleFormatSystem::instance().save(this, Glib::filename_to_uri(_filename), format, charset, newline);

		return true;
	}
	catch(const EncodingConvertError &ex)
	{
		Glib::ustring title = build_message(
					_("Could not save the file \"%s\" using the character coding %s."), 
					basename.c_str(), Encodings::get_label_from_charset(charset).c_str());
		Glib::ustring msg = _("The document contains one or more characters "
				"that cannot be encoded using the specified character coding.");

		ErrorDialog dialog(title, msg);
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		dialog.run();
	}
	catch(const std::exception &ex)
	{
		dialog_error(_("Save Document Failed."), ex.what());
	}
	return false;
}

/*
 *	retourne le model pour la gestion des sous-titres
 *	on passe par un model (gtk) pour eviter d'avoir des doublons 
 *	dans les données et donc une grosse consomation memoire (inutile)
 */
Glib::RefPtr<SubtitleModel> Document::get_subtitle_model()
{
	return m_subtitleModel;
}

/*
 *	retourne le model pour la gestion des styles
 */
Glib::RefPtr<StyleModel> Document::get_style_model()
{
	return m_styleModel;
}

/*
 *	retourne les informations sur le script 
 *	principalement pour SSA/ASS
 */
ScriptInfo& Document::get_script_info()
{
	return m_scriptInfo;
}

/*
 *
 */
SubtitleView* Document::get_subtitle_view()
{
	if(m_subtitleView == NULL)
		create_subtitle_view();

	return m_subtitleView;
}

/*
 *
 */
void Document::create_subtitle_view()
{
	se_debug(SE_DEBUG_APP);

	m_subtitleView = manage(new SubtitleView(*this));
	m_subtitleView->show();
}

/*
 *	affiche un message pour l'utilisateur sur ce document
 *	statusbar
 */
void Document::message(const gchar *format, ...)
{
	va_list args;
	gchar *formatted = NULL;

	va_start(args, format);
	formatted = g_strdup_vprintf(format, args);
	va_end(args);

	se_debug_message(SE_DEBUG_APP, formatted);

	m_signal_message(formatted);
	
	g_free(formatted);
}

/*
 *
 */
sigc::signal<void, Glib::ustring>& Document::get_signal_message()
{
	return m_signal_message;
}


/*
 *	affiche un message pour l'utilisateur sur ce document
 *	statusbar
 */
void Document::flash_message(const gchar *format, ...)
{
	va_list args;
	gchar *formatted = NULL;

	va_start(args, format);
	formatted = g_strdup_vprintf(format, args);
	va_end(args);

	se_debug_message(SE_DEBUG_APP, formatted);

	m_signal_flash_message(formatted);

	g_free(formatted);
}

/*
 *
 */
sigc::signal<void, Glib::ustring>& Document::get_signal_flash_message()
{
	return m_signal_flash_message;
}



/*
 * TODO
 *	uri :
 *		file:///home/toto/mysubtitle.srt
 *	name :
 *		mysubtitle.srt
 *	filename :
 *		/home/toto/mysubtitle.srt
 *	media-uri :
 *		audio or video for internal player
 */
void Document::set_data(const Glib::ustring &key, const Glib::ustring &value)
{
	m_data[key] = value;
}

/*
 *
 */
Glib::ustring Document::get_data(const Glib::ustring &key)
{
	std::map<Glib::ustring, Glib::ustring>::iterator it = m_data.find(key);

	if(it == m_data.end())
	{
		std::cerr << build_message("get_data failed:'%s'", key.c_str()) << std::endl;
		return "";
	}

	return it->second;
}

/*
 *
 */
bool Document::has_data(const Glib::ustring &key)
{
	std::map<Glib::ustring, Glib::ustring>::iterator it = m_data.find(key);

	if(it != m_data.end())
		return true;
	return false;
}


/*
 *
 */
Subtitles Document::subtitles()
{
	return m_subtitles;
}

/*
 *
 */
Styles Document::styles()
{
	return m_styles;
}

/*
 *	Command System
 */

/*
 *
 */
void Document::start_command(const Glib::ustring &description)
{
	CommandSystem::start(description);
}

/*
 *
 */
void Document::add_command(Command *cmd)
{
	CommandSystem::add(cmd);
}

/*
 *
 */
void Document::finish_command()
{
	if(CommandSystem::is_recording())
	{
		CommandSystem::finish();

		make_document_changed();
	}
}

/*
 *
 */
CommandSystem& Document::get_command_system()
{
	return *this;
}

/*
 *	the document has changed (start_command and finish_command are used)
 *	after save the document toggle state of false
 *	the signal "document-changed" is used after any change
 */
bool Document::get_document_changed()
{
	return m_document_changed;
}

/*
 *	turn m_document_changed to true and emit a signal "document-changed"
 */
void Document::make_document_changed()
{
	m_document_changed = true;

	emit_signal("document-changed");
}

/*
 *	turn m_document_changed to false and emit a signal "document-changed"
 */
void Document::make_document_unchanged()
{
	m_document_changed = false;

	emit_signal("document-changed");
}

/*
 *
 */
void Document::set_timing_mode(TIMING_MODE mode)
{
	m_timing_mode = mode;

	emit_signal("timing-mode-changed");
}
	
/*
 *
 */
TIMING_MODE Document::get_timing_mode()
{
	return m_timing_mode;
}

/*
 *
 */
void Document::set_edit_timing_mode(TIMING_MODE mode)
{
	m_edit_timing_mode = mode;

	for(Subtitle sub = subtitles().get_first(); sub; ++sub)
	{
		sub.update_view_mode_timing();
	}
	emit_signal("edit-timing-mode-changed");
}
	
/*
 *
 */
TIMING_MODE Document::get_edit_timing_mode()
{
	return m_edit_timing_mode;
}

/*
 *
 */
void Document::set_framerate(FRAMERATE framerate)
{
	m_framerate = framerate;

	for(Subtitle sub = subtitles().get_first(); sub; ++sub)
	{
		sub.update_view_mode_timing();
	}
	emit_signal("framerate-changed");
}

/*
 *
 */
FRAMERATE Document::get_framerate()
{
	return m_framerate;
}


/*
 *
 */
Document* Document::create_from_file(const Glib::ustring &uri, const Glib::ustring &charset)
{
	se_debug_message(SE_DEBUG_APP, "uri=%s charset=%s", uri.c_str(), charset.c_str());

	Glib::ustring filename = Glib::filename_from_uri(uri);
	Glib::ustring basename = Glib::path_get_basename(filename);
	Glib::ustring label_charset = Encodings::get_label_from_charset(charset);

	try
	{
		Document *doc = new Document;
		doc->setCharset(charset);

		if(doc->open(filename))
			return doc;

		delete doc;
		return NULL;// throw ?
	}
	catch(const UnrecognizeFormatError &ex)
	{
		Glib::ustring title = build_message(
				_("Could not recognize the subtitle format for the file \"%s\"."), 
				basename.c_str());
		Glib::ustring msg = _("Please check that the file contains subtitles in a supported format.");

		ErrorDialog dialog(title, msg);
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		dialog.run();
	}
	catch(const EncodingConvertError &ex)
	{
		Glib::ustring title, msg;

		if(charset.empty())
		{
			title = build_message(_("Could not open automatically the file \"%s\"."), basename.c_str());
			msg = _("Subtitle Editor was not able to automatically determine the file encoding. "
					"Select a different character coding from the menu and try again.");
		}
		else
		{
			title = build_message(
					_("Could not open the file \"%s\" using the character coding %s."), 
					basename.c_str(), Encodings::get_label_from_charset(charset).c_str());
			msg = _("Select a different character coding from the menu and try again.");
		}
		
		ErrorDialog dialog(title, msg);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
		
		Gtk::Label labelEncoding(_("Character Coding:"), 1.0, 0.5);
		ComboBoxEncoding comboEncoding(false);

		Gtk::HBox hbox(false, 6);
		dialog.get_vbox()->pack_start(hbox, false, false);
		hbox.pack_start(labelEncoding);
		hbox.pack_start(comboEncoding);

		dialog.show_all();
		if(dialog.run() == Gtk::RESPONSE_OK)
		{
			dialog.hide();

			return Document::create_from_file(uri, comboEncoding.get_value());
		}
	}
	catch(const std::exception &ex)
	{
		Glib::ustring title = build_message(_("Could not open the file \"%s\""), basename.c_str());
		Glib::ustring msg = ex.what();

		ErrorDialog dialog(title, msg);
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		dialog.run();
	}
	catch(...)
	{
		Glib::ustring title = build_message(_("Could not open the file \"%s\""), basename.c_str());
		Glib::ustring msg = _("An unknown error occurred while opening the file.");

		ErrorDialog dialog(title, msg);
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
		dialog.run();
	}
	return NULL;
}

/*
 *
 */
sigc::signal<void>& Document::get_signal(const std::string &name)
{
	return m_signal[name];
}

/*
 *
 */
void Document::emit_signal(const std::string &name)
{
	m_signal[name].emit();
}

