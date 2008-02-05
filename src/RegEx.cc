#include "utility.h"
#include "RegEx.h"

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
 

#ifdef USE_PCRE

/*
 *
 */
RegEx::RegEx(const Glib::ustring &ex, int flags)
:m_pcre(NULL)
{
	se_debug_message(SE_DEBUG_REGEX, "expression=%s", ex.c_str());

	const char *error_ptr = NULL;
	int error_offset = 0;
	int error_code = 0;

	int options = PCRE_UTF8 | PCRE_MULTILINE;

	if(flags & CASELESS)
		options |= PCRE_CASELESS;

	m_pcre = pcre_compile2(ex.c_str(), options, 
			&error_code, &error_ptr, &error_offset, NULL);

	if(m_pcre == NULL || error_code != 0)
	{
		std::cerr << "Error: " << error_ptr << std::endl; 
		//return msg error_code
	}
}

/*
 *
 */
RegEx::~RegEx()
{
	se_debug(SE_DEBUG_REGEX);

	pcre_free(m_pcre);
}

/*
 *
 */
bool RegEx::exec(const Glib::ustring &string)
{
	se_debug_message(SE_DEBUG_REGEX, "string=%s", string.c_str());

	if(m_pcre == NULL)
		return false;

	int options = PCRE_ANCHORED;

	int rc = pcre_exec(
			(const pcre *)m_pcre,
			NULL,	
			string.c_str(), (int)string.bytes(),
			0, 
			options, 
			NULL, 0);

	if(rc >= 0)
		return true;

	return false;
}

/*
 *
 */
bool RegEx::exec(const Glib::ustring &string, Glib::ustring::size_type &start, Glib::ustring::size_type &len)
{
	se_debug_message(SE_DEBUG_REGEX, "string=%s", string.c_str());

	if(m_pcre == NULL)
		return false;

	int options = 0;

	int ovector[2];

	int rc = pcre_exec(
			(const pcre *)m_pcre,
			NULL,
			string.c_str(), (int)string.bytes(),
			0,
			options, 
			ovector, 2);

	if(rc >= 0)
	{
		std::string tmp = string;

		start = Glib::ustring( tmp.substr(0, ovector[0]) ).size();
		len = Glib::ustring( tmp.substr(ovector[0], ovector[1] - ovector[0]) ).size();

		return true;
	}
	return false;
}


#else//USE_PCRE

/*
 *
 */
RegEx::RegEx(const std::string &pattern, int flags)
:pcrecpp::RE(pattern, pcrecpp::RE_Options(flags))
{
	m_options = flags;

	se_debug_message(SE_DEBUG_REGEX, "expression=%s", pattern.c_str());

	if(!error().empty())
		std::cerr << "RegEx::Error: " << error() << std::endl;
}

/*
 *
 */
RegEx::~RegEx()
{
	se_debug(SE_DEBUG_REGEX);
}

/*
 *
 */
bool RegEx::exec(const Glib::ustring &string)
{
	return FullMatch(string.c_str());
	//return PartialMatch(string.c_str());
}

bool RegEx::exec(const Glib::ustring &string, Glib::ustring::size_type &start, Glib::ustring::size_type &len)
{

#warning "FIXME: Utiliser PCRECPP au lieu de PCRE"
	// on recupere les autres info de pcrecpp

	int flags = PCRE_UTF8 | PCRE_MULTILINE;

	// pour pcre
	pcre* m_pcre = NULL;
	const char *error_ptr = NULL;
	int error_offset = 0;
	int error_code = 0;

	if(m_options & CASELESS)
		flags |= PCRE_CASELESS;

	// crée l'objet
	m_pcre = pcre_compile2(pattern().c_str(), flags,
			&error_code, &error_ptr, &error_offset, NULL);

	if(m_pcre == NULL || error_code != 0)
	{
		return false;
	}
	else
	{
		int options = 0;
		int ovector[2];

		int rc = pcre_exec(
				(const pcre *)m_pcre,
				NULL,
				string.c_str(), (int)string.bytes(),
				0,
				options, 
				ovector, 2);

		pcre_free(m_pcre);

		if(rc >= 0)
		{
			std::string tmp = string;
			
			start = Glib::ustring( tmp.substr(0, ovector[0]) ).size();
			len = Glib::ustring( tmp.substr(ovector[0], ovector[1] - ovector[0]) ).size();

			return true;
		}
	}

	return false;
}

#endif//USE_PCRE

