#ifndef _SubtitleSystem_h
#define _SubtitleSystem_h

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
#include <list>
#include "Document.h"
#include "SubtitleFormat.h"


class SubtitleSystem
{
public:
	static SubtitleSystem& getInstance();

	/*
	 *	retourne la list des formats supporter
	 */
	std::list<Glib::ustring> get_formats();

	/*
	 * Check if the format is supported.
	 */
	bool is_supported(const Glib::ustring &format);

	/*
	 *	determine quel est le format du sous-titre
	 */
	Glib::ustring find_subtitle_format(const Glib::ustring &filename);

	/*
	 *	retourne l'extension utiliser par le format
	 *	ex: "ass", "ssa", "srt", ...
	 */
	Glib::ustring get_extension(const Glib::ustring &format);

	/*
	 *	crée la class du format
	 */
	SubtitleFormat* create_subtitle_format(const Glib::ustring &name, Document *doc);
protected:
	/*
	 *	cherche le format de la line str
	 */
	Glib::ustring	find_format(const char *str);
};

#endif//_SubtitleSystem_h
