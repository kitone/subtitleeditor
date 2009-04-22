#ifndef _isocodes_h
#define _isocodes_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2009, kitone
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

#include <glibmm/ustring.h>

namespace isocodes {

/*
 * Convert ISO 639 code to localized language name.
 * ex: "fr" to "French"
 */
Glib::ustring to_language(const Glib::ustring &code);

/*
 * Convert ISO 3166 code to localized country name.
 * ex: "FR" to "France"
 */
Glib::ustring to_country(const Glib::ustring &code);

/*
 * Convert ISO 15924 code to localized country name.
 * ex: "Latn" to "Latin"
 */
Glib::ustring to_script(const Glib::ustring &code);

/*
 * Convert from ISO XXX to good localized name:
 * ex: "fr_FR" to "French (France)", "US" to "United States"...
 */
Glib::ustring to_name(const Glib::ustring &code);

}//namespace isocodes

#endif//_isocodes_h
