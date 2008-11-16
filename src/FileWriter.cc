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

#include "FileWriter.h"
#include <fstream>
#include "Error.h"
#include "utility.h"
#include "Encodings.h"

/*
 *
 */
FileWriter::FileWriter(const Glib::ustring &uri, const Glib::ustring &charset, const Glib::ustring &newline)
{
	m_uri = uri;
	m_charset = charset;
	m_newline = newline;
}

/*
 * Write to the file.
 */
void FileWriter::to_file()
{
	// Convert newline if needs
	if(m_newline != "Unix")
		m_data = Glib::Regex::create("\n")->replace(m_data, 0, (m_newline == "Windows") ? "\r\n": "\r", (Glib::RegexMatchFlags)0);

	std::string data = Encoding::convert_from_utf8_to_charset(m_data, m_charset); 

	std::ofstream file(Glib::filename_from_uri(m_uri).c_str());
	if(!file)
		throw IOFileError(_("Couldn't open the file."));

	file << data;
	file.close();
}

/*
 *
 */
void FileWriter::write(const Glib::ustring &buf)
{
	m_data += buf;
}
