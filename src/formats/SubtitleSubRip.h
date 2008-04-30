#ifndef _SubtitleSubRip_h
#define _SubtitleSubRip_h

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

class SubtitleSubRip : public SubtitleFormat
{
public:
	SubtitleSubRip(Document* doc);
	~SubtitleSubRip();

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
	 *	convertir le temps interne (SubtitleTime) 
	 *	en format SubRip
	 */
	Glib::ustring subtitletime_to_subrip_time(const Glib::ustring &time);

};

#endif//_SubtitleSubRip_h