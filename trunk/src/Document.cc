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
 

#include "Document.h"
#include "Config.h"
#include <iostream>
#include "utility.h"

#include "SubtitleSystem.h"

#include <ctime>
#include <memory>

/*
 *	constructeur
 *	s'il y a une option dans le fichier config [encodings] default=xxxx
 *	on l'utilise pour choisir le charset, sinon on utilise UTF-8
 */
Document::Document()
:CommandSystem(*this), m_subtitles(*this), m_styles(*this), m_subtitleView(NULL)
{
	m_document_changed = false;

	Config &cfg = Config::getInstance();
	Glib::ustring encoding;
	
	if(cfg.get_value_string("encodings", "default", encoding))
		m_charset = encoding;
	else
		m_charset = "UTF-8";
#warning "Fixme > Use Config Option"
	m_format = "SubRip";

	m_newline = "Unix";

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
#define DIALOG_ERROR(ex, filename, charset) \
	if(charset.empty())	{ \
			dialog_error( \
					build_message( \
						_("Could not open the file <i>%s</i>."), \
						filename.c_str()), \
					ex.what()); } \
	else { \
			dialog_error( \
					build_message( \
						_("Could not open the file <i>%s</i> using the <i>%s</i> character coding."), \
						filename.c_str(), charset.c_str()), \
					ex.what());	}

	Glib::ustring filename = _filename;
	Glib::ustring charset = getCharset();

	try
	{
		Glib::ustring format = SubtitleSystem::getInstance().find_subtitle_format(filename);

		if(!format.empty())
		{
			std::auto_ptr<SubtitleFormat> sf(SubtitleSystem::getInstance().create_subtitle_format(format, this));

			if(sf.get() != NULL) // need ?
			{
				get_subtitle_view()->unset_model();
			
				bool res = sf->open(filename);

				get_subtitle_view()->set_model(get_subtitle_model());

				if(res)
				{
					setFilename(filename);
					setCharset(sf->get_charset());
					setFormat(format);

					emit_signal("document-changed");
					emit_signal("document-property-changed");
				}

				return res;
			}
		}
	}
	catch(const SubtitleException &ex)
	{
		DIALOG_ERROR(ex, filename, charset);
	}
	catch(const Glib::Error &ex)
	{
		DIALOG_ERROR(ex, filename, charset);
	}
	catch(const std::exception &ex)
	{
		DIALOG_ERROR(ex, filename, charset);
	}

#undef DIALOG_ERROR
	return false;
}



/*
 *	encodings ""=auto, "UTF-8", ...
 *	format "SubRip", "MicroDVD", "MPL2", ...
 *	rename, change le nom du document (filename)
 */
bool Document::save(const Glib::ustring &_filename)
{
	try
	{
		Glib::ustring filename = _filename;
		Glib::ustring format = getFormat();
		Glib::ustring charset = getCharset();

		std::auto_ptr<SubtitleFormat> sf(SubtitleSystem::getInstance().create_subtitle_format(format, this));
		
		if(sf.get() != NULL) // need ?
		{
			bool res = sf->save(filename);

			if(res)
			{
				setCharset(charset);
				setFilename(filename);
				setFormat(format);

				make_document_unchanged();
				emit_signal("document-property-changed");
			}

			return res;
		}
	}
	catch(const SubtitleException &ex)
	{
		dialog_error(_("Save Document Failed."), ex.what());
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

