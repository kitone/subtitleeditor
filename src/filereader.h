#ifndef _FileReader_h
#define _FileReader_h

/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2013, kitone
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

#include <sstream>
#include <string>
#include <glibmm.h>
#include "reader.h"

/*
 * Helper to read file.
 * Can automatically detect the character coding and convert to UTF-8.
 * Detect the newline type.
 * Return lines without character of newline (CR,LF or CRLF)
 */
class FileReader : public Reader
{
public:

	/*
	 * Constructor.
	 *
	 * Open the file from an uri and convert the contents from charset to UTF-8.
	 * If charset is empty, try to autodetect the character coding.
	 *
	 * Error: throw an IOFileError exception if failed.
	 */
	FileReader(const Glib::ustring &uri, const Glib::ustring &charset, int max_data_size = -1);

	/*
	 * Return the uri of the file.
	 */
	Glib::ustring get_uri() const;

	/*
	 * Return the charset of the file.
	 */
	Glib::ustring get_charset() const;


protected:
/*
	bool get_contents_from_file(
					const Glib::ustring &uri, 
					const Glib::ustring &charset, 
					Glib::ustring &utf8_contents, 
					Glib::ustring &charset_contents, 
					int max_data_size);
*/
	Glib::ustring m_uri;
	Glib::ustring m_charset;

};

#endif//_FileReader_h

