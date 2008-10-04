#include "FileReader.h"
#include "utility.h"
#include "Error.h"
#include "Encodings.h"

/*
 * Reads an entire file into a string, with good error checking.
 * If charset is empty, auto detection is try.
 */
bool get_contents_from_file(const Glib::ustring &uri, const Glib::ustring &charset, Glib::ustring &utf8_contents, Glib::ustring &charset_contents)
{
	se_debug_message(SE_DEBUG_UTILITY, "Try to get contents from file uri=%s with charset=%s", uri.c_str(), charset.c_str());

	try
	{
		std::string content = Glib::file_get_contents(Glib::filename_from_uri(uri));

		if(charset.empty())
		{
			// Try to autodetect
			utf8_contents = Encoding::convert_to_utf8(content, charset_contents);

			se_debug_message(SE_DEBUG_UTILITY, 
					"Success to get the contents of the file %s with %s charset", 
					uri.c_str(), charset_contents.c_str());
				
			return true;
		}
		else
		{
			// try with charset
			utf8_contents = Encoding::convert_to_utf8_from_charset(content, charset);

			se_debug_message(SE_DEBUG_UTILITY, 
					"Success to get the contents of the file %s with %s charset", 
					uri.c_str(), charset.c_str());
					return true;
		}
	}
	catch(const Glib::Exception &ex)
	{
		throw IOFileError(ex.what());
	}

	return false;
}

/*
 * Constructor.
 * 
 * Open the file from an uri and convert the contents from charset to UTF-8.
 * If charset is empty, try to autodetect the character coding.
 */
FileReader::FileReader(const Glib::ustring &uri, const Glib::ustring &charset)
{
	if(get_contents_from_file(uri, charset, m_data, m_charset) == false)
		return;

	m_uri = uri;
	// FIXME: build lines in getline
	m_lines = Glib::Regex::split_simple("\\R", m_data); 
	m_iter = m_lines.begin();
}

/*
 * Return the uri of the file.
 */
Glib::ustring FileReader::get_uri() const
{
	return m_uri;
}
	
/*
 * Return the contents of the file.
 */
const Glib::ustring& FileReader::get_data() const
{
	return m_data;
}
	
/*
 * Return the charset of the file.
 */
Glib::ustring FileReader::get_charset() const
{
	return m_charset;
}

/*
 * Return the newline detected of the file.
 */
Glib::ustring FileReader::get_newline()
{
	if(Glib::Regex::match_simple("\\r\\n", m_data))
		return "Windows";
	else if(Glib::Regex::match_simple("\\r", m_data))
		return "Macintosh";
	else if(Glib::Regex::match_simple("\\n", m_data))
		return "Unix";

	return Glib::ustring();
}

/*
 * Get the next line of the file without newline character (CR, LF or CRLF).
 */
bool FileReader::getline(Glib::ustring &line)
{
	if(m_iter == m_lines.end())
		return false;

	line = *m_iter;
		++m_iter;

	return true;
}

