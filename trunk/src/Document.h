#ifndef _Document_h
#define _Document_h

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
 

//#include "SubtitleModel.h"
#include <string>
#include <map>
#include <sigc++/sigc++.h>
#include "StyleModel.h"
#include "ScriptInfo.h"
#include "Subtitles.h"
#include "Styles.h"
#include "SubtitleView.h"
#include "CommandSystem.h"
#include "TimeUtility.h"


typedef Glib::RefPtr<SubtitleModel> SubtitleModelPtr;
typedef SubtitleView* SubtitleViewPtr;
typedef std::list<Document*> DocumentList;

/*
 *	gestion des signaux
 *
 *	"document-property-changed" (filename, format...)
 *	"subtitle-selection-changed"
 *	"subtitle-time-changed"	
 */

class Document : protected CommandSystem
{
public:

	/*
	 *
	 */
	static Document* create_from_file(const Glib::ustring &uri, const Glib::ustring &charset = Glib::ustring());
	
	/*
	 *	constructeur
	 *	s'il y a une option dans le fichier config [encodings] default=xxxx
	 *	on l'utilise pour choisir le charset, sinon on utilise UTF-8
	 */
	Document();

	/*
	 *	constructeur par copy
	 */
	Document(Document &copy);

	/*
	 *	destructeur
	 */
	~Document();

	/*
	 *	efface les informations :
	 *	 - script info
	 *	 - subtitle model
	 *	 - style model
	 *	
	 *	reinitialize le charset
	 */
	void clear();

	/*
	 *
	 */
	void setFormat(const Glib::ustring &format);

	/*
	 *	SubRip, MicroDVD, ...
	 */
	Glib::ustring getFormat();
	
	/*
	 *
	 */
	void setCharset(const Glib::ustring &charset);

	/*
	 *	retourne le charset (encodage) du document
	 *	ça permet de sauvegarder dans le meme encodage qu'a la lecture
	 *	...
	 */
	Glib::ustring getCharset();

	/*
	 *	"Unix", "Windows"
	 */
	void setNewLine(const Glib::ustring &name);

	/*
	 *	"Unix", "Windows"
	 */
	Glib::ustring getNewLine();

	/*
	 *	le nom complet du document 
	 *	ex: /home/toto/subtitle05.ass
	 */
	void setFilename(const Glib::ustring &filename);

	/*
	 *	le nom complet du document 
	 *	ex: /home/toto/subtitle05.ass
	 */
	Glib::ustring getFilename();

	/*
	 *	le nom du document
	 *	si filename = /home/toto/subtitle05.ass
	 *	name = subtitle05.ass
	 */
	void setName(const Glib::ustring &name);

	/*
	 *	le nom du document
	 *	si filename = /home/toto/subtitle05.ass
	 *	name = subtitle05.ass
	 */
	Glib::ustring getName();



	/*
	 *	encodings ""=auto, "UTF-8", ...
	 *	clear permet d'effacer l'ancien model, sinon on ajoute a la suite du document
	 *
	 *	Used Document::create_from_file.
	 */
	bool open(const Glib::ustring &filename);

	/*
	 *	format "SubRip", "MicroDVD", "MPL2", ...
	 *	encodings ""=auto, "UTF-8", ...
	 *	rename, change le nom du document (filenameDocument)
	 */
	bool save(const Glib::ustring &filename);



	/*
	 *	retourne le model pour la gestion des styles
	 */
	Glib::RefPtr<StyleModel> get_style_model();

	/*
	 *	retourne les informations sur le script 
	 *	principalement pour SSA/ASS
	 */
	ScriptInfo& get_script_info();


	/*
	 *	affiche un message pour l'utilisateur sur ce document
	 *	statusbar
	 */
	void message(const gchar *format, ...);

	/*
	 *
	 */
	sigc::signal<void, Glib::ustring>& get_signal_message();

	/*
	 *	affiche un message pendant 3 sec 
	 *	statusbar
	 */
	void flash_message(const gchar *format, ...);

	/*
	 *
	 */
	sigc::signal<void, Glib::ustring>& get_signal_flash_message();
	

	/*
	 *	Data system
	 */
	
	/*
	 *
	 */
	void set_data(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 *
	 */
	Glib::ustring get_data(const Glib::ustring &key);

	/*
	 *
	 */
	bool has_data(const Glib::ustring &key);

	/*
	 *
	 */
	Subtitles subtitles();

	/*
	 *
	 */
	Styles styles();


	/*
	 *	the document has changed (start_command and finish_command are used)
	 *	after save the document toggle state of false
	 *	the signal "document-changed" is used after any change
	 */
	bool get_document_changed();

	/*
	 *	turn m_document_changed to true and emit a signal "document-changed"
	 */
	void make_document_changed();

	/*
	 *	turn m_document_changed to false and emit a signal "document-changed"
	 */
	void make_document_unchanged();

	/*
	 *	Command System (Undo/Redo)
	 *	start_command(_("XXX"));
	 *		change subtitle...
	 *	finish_command();
	 */
	void start_command(const Glib::ustring &description);
	void add_command(Command *cmd);
	void finish_command();

	/*
	 *
	 */
	CommandSystem& get_command_system();
	
	/*
	 *
	 */
	Gtk::Widget* widget();

	/*
	 *
	 */
	void set_timing_mode(TIMING_MODE mode);
	
	/*
	 *
	 */
	TIMING_MODE get_timing_mode();

	/*
	 *
	 */
	void set_edit_timing_mode(TIMING_MODE mode);

	/*
	 *
	 */
	TIMING_MODE get_edit_timing_mode();

	/*
	 *
	 */
	void set_framerate(FRAMERATE framerate);

	/*
	 *
	 */
	FRAMERATE get_framerate();

	/*
	 *
	 */
	sigc::signal<void>& get_signal(const std::string &name);

	/*
 	 *
	 */
	void emit_signal(const std::string &name);

protected:
	friend class Command;
	friend class Subtitle;
	friend class Subtitles;
	friend class SubtitleView;

	/*
	 *	retourne le model pour la gestion des sous-titres
	 *	on passe par un model (gtk) pour eviter d'avoir des doublons 
	 *	dans les données et donc une grosse consomation memoire (inutile)
	 */
	SubtitleModelPtr get_subtitle_model();

	/*
	 *
	 */
	SubtitleViewPtr get_subtitle_view();

	/*
	 *
	 */
	void create_subtitle_view();
public:
	Glib::ustring									m_format;
	std::string										m_charset;
	Glib::ustring									m_newline;
	ScriptInfo										m_scriptInfo;
	Glib::RefPtr<StyleModel>			m_styleModel;
	//Glib::RefPtr<NameModel>			m_nameModel;

	// filename movie, ...
	//Glib::ustring filenameDocument;
	Glib::ustring m_uriMovie;

	TIMING_MODE m_timing_mode;
	TIMING_MODE m_edit_timing_mode;
	FRAMERATE		m_framerate;
protected:
	Glib::ustring m_name;		// ex: toto.ass
	Glib::ustring m_filename;	// /home/titi/toto.ass

	//std::map<std::string, sigc::signal<void> >	m_signal; // In Signal class

	sigc::signal<void, Glib::ustring> m_signal_message;
	sigc::signal<void, Glib::ustring> m_signal_flash_message;

	std::map<Glib::ustring, Glib::ustring> m_data;

	Subtitles	m_subtitles;
	Styles		m_styles;

	bool m_document_changed;

	SubtitleView*									m_subtitleView;
	Glib::RefPtr<SubtitleModel>		m_subtitleModel;

	std::map< std::string, sigc::signal<void> > m_signal;
};

#endif//_Document_h

