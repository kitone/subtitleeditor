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

#include "utility.h"
#include "Encodings.h"
#include "Error.h"
#include "cfg.h"

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


namespace Encoding
{

	/*
	 * Trying to convert from charset to UTF-8.
	 * Return utf8 string or throw EncodingConvertError exception.
	 */
	Glib::ustring convert_to_utf8_from_charset(const std::string &content, const Glib::ustring &charset)
	{
		se_debug_message(SE_DEBUG_UTILITY, "Trying to convert from %s to UTF-8", charset.c_str());
		
		// Only if it's UTF-8 to UTF-8
		if(charset == "UTF-8")
		{
			if(Glib::ustring(content).validate() == false)
				throw EncodingConvertError(_("It's not valid UTF-8."));

			return content;
		}
		else
		{
			try
			{
				Glib::ustring utf8_content = Glib::convert(content, "UTF-8", charset);

				if(!utf8_content.validate() || utf8_content.empty())
					throw EncodingConvertError(build_message(_("Couldn't convert from %s to UTF-8"), charset.c_str()));
					
				return utf8_content;
			}
			catch(const Glib::ConvertError &ex)
			{
				se_debug_message(SE_DEBUG_UTILITY, "Glib::ConvertError: %s", ex.what().c_str());
				throw EncodingConvertError(build_message(_("Couldn't convert from %s to UTF-8"), charset.c_str()));
			}
			catch(...)
			{
				se_debug_message(SE_DEBUG_UTILITY, "Unknow error");
				throw EncodingConvertError(build_message(_("Couldn't convert from %s to UTF-8"), charset.c_str()));
			}
		}
	}

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
	Glib::ustring convert_to_utf8(const std::string &content, Glib::ustring &charset)
	{
		if(content.empty())
			return Glib::ustring();

		// First check if it's not UTF-8.
		se_debug_message(SE_DEBUG_UTILITY, "Trying to UTF-8...");

		try
		{
			Glib::ustring utf8_content = Encoding::convert_to_utf8_from_charset(content, "UTF-8");

			if(utf8_content.validate() && utf8_content.empty() == false)
			{
				charset = "UTF-8";
				return content;
			}
		}
		catch(const EncodingConvertError &ex)
		{
			se_debug_message(SE_DEBUG_UTILITY, "EncodingConvertError: %s", ex.what());
		}
		
		// Try to automatically dectect the encoding
		
		// With the user charset preferences...
		se_debug_message(SE_DEBUG_UTILITY, "Trying with user encodings preferences...");

		std::list<Glib::ustring> user_encodings = Config::getInstance().get_value_string_list("encodings", "encodings");
		
		for(std::list<Glib::ustring>::const_iterator it = user_encodings.begin(); it != user_encodings.end(); ++it)
		{
			try
			{
				Glib::ustring utf8_content = Encoding::convert_to_utf8_from_charset(content, *it);

				if(utf8_content.validate() && utf8_content.empty() == false)
				{
					charset = *it;
					return utf8_content;
				}
			}
			catch(const EncodingConvertError &ex)
			{
				// invalid, try with the next...
				se_debug_message(SE_DEBUG_UTILITY, "EncodingConvertError: %s", ex.what());
			}
		}
		
		// With all charset...
		se_debug_message(SE_DEBUG_UTILITY, "Trying with all encodings...");

		for(unsigned int i=0; encodings_info[i].name != NULL; ++i)
		{
			Glib::ustring it = encodings_info[i].charset;

			try
			{
				Glib::ustring utf8_content = Encoding::convert_to_utf8_from_charset(content, it);

				if(utf8_content.validate() && utf8_content.empty() == false)
				{
					charset = it;
					return utf8_content;
				}
			}
			catch(const EncodingConvertError &ex)
			{
				// invalid, try with the next...
				se_debug_message(SE_DEBUG_UTILITY, "EncodingConvertError: %s", ex.what());
			}
		}

		// Failed to determine the encoding...
		throw EncodingConvertError(
			_("subtitleeditor was not able to automatically determine the encoding of the file you want to open."));
	}

	/*
	 * Convert the UTF-8 text to the charset.
	 * Throw EncodingConvertError exception.
	 */
	std::string convert_from_utf8_to_charset(const Glib::ustring &utf8_content, const Glib::ustring &charset)
	{
		se_debug_message(SE_DEBUG_UTILITY, "Trying to convert from UTF-8 to %s", charset.c_str());

		try
		{
			std::string content = Glib::convert(utf8_content, charset, "UTF-8");

			return content;
		}
		catch(const Glib::ConvertError& ex)
		{
			throw EncodingConvertError(build_message(
						_("Could not convert the text to the character coding '%s'"), charset.c_str()));
		}
	}

}//namespace Encoding

