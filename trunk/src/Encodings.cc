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


#include "Encodings.h"

/*
 *
 */
bool Encodings::is_initialized = false;

/*
 *
 */
bool Encodings::initialize()
{
	if(is_initialized)
		return true;

	for(unsigned int i=0; encodings_info[i].name != NULL; ++i)
	{
		encodings_info[i].name = _(encodings_info[i].name);
	}

	is_initialized = true;
	return true;
}

/*
 *
 */
EncodingInfo* Encodings::get_from_charset(const Glib::ustring &charset)
{
	initialize();
	
	for(unsigned int i=0; encodings_info[i].name != NULL; ++i)
	{
		if(charset == encodings_info[i].charset)
			return &encodings_info[i];
	}
	return NULL;
}

/*
 * Return a human readable string or empty string, ex:
 * "name (charset)"
 * "Unicode (UTF-8)"
 */
Glib::ustring Encodings::get_label_from_charset(const Glib::ustring &charset)
{
	EncodingInfo *info = get_from_charset(charset);
	if(info == NULL)
		return Glib::ustring();

	Glib::ustring label;

	label += info->name;
	label += " (";
	label += info->charset;
	label += ")";

	return label;
}

