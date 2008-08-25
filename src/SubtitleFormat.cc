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
 

#include "SubtitleFormat.h"
#include "Encodings.h"
#include <iostream>
#include "Config.h"

/*
 *
 */
SubtitleFormat::SubtitleFormat(Document *doc, const Glib::ustring &format_name)
:	m_document(doc),
	m_scriptInfo(&doc->m_scriptInfo), 
	m_styleModel(doc->m_styleModel)
{
	m_use_auto_deteced = false;

	Config::getInstance().get_value_string_list("encodings", "encodings", m_list_encodings);

	set_newline(doc->getNewLine());
	set_charset(doc->getCharset());
}

/*
 *
 */
SubtitleFormat::~SubtitleFormat()
{
}

/*
 *
 */
Glib::ustring SubtitleFormat::get_format_name()
{
	return m_format_name;
}

/*
 *
 */
void SubtitleFormat::set_charset(const std::string &charset)
{
	m_charset = charset;

	m_use_auto_deteced = charset.empty();

	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "charset=[%s]", charset.c_str());
}

/*
 *
 */
std::string SubtitleFormat::get_charset()
{
	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "charset=[%s]", m_charset.c_str());

	return m_charset;
}

/*
 *
 */
void SubtitleFormat::set_newline(const Glib::ustring &os)
{
	if(os == "Unix")
		m_newline = "\n";
	else if(os == "Windows")
		m_newline = "\r\n";
	else
	{
		std::cerr << "SubtitleFormat can't found newline for [" << os << "]" << std::endl;
		// default unix
		m_newline = "\n";
	}
}

/*
 *	retour le '\n' ou '\r\n' selon la config du doc
 *	"Unix", "Windows"
 */
std::string SubtitleFormat::get_newline()
{
	return m_newline;
}

/*
 *
 */
bool SubtitleFormat::open(const Glib::ustring &filename)
{
	if(on_open(filename))
	{
		/*
		m_document->setFilename(filename);
		m_document->setCharset(get_charset());
		m_document->setFormat(get_format_name());
		*/
		return true;
	}
	return false;
}
	
/*
 *
 */
bool SubtitleFormat::save(const Glib::ustring &filename)
{
	return on_save(filename);
}

/*
 *
 */
std::string convert_to_utf8_from_charset(const std::string &content, const std::string &charset)
{
	se_debug_message(SE_DEBUG_LOADER | SE_DEBUG_SAVER, "charset=[%s] content=[%s]", charset.c_str(), content.c_str());

	// seulement si c'est de l'UTF-8 to UTF-8 on ne passe pas par une convertion
	// juste une verification
	if(charset == "UTF-8")
	{
		if(Glib::ustring(content).validate())
		{
			return content;
		}
		else
		{
			throw Glib::ConvertError(Glib::ConvertError::ILLEGAL_SEQUENCE, 
					_("It's not valid UTF-8.\nPlease use another character encoding."));
		}
	}
	else
	{
		try
		{
			// Convert to utf-8 from charset
			std::string utf8_content = Glib::convert(content, "UTF-8", charset);
			
			return utf8_content;
		}
		catch(const Glib::ConvertError &ex)
		{
			//FIXME
			throw Glib::ConvertError(Glib::ConvertError::ILLEGAL_SEQUENCE,
					build_message(_("Could not convert from %s to UTF-8"), charset.c_str()));
		}
	}
}

/*
 *
 */
std::string convert_to_utf8(const std::list<Glib::ustring> &m_list_encodings, const std::string &content, std::string& encoding)
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);

	if(encoding.empty() == false) // has encoding
	{
		return convert_to_utf8_from_charset(content, encoding);
	}
	else
	{
		// Automatically detect the encoding used
		
		// has user preference
		if(m_list_encodings.size() > 0)
		{
			std::list<Glib::ustring>::const_iterator it;
			for(it = m_list_encodings.begin(); it != m_list_encodings.end(); ++it)
			{
				std::string utf8_content;
			
				try
				{
					utf8_content = convert_to_utf8_from_charset(content,(*it));
				
					if(utf8_content.empty() == false)
					{
						encoding = (*it);
						return utf8_content;
					}
				}
				catch(Glib::ConvertError &ex)
				{
					// invalid, next
				}
			}
		}
		else
		{
			for(unsigned int i=0; encodings_info[i].name!= NULL; ++i)
			{
				std::string charset = encodings_info[i].charset;
				std::string utf8_content;
			
				try
				{
					utf8_content = convert_to_utf8_from_charset(content,charset);
				
					if(utf8_content.empty() == false)
					{
						encoding = charset;
						return utf8_content;
					}
				}
				catch(Glib::ConvertError &ex)
				{
					// invalid, next
				}
			}
		}
	}

	throw Glib::ConvertError(
			Glib::ConvertError::FAILED, 
			_("subtitleeditor was not able to automatically determine the encoding of the file you want to open."));
	
	return Glib::ustring();
}

/*
 *	convertir text en utf8
 *	le charset du text est defini par m_charset
 */
Glib::ustring SubtitleFormat::charset_to_utf8(const std::string &content)
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);

	if(content.empty())
		return "";

	return convert_to_utf8(m_list_encodings, content, m_charset);
}

/*
 *	convertir le text (utf-8) en codage defini par m_charset
 */
std::string SubtitleFormat::utf8_to_charset(const Glib::ustring &content)
{
	se_debug(SE_DEBUG_LOADER | SE_DEBUG_SAVER);

	try
	{
		if(m_charset.empty())
		{
			std::cerr << "utf8_to_charset charset is empty" << std::endl;
			return content;
		}
		
		std::string res = Glib::convert(content, m_charset, "UTF-8");
		return res;
	}
	catch(Glib::ConvertError& ex)
	{
		std::cerr << content << std::endl;
		std::cerr << "utf8_to_charset[" << m_charset << "] " << ex.what() << std::endl;
		std::cerr << std::endl;
	}

	return "";
}

/*
 *	remplace la fin de ligne '\n' par str
 */
void SubtitleFormat::newline_to_characters(Glib::ustring &text, const Glib::ustring &str)
{
	find_and_replace(text, "\n", str);
}

/*
 *	remplace "characters" par '\n'
 */
void SubtitleFormat::characters_to_newline(Glib::ustring &text, const Glib::ustring &characters)
{
	find_and_replace(text, characters, "\n");
}


/*
 *
 */
Document* SubtitleFormat::document()
{
	return m_document;
}
