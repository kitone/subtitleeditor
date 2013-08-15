#ifndef _Encodings_h
#define _Encodings_h

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


#include "i18n.h"
#include <glibmm.h>

struct EncodingInfo
{
	const gchar *charset;
	const gchar *name;
};

static EncodingInfo encodings_info [] = {

	{ "ISO-8859-1", N_("Western") },
	{ "ISO-8859-2", N_("Central European") },
	{ "ISO-8859-3", N_("South European") },
	{ "ISO-8859-4", N_("Baltic") },
	{ "ISO-8859-5", N_("Cyrillic") },
	{ "ISO-8859-6", N_("Arabic") },
	{ "ISO-8859-7", N_("Greek") },
	{ "ISO-8859-8", N_("Hebrew Visual") },
	{ "ISO-8859-8-I", N_("Hebrew") },
	{ "ISO-8859-9", N_("Turkish") },
	{ "ISO-8859-10", N_("Nordic") },
	{ "ISO-8859-13", N_("Baltic") },
	{ "ISO-8859-14", N_("Celtic") },
	{ "ISO-8859-15", N_("Western") },
	{ "ISO-8859-16", N_("Romanian") },

	{ "UTF-8", N_("Unicode") },
	{ "UTF-7", N_("Unicode") },
	{ "UTF-16", N_("Unicode") },
	{ "UCS-2", N_("Unicode") },
	{ "UCS-4", N_("Unicode") },

	{ "ARMSCII-8", N_("Armenian") },
	{ "BIG5", N_("Chinese Traditional") },
	{ "BIG5-HKSCS", N_("Chinese Traditional") },
	{ "CP866", N_("Cyrillic/Russian") },

	{ "EUC-JP", N_("Japanese") },
	{ "EUC-KR", N_("Korean") },
	{ "EUC-TW", N_("Chinese Traditional") },

	{ "GB18030", N_("Chinese Simplified") },
	{ "GB2312", N_("Chinese Simplified") },
	{ "GBK", N_("Chinese Simplified") },
	{ "GEORGIAN-ACADEMY", N_("Georgian") },
	{ "HZ", N_("Chinese Simplified") },

	{ "IBM850", N_("Western") },
	{ "IBM852", N_("Central European") },
	{ "IBM855", N_("Cyrillic") },
	{ "IBM857", N_("Turkish") },
	{ "IBM862", N_("Hebrew") },
	{ "IBM864", N_("Arabic") },

	{ "ISO2022JP", N_("Japanese") },
	{ "ISO2022KR", N_("Korean") },
	{ "ISO-IR-111", N_("Cyrillic") },
	{ "JOHAB", N_("Korean") },
	{ "KOI8R", N_("Cyrillic") },
	{ "KOI8U", N_("Cyrillic/Ukrainian") },

	
	{ "SHIFT_JIS", N_("Japanese") },
	{ "TCVN", N_("Vietnamese") },
	{ "TIS-620", N_("Thai") },
	{ "UHC", N_("Korean") },
	{ "VISCII", N_("Vietnamese") },

	{ "WINDOWS-1250", N_("Central European") },
	{ "WINDOWS-1251", N_("Cyrillic") },
	{ "WINDOWS-1252", N_("Western") },
	{ "WINDOWS-1253", N_("Greek") },
	{ "WINDOWS-1254", N_("Turkish") },
	{ "WINDOWS-1255", N_("Hebrew") },
	{ "WINDOWS-1256", N_("Arabic") },
	{ "WINDOWS-1257", N_("Baltic") },
	{ "WINDOWS-1258", N_("Vietnamese") },

	{ NULL, NULL}
};


class Encodings
{
public:

	/*
	 *
	 */
	static EncodingInfo* get_from_charset(const Glib::ustring &charset);

	/*
	 * Return a human readable string or empty string, ex:
	 * "name (charset)"
	 * "Unicode (UTF-8)"
	 */
	static Glib::ustring get_label_from_charset(const Glib::ustring &charset);

protected:
	static bool is_initialized;

	static bool initialize();
};

namespace Encoding
{

	/*
	 * Trying to convert from charset to UTF-8.
	 * Return utf8 string or throw EncodingConvertError exception.
	 */
	Glib::ustring convert_to_utf8_from_charset(const std::string &content, const Glib::ustring &charset);
	
	/*
	 * Trying to autodetect the charset and convert to UTF-8.
	 * 3 steps:
	 *	- Try UTF-8
	 *	- Try with user encoding preferences
	 *	- Try with all encodings
	 *
	 * Return utf8 string and sets charset found 
	 * or throw EncodingConvertError exception.
	 */
	Glib::ustring convert_to_utf8(const std::string &content, Glib::ustring &charset);

	/*
	 * Convert the UTF-8 text to the charset.
	 * Throw EncodingConvertError exception.
	 */
	std::string convert_from_utf8_to_charset(const Glib::ustring &utf8_content, const Glib::ustring &charset);

}//namespace Encoding

#endif//_Encodings_h

