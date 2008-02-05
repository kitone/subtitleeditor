#ifndef _SubtitleSSA_h
#define _SubtitleSSA_h

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
#include <iostream>
#include <fstream>
#include <string>
#include "Document.h"
#include "SubtitleFormat.h"



/*
 *	num | flag | start | end | (?lenght) | style | name | text	 |||? gauche|droite|vertical
 */
class SubtitleSSA : public SubtitleFormat
{
public:
	SubtitleSSA(Document* doc);
	~SubtitleSSA();

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
	 *	convertir le temps utiliser par subtitle editor
	 *	en temps valide pour le format SSA
	 *	0:00:00.000 -> 0:00:00.00
	 */
	Glib::ustring subtitletime_to_ssa_time(const SubtitleTime &time);

	/*
	 *	convertir le temps SSA en SubtitleTime (string)
	 *	0:00:00.00 -> 0:00:00.000
	 */
	Glib::ustring ass_time_to_subtitletime(const Glib::ustring &time);
	

	/*
	 *	convertir une couleur en format SSA
	 */
	Glib::ustring color_to_ssa_color(const Glib::ustring &color);
	
	/*
	 *	convertir une couleur SSA en Color (interne)
	 */
	Color ssa_color_to_color(const unsigned int &ssa_color);


	/*
	 *	
	 */
	std::vector< std::string > build(const std::string &line, unsigned int column);


	/*
	 *	retire '*' du style
	 */
	Glib::ustring clean_style_name(const Glib::ustring &name);

	/*
	 *	-1=true 
	 *	 0=false
	 */
	std::string bool_to_string(const bool &val);

	/*
	 *
	 */
	bool string_to_bool(const std::string &val);

	/*
	 *
	 */
	int string_to_int(const std::string &val);

protected:
	std::vector<std::string> formats;
	std::map<std::string, unsigned int> map; 
	
	// alignment utiliser par SSA
	// 5	-  6 -	7
	// 9	- 10 - 11
	// 1	-  2 -	3
	std::map<unsigned int, unsigned int> alignment_map;
};

#endif//_SubtitleSSA_h
