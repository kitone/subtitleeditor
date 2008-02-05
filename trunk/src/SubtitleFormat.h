#ifndef _SubtitleFormat_h
#define _SubtitleFormat_h

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
 

#include <glibmm.h>
#include "Document.h"
#include <exception>

/*
 *
 */
class SubtitleException : public std::exception
{
public:
	SubtitleException(const char *classname, const char *msg)
	:m_classname(classname), m_msg(msg)
	{
	}

	virtual ~SubtitleException() throw()
	{
	}

	const char* classname() const throw()
	{
		return m_classname.c_str();
	}

	virtual const char* what() const throw()
	{
		return m_msg.c_str();
	}

private:
	std::string m_classname;
	std::string m_msg;
};


/*
 *
 */
class SubtitleFormat
{
public:

	/*
	 *	charset,newline sont init avec les info du doc
	 *	il est possible de modifier les valeurs avant
	 *	la lecture ou sauvegarde
	 *
	 *	ex:
	 *	doc: charset=ISO-8859-15 & newline=Windows
	 *	
	 *	SubtitleFormat sf(doc);
	 *
	 *	sf.set_charset("UTF-8");
	 *	sf.set_newline("Unix");
	 *	sf.save("/home/toto/sub.srt", true);
	 *
	 *	doc: charset=UTF-8 & newline=Unix
	 */
	SubtitleFormat(Document* doc, const Glib::ustring &format_name);

	/*
	 *
	 */
	virtual ~SubtitleFormat();


	/*
	 *	"UTF-8", "ISO-8859-15" ...
	 *	"" utilise l'auto detection
	 */
	void set_charset(const std::string &charset);

	/*
	 *	"UTF-8", "ISO-8859-15" ...
	 *	
	 */
	std::string get_charset();

	/*
	 *	os = "Unix" ou "Windows"
	 */
	void set_newline(const Glib::ustring &os);
	
	/*
	 *	retour le '\n' ou '\r\n' selon la config du doc
	 *	"Unix", "Windows"
	 */
	std::string get_newline();
	
	/*
	 *	retourne le nom du format
	 *	ex: "SubRip", "MicroDVD"...
	 */
	Glib::ustring get_format_name();

	//virtual bool open(const Glib::ustring &filename, const Glib::ustring &encodings) { return false; }
	//virtual bool save(const Glib::ustring &filename, const Glib::ustring &encodings) { return false; }

	/*
	 *
	 */
	bool open(const Glib::ustring &filename);
	
	/*
	 *
	 */
	bool save(const Glib::ustring &filename);

	/*
	 *
	 */
	virtual bool on_open(const Glib::ustring &filename) = 0;

	/*
	 *
	 */
	virtual bool on_save(const Glib::ustring &filename) = 0;

	/*
	 *
	 */
	Document* document();

protected:

	/*
	 *	convertir text en utf8
	 *	le charset du text est defini par m_charset
	 */
	Glib::ustring charset_to_utf8(const std::string &text);

	/*
	 *	convertir le text (utf-8) en codage defini par m_charset
	 */
	std::string utf8_to_charset(const Glib::ustring &text);


	/*
	 *	remplace la fin de ligne '\n' par "str"
	 */
	void newline_to_characters(Glib::ustring &text, const Glib::ustring &str);

	/*
	 *	remplace "characters" par '\n'
	 */
	void characters_to_newline(Glib::ustring &text, const Glib::ustring &characters);


protected:
	/*
	 *
	 */
	Glib::ustring m_format_name;

	/*
	 *	represente le charset utiliser pour la lecture (charset -> utf8) 
	 *	ou la sauvegarde (utf8 -> charset)
	 */
	std::string	m_charset;

	/*
	 *
	 */
	std::string	m_newline;

	/*
	 *
	 */
	Document* m_document;

	/*
	 *
	 */
	SubtitleColumnRecorder	m_column;

	/*
	 *
	 */
	ScriptInfo*	m_scriptInfo;

	/*
	 *
	 */
	Glib::RefPtr<StyleModel> m_styleModel;

private:
	std::list<Glib::ustring> m_list_encodings;
	bool	m_use_auto_deteced;
};


#endif//_SubtitleFormat_h
