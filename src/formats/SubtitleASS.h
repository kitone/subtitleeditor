#ifndef _SubtitleASS_h
#define _SubtitleASS_h

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
#include "SubtitleFormat.h"


class SubtitleASS : public SubtitleFormat
{
public:
	SubtitleASS(Document *doc);
	~SubtitleASS();

	/*
	 *
	 */
	bool on_open(const Glib::ustring &filename);

	/*
	 *
	 */
	bool on_save(const Glib::ustring &filename);

	/*
	 *
	 */
	static Glib::ustring get_name();
	
	/*
	 *
	 */
	static bool check(const std::string &line);

	/*
	 *
	 */
	static Glib::ustring get_extension();
	
protected:

	/*
	 *	READ BLOCK
	 */
	
	/*
	 *	lecture du block [ScriptInfo]
	 */
	bool readScripInfo(const std::string &line);

	/*
	 *	lecture du block [V4+ Styles]
	 */
	bool readStyles(const std::string &line);

	/*
	 *	lecture du block [Events]
	 */
	bool readEvents(const std::string &line);


protected:

	/*
	 *	convertir le temps utiliser par subtitle editor en temps valide pour le format ASS
	 *	0:00:00.000 -> 0:00:00.00
	 */
	Glib::ustring subtitletime_to_ass_time(const SubtitleTime &time);

	/*
	 *	convertir un temps ASS en SubtitleTime (string)
	 */
	Glib::ustring ass_time_to_subtitletime(const Glib::ustring &time);
	
	/*
	 *	convertir une couleur en couleur ASS pour la sauvegarde
	 */
	Glib::ustring color_to_ass_color(const Glib::ustring& color);

	/*
	 *	convertir une couleur ASS en Color (interne)
	 */
	Color ass_color_to_color(const Glib::ustring &str);

	/*
	 *	-1=true 
	 *	 0=false
	 */
	std::string bool_to_string(const Glib::ustring &val);

	/*
	 *
	 */
	bool string_to_bool(const std::string &val);

	/*
	 *
	 */
	int string_to_int(const std::string &val);

	/*
	 *	
	 */
	std::vector< std::string > build(const std::string &line, unsigned int column);


	/*
	 *
	 */
	std::vector<std::string> build_format(const std::string &text, int column=-1, bool remove_space=false);
	
	
	/*
	 *
	 */
	Glib::ustring clean_style_name(const Glib::ustring &name);

protected:

	std::vector<std::string> formats;
	std::map<std::string, unsigned int> map; 
	
};

#endif//_SubtitleASS_h

