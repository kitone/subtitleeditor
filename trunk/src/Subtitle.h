#ifndef _Subtitle_h
#define _Subtitle_h

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
 

#include "SubtitleModel.h"
//#include "Document.h"

/*
 *	Cette class fonctionne avec un iterator (SubtitleModel).
 *	Elle permet de modifier les attributs de l'iter
 *	il est possible de modifier directement l'iter mais
 *	dans le cas ou on modifie par exemple le temps "end"
 *	il faut aussi modifier le temps "duration", etc...
 *
 *	Passer par cette class evite les oublies de ce genre.
 *
 *  /!\	
 *  La durée de vie d'un Subtitle est la même que l'iter!
 *  On ne verifie pas la validiter des arguments! (pour les performances)
 */
class SubtitleCommand;
class Subtitles;
class Document;

class Subtitle
{
	friend class Subtitles;
	friend class SubtitleCommand;
public:
	Subtitle();
	Subtitle(Document* doc, const Glib::ustring &path);
	Subtitle(Document* doc, const Gtk::TreeIter &iter);
	~Subtitle();

	/*
	 * valide ?
	 */
	operator bool() const;

	/*
	 *
	 */
	Subtitle&	operator++();

	/*
	 *
	 */
	Subtitle&	operator--();

	/*
	 *
	 */
	bool operator==(const Subtitle &sub) const;
	bool operator!=(const Subtitle &sub) const;

	/*
	 *
	 */
	void set_num(unsigned int num);

	/*
	 *
	 */
	unsigned int get_num() const;


	/*
	 *
	 */
	void set_layer(const Glib::ustring &layer);

	/*
	 *
	 */
	Glib::ustring get_layer() const;

	/*
	 *	petite optimisation qui permet de calculer 
	 *	qu'une seule fois duration
	 */
	void set_start_and_end(const SubtitleTime &start, const SubtitleTime &end);

	/*
	 *
	 */
	void set_start(const Glib::ustring &time);
	void set_start(const SubtitleTime &time);

	/*
	 *
	 */
	SubtitleTime get_start() const;


	/*
	 *
	 */
	void set_end(const Glib::ustring &time);
	void set_end(const SubtitleTime &time);

	/*
	 *
	 */
	SubtitleTime get_end() const;


	/*
	 *
	 */
	void set_duration(const Glib::ustring &time);
	void set_duration(const SubtitleTime &time);

	/*
	 *
	 */
	SubtitleTime get_duration() const;


	/*
	 *
	 */
	void set_style(const Glib::ustring &style);

	/*
	 *
	 */
	Glib::ustring get_style() const;


	/*
	 *
	 */
	void set_name(const Glib::ustring &name);

	/*
	 *
	 */
	Glib::ustring get_name() const;
	
	
	/*
	 *
	 */
	void set_margin_l(const Glib::ustring &value);
	void set_margin_r(const Glib::ustring &value);
	void set_margin_v(const Glib::ustring &value);

	/*
	 *
	 */
	Glib::ustring get_margin_l() const;
	Glib::ustring get_margin_r() const;
	Glib::ustring get_margin_v() const;


	/*
	 *
	 */
	void set_effect(const Glib::ustring &effect);

	/*
	 *
	 */
	Glib::ustring get_effect() const;


	/*
	 *
	 */
	void set_text(const Glib::ustring &text);

	/*
	 *
	 */
	Glib::ustring get_text() const;


	/*
	 *
	 */
	void set_translation(const Glib::ustring &text);

	/*
	 *
	 */
	Glib::ustring get_translation() const;

	/*
	 *	ex: 6 or 3\n3
	 */
	Glib::ustring get_characters_per_line_text() const;


	/*
	 *	ex: 6 or 3\n3
	 */
	Glib::ustring get_characters_per_line_translation() const;

	/*
	 *
	 */
	void set_note(const Glib::ustring &text);

	/*
	 *
	 */
	Glib::ustring get_note() const;


	/*
	 *	copie le s-t dans sub
	 */
	void copy_to(Subtitle &sub);

	/*
	 *
	 */
	void set(const Glib::ustring &name, const Glib::ustring &value);

	/*
	 *
	 */
	Glib::ustring get(const Glib::ustring &name) const;

	/*
	 *
	 */
	void set(const std::map<Glib::ustring, Glib::ustring> &values);

	/*
	 *
	 */
	void get(std::map<Glib::ustring, Glib::ustring> &values);

protected:

	/*
	 *
	 */
	void push_command(const Glib::ustring &name, const Glib::ustring &value);

	/*
	 *
	 */
	void set_characters_per_second_text(const Glib::ustring &cps);

	/*
	 *
	 */
	void update_characters_per_sec();
protected:
	static SubtitleColumnRecorder column;
	Document *m_document;
	Gtk::TreeIter m_iter;
	Glib::ustring m_path;
	
};


#endif//_Subtitle_h
