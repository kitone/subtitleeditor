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
#include <string>
#include <glibmm.h>
#include <iostream>
#include <sstream>
#include <gtkmm.h>
#include "Config.h"
#include "SubtitleTime.h"
#include "SubtitleEditorWindow.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

/*
 *	profile name use by config dir
 */
Glib::ustring static_profile_name = "default";

/*
 *
 */
Glib::ustring build_message(const char *format, ...)
{
	Glib::ustring res;
	
	va_list args;
	char *formatted = NULL;

	va_start(args, format);
	formatted = g_strdup_vprintf(format, args);
	va_end(args);

	res = formatted;
	
	g_free(formatted);

	return res;
}

/*
 *	the profile name for the config dir
 *	~/config/subtitleeditor/{profile}
 */
void set_profile_name(const Glib::ustring &profile)
{
	se_debug_message(SE_DEBUG_UTILITY, "profile=%s", profile.c_str());

	if(!profile.empty())
		static_profile_name = profile;
}


/*
 *	~/.config/subtitleeditor/{profile}/
 *	XDG Base Directory Specification
 */
Glib::ustring get_config_dir(const Glib::ustring &file)
{
	const gchar *configdir = g_get_user_config_dir();
	
	Glib::ustring path = Glib::build_filename(configdir, "subtitleeditor");

	// create config path if need
	if(Glib::file_test(path, Glib::FILE_TEST_IS_DIR) == false)
	{
		//g_mkdir(path.c_str(), 0700);
		Glib::spawn_command_line_sync("mkdir " + path);
	}
	
	// create profile path if need
	path = Glib::build_filename(path, static_profile_name);

	if(Glib::file_test(path, Glib::FILE_TEST_IS_DIR) == false)
	{
		Glib::spawn_command_line_sync("mkdir " + path);
	}


	return Glib::build_filename(path, file);
}


/*
 *
 */
