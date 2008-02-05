#ifndef _RegEx_h
#define _RegEx_h

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
 

#include <glibmm/ustring.h>
#include <pcre.h>
#include <pcrecpp.h>


class RegEx : public pcrecpp::RE
{
public:
	enum Options
	{
		UTF8 = PCRE_UTF8,
		CASELESS = PCRE_CASELESS,
		MULTILINE = PCRE_MULTILINE
	};

	RegEx(const std::string &pattern, int flags=UTF8|MULTILINE);
	~RegEx();

	/*
	 *
	 */
	bool exec(const Glib::ustring &string);

	/*
	 *
	 */
	bool exec(const Glib::ustring &string, Glib::ustring::size_type &start, Glib::ustring::size_type &len);
private:
	int m_options;
};


#endif//_RegEx_h

