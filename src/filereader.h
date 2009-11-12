#ifndef _FileReader_h
#define _FileReader_h

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

#include <sstream>
#include <string>
#include <glibmm.h>

/*
 * Helper to read file.
 * Can automatically detect the character coding and convert to UTF-8.
 * Detect the newline type.
 * Return lines without character of newline (CR,LF or CRLF)
 */
class FileReader
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
	 * Return the contents of the file.
	 */
	const Glib::ustring& get_data() const;
	
	/*
	 * Return the charset of the file.
	 */
	Glib::ustring get_charset() const;

	/*
	 * Return the newline detected of the file.
	 */
	Glib::ustring get_newline();

	/*
	 * Get the next line of the file without newline character (CR, LF or CRLF).
	 */
	bool getline(Glib::ustring &line);

	/*
	 * Return all lines detected of the file, without newline character (CR, LF or CRLF).
	 */
	std::vector<Glib::ustring> get_lines();

private:

	/*
	 * Split the data to separate lines.
	 */
	void initialize_lines();

protected:
	Glib::ustring m_uri;
	Glib::ustring m_data;
	Glib::ustring m_charset;
	bool m_lines_init;
	std::vector<Glib::ustring>::const_iterator m_iter;
	std::vector<Glib::ustring> m_lines;
};

#endif//_FileReader_h