void dialog_warning(const Glib::ustring &primary_text, const Glib::ustring &secondary_text)
{
	Glib::ustring msg;

	msg += "<span weight=\"bold\" size=\"larger\">";
	msg += primary_text;
	msg += "</span>\n\n";
	msg += secondary_text;

	Gtk::MessageDialog dialog(msg, true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
	dialog.run();
	//MessageDialog *dialog = new MessageDialog(msg, Gtk::MESSAGE_WARNING);
}

/*
 *
 */
void dialog_error(const Glib::ustring &primary_text, const Glib::ustring &secondary_text)
{
	Glib::ustring msg;

	msg += "<span weight=\"bold\" size=\"larger\">";
	msg += primary_text;
	msg += "</span>\n\n";
	msg += secondary_text;

	Gtk::MessageDialog dialog(msg, true, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dialog.run();
	//MessageDialog *dialog = new MessageDialog(msg, Gtk::MESSAGE_ERROR);
}





#ifdef HAVE_ISO_CODES

#include <fstream>
#include <map>

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

/*
 * max length of sscanf'ed fields + 1
 */
#define ISO_639_2_TERMINOLOGY_LEN	4
#define ISO_639_2_BIBLIOGRAPHY_LEN	4
#define ISO_639_1_CODE_LEN	3
#define ISO_639_NAME_LEN	256

#define ISO_3166_ALPHA2_LEN	3
#define ISO_3166_NAME_LEN	256

static bool map_iso_639_initialised = false;
static bool map_iso_3166_initialised = false;

std::map<Glib::ustring, Glib::ustring> map_iso_639;
std::map<Glib::ustring, Glib::ustring> map_iso_3166;

/*
 *
 */
void read_iso_639(const Glib::ustring &filename)
{
	std::ifstream file(filename.c_str());
	if(!file)
	{
		std::cerr << "Cannot open file: " << filename << std::endl;
		return;
	}

	std::string line;
	

	gchar *format = g_strdup_printf("%%%is %%%is %%%is %%n%%%ic%%n",
			ISO_639_2_TERMINOLOGY_LEN - 1,
			ISO_639_2_BIBLIOGRAPHY_LEN - 1,
			ISO_639_1_CODE_LEN - 1,
			ISO_639_NAME_LEN - 1);

	char terminology[ISO_639_2_TERMINOLOGY_LEN];
	char bibliography[ISO_639_2_BIBLIOGRAPHY_LEN];
	char code[ISO_639_1_CODE_LEN];
	char name[ISO_639_NAME_LEN];
	int plen, nlen;
	
	while(!file.eof() && std::getline(file, line))
	{
		if(!line.empty() && line[0] != '#')
		{
			if(sscanf(line.c_str(), format, terminology, bibliography, code, &plen, name, &nlen) > 0)
			{
				if(nlen - plen > 0 && std::string(code) != "XX")
				{
					name[nlen - plen] = '\0';
					map_iso_639[code]=name;
				}
			}
		}
	}
	g_free(format);

	file.close();

	map_iso_639_initialised = true;
}


/*
 *
 */
void read_iso_3166(const Glib::ustring &filename)
{
	std::ifstream file(filename.c_str());
	if(!file)
	{
		std::cerr << "Cannot open file: " << filename << std::endl;
		return;
	}

	std::string line;

	gchar *format = g_strdup_printf("%%%is %%n%%%ic%%n",
			ISO_3166_ALPHA2_LEN - 1,
			ISO_3166_NAME_LEN - 1);
	char code[ISO_3166_ALPHA2_LEN];
	char name[ISO_3166_NAME_LEN];
	int plen, nlen;

	while(!file.eof() && std::getline(file, line))
	{
		if(!line.empty() && line[0] != '#')
		{
			if(sscanf(line.c_str(), format, code, &plen, name, &nlen) > 0)
			{
				name[nlen - plen] = '\0';
				map_iso_3166[code]=name;
			}
		}
	}
	g_free(format);

	file.close();

	map_iso_3166_initialised = true;
}


/*
 *
 */
void initialised()
{
	//bindtextdomain (ISO_639_DOMAIN, ISO_CODES_PATH);
	bind_textdomain_codeset (ISO_639_DOMAIN, "UTF-8");

	//bindtextdomain(ISO_3166_DOMAIN, ISO_CODES_PATH);
	bind_textdomain_codeset (ISO_3166_DOMAIN, "UTF-8");

	read_iso_639(Glib::build_filename(ISO_CODES_PATH, "iso_639.tab"));
	read_iso_3166(Glib::build_filename(ISO_CODES_PATH, "iso_3166.tab"));
}

/*
 *
 */
Glib::ustring get_iso_name_for_lang_code(const Glib::ustring &code)
{
	Glib::ustring c1, c2;

	if(!map_iso_639_initialised)
		initialised();

	if(code.size() == 2)
	{
		c1 = code.substr(0, 2);
		
		Glib::ustring s639 = dgettext(ISO_639_DOMAIN, map_iso_639[c1].c_str());

		return s639;
	}
	else if(code.size() == 5)
	{
		c1 = code.substr(0, 2);
		c2 = code.substr(3, 5);

		Glib::ustring s639 = dgettext(ISO_639_DOMAIN, map_iso_639[c1].c_str());
		Glib::ustring s3166 = dgettext(ISO_3166_DOMAIN, map_iso_3166[c2].c_str());
	
		return s639 + " (" + s3166 + ")";
	}

	return code;
}

#else //HAVE_ISO_CODES

Glib::ustring get_iso_name_for_lang_code(const Glib::ustring &code)
{
	return code;
}

#endif //HAVE_ISO_CODES


namespace utility
{
	bool string_to_bool(const std::string &str)
	{
		std::istringstream s(str);
		bool val = false;
		s >> val;
		return val;
	}
	
	/*
	 *
	 */
	int string_to_int(const std::string &str)
	{
		std::istringstream s(str);
		int val = 0;
		s >> val;
		return val;
	}

	/*
	 *
	 */
	int string_to_long(const std::string &str)
	{
		std::istringstream s(str);
		long val = 0;
		s >> val;
		return val;
	}

	/*
	 *
	 */
	double string_to_double(const std::string &str)
	{
		std::istringstream s(str);
		double val = 0;
		s >> val;
		return val;
	}
	
	/*
	 *
	 */
	void split(const std::string &str, const char &c, std::vector<std::string> &array, int max)
	{
		array.clear();

		std::istringstream iss(str);
		std::string word;

		if(max > 0)
		{
			int count = 1;
			while(std::getline(iss, word, (count < max) ? c : '\n'))
			{
				//std::cout << "word:" << word << std::endl;
				array.push_back(word);

				++count;
			}
		}
		else
		{
			while(std::getline(iss, word, c))
			{
				//std::cout << "word:" << word << std::endl;
				array.push_back(word);
			}
		}
	}

	void usplit(const Glib::ustring &str, const Glib::ustring::value_type &delimiter, std::vector<Glib::ustring> &container)
	{
		Glib::ustring::const_iterator it = str.begin(), end = str.end(), first;

		for(first = it; it != end; ++it)
		{
			if(delimiter == *it)
			{
				if(first != it) // || keep_blank)
				{
					// extract the current field from the string
					container.push_back(Glib::ustring(first, it));
					// skip the next delimiter
					first = it; ++first;
				}
				else
					++first;
			}
		}

		if(first != it) // || keep_blank
		{
			// extract the last field from the string
			container.push_back(Glib::ustring(first, it));
		}
	}

	/*
	 * Search and replace function. 
	 */
	void replace(Glib::ustring &text, const Glib::ustring &pattern, const Glib::ustring &replace_by)
	{
		Glib::ustring::size_type pos = 0;
	
		while((pos = text.find(pattern, pos)) != Glib::ustring::npos)
		{
			text.replace(pos, pattern.size(), replace_by);
			pos = pos + replace_by.size();
		}
	}
	
	/*
	 * Search and replace function. 
	 */
	void replace(std::string &text, const std::string &pattern, const std::string &replace_by)
	{
		std::string::size_type pos = 0;
	
		while((pos = text.find(pattern, pos)) != std::string::npos)
		{
			text.replace(pos, pattern.size(), replace_by);
			pos = pos + replace_by.size();
		}
	}

	/*
	 *	transforme test/file.srt en /home/toto/test/file.srt 
	 */
	Glib::ustring create_full_path(const Glib::ustring &_path)
	{
		if(_path.empty())
			return Glib::ustring();

		if(Glib::path_is_absolute(_path))
			return _path;

		Glib::ustring path =_path;

		// remove ./
		{
			Glib::ustring str("./");
			if(path.compare(0, str.length(), str) == 0)
				path.replace(0, str.length(), "");
		}

		Glib::ustring curdir = Glib::get_current_dir();

		Glib::ustring newpath = Glib::build_filename(curdir, path);

		return newpath;
	}


	/*
	 *	est ce un chiffre
	 */
	bool is_num(const Glib::ustring &str)
	{
		if(str.empty())
			return false;
		
		std::istringstream s(str);

		int num = 0;

		return s >> num != 0;
	}

	/*
	 * Get the number of characters per second.
	 * msec = SubtitleTime::totalmsecs
	 */
	double get_characters_per_second(const Glib::ustring &text, const long msecs)
	{
		if(msecs == 0)
			return 0;

		std::vector<int> num_characters = utility::get_characters_per_line(text);
		
		if (num_characters.size() == 0)
			return 0;

		unsigned int len = 0;

		for (std::vector<int>::const_iterator it = num_characters.begin(); it != num_characters.end(); ++it)
			len += *it;
		
		len += 2 * (num_characters.size() - 1);	// a newline counts as 2 characters

		if(len == 0)
			return 0;

		double cps = static_cast<double>(len * 1000) / msecs;

		return cps;
	}
	
	/*
	 *	get number of characters for each line in the text
	 */
	std::vector<int> get_characters_per_line(const Glib::ustring &text)
	{
		std::vector<int> num_characters;
		std::istringstream iss(utility::get_stripped_text(text));
		std::string line;

		while( std::getline(iss, line) )
		{
			Glib::ustring::size_type len = reinterpret_cast<Glib::ustring&>(line).size();
			num_characters.push_back(len);
		}
		
		return num_characters;
	}
	
	/*
	 * get a text stripped from tags
	 */
	Glib::ustring get_stripped_text(const Glib::ustring &text)
	{
		// pattern for tags like <i>, </i>, {\comment}, etc.
		static Glib::RefPtr<Glib::Regex> tag_pattern = Glib::Regex::create("<.*?>|{.*?}");

		return tag_pattern->replace(text, 0, "", static_cast<Glib::RegexMatchFlags>(0));
	}

	/*
	 *
	 */
	void set_transient_parent(Gtk::Window &window)
	{
		Gtk::Window *root = dynamic_cast<Gtk::Window*>(SubtitleEditorWindow::get_instance());
		if(root)
			window.set_transient_for(*root);
	}

}//namespace utility

