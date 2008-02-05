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
 *
 */
bool check_end_char(const gchar c)
{
	switch(c)
	{
	case '\0':
	case '\t':
	case '\015':
	//case '\r':
	case '\n':
		return true;
	default:
	 return false;
	}
	return false;
}


/*
 *
 */
Glib::ustring check_end_char(const Glib::ustring &str)
{
	Glib::ustring::size_type pos = str.find('\015');
	
	if(pos != Glib::ustring::npos)
		return str.substr(0, pos);
	
	return str;
}


/*
 *
 */
Glib::ustring get_share_dir(const Glib::ustring &file)
{
	Glib::ustring filename;
	
	filename = Glib::build_filename(PACKAGE_SHARE_DIR, file);

	se_debug_message(SE_DEBUG_UTILITY, "%s", filename.c_str());

	if(Glib::file_test(filename, Glib::FILE_TEST_EXISTS))
		return filename;

	// utiliser pour le dev 
	filename = Glib::build_filename("share", file);

	se_debug_message(SE_DEBUG_UTILITY, "%s", filename.c_str());

	if(Glib::file_test(filename, Glib::FILE_TEST_EXISTS))
		return filename;

	se_debug_message(SE_DEBUG_UTILITY, "%s not found!", filename.c_str());
	//
	std::cerr << "get_share_dir not found:" << file << std::endl;
	return "";
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
		Glib::spawn_command_line_async("mkdir " + path);
	}
	
	// create profile path if need
	path = Glib::build_filename(path, static_profile_name);

	if(Glib::file_test(path, Glib::FILE_TEST_IS_DIR) == false)
	{
		Glib::spawn_command_line_async("mkdir " + path);
	}


	return Glib::build_filename(path, file);
}


/*
 *
 */
void find_and_replace(Glib::ustring &text, const Glib::ustring &find, const Glib::ustring &replace)
{
	se_debug_message(SE_DEBUG_SEARCH, "text=<%s> find=<%s> replace=<%s>", 
			text.c_str(), find.c_str(), replace.c_str());

	Glib::ustring::size_type pos = 0;
	
	while((pos = text.find(find, pos)) != Glib::ustring::npos)
	{
		text.replace(pos, find.size(), replace);
		pos = pos + replace.size();
	}
}

/*
class MessageDialog : public Gtk::MessageDialog
{
public:
	MessageDialog(const Glib::ustring &msg, Gtk::MessageType type)
	:Gtk::MessageDialog(msg, true, type, Gtk::BUTTONS_OK, false)
	{
		std::cout << "MessageDialog" << std::endl;
		show();
		//run();
	}

	~MessageDialog()
	{
		std::cout << "~MessageDialog" << std::endl;
	}

	virtual void on_response(int id)
	{
		std::cout << "on_response:" << id << std::endl;
		delete this;
	}
};
*/

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


/*
 *
 */
SpinButtonTiming::SpinButtonTiming()
{
	set_increments(100, 1);

	set_use_negative(false);

	int width, height;

	create_pango_layout("00:00:00.000")->get_pixel_size(width, height);

	set_size_request(width + 25, -1);

	// default value
	set_numeric(false);

	set_text(SubtitleTime::null());

	set_numeric(true);
}

/*
 *
 */
SpinButtonTiming::SpinButtonTiming(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::SpinButton(cobject)
{
	set_increments(100, 1);

	set_use_negative(false);

	int width, height;

	create_pango_layout("00:00:00.000")->get_pixel_size(width, height);

	set_size_request(width + 25, -1);

	// default value
	set_numeric(false);

	set_text(SubtitleTime::null());

	set_numeric(true);
}

/*
 *
 */
int SpinButtonTiming::on_input(double *new_value)
{
	Glib::ustring text = get_text();

	if(SubtitleTime::validate(text))
		*new_value = (double) SubtitleTime(text).totalmsecs;

	return true;
}

/*
 *
 */
bool SpinButtonTiming::on_output()
{
	//bool state = Gtk::SpinButton::on_output();
	//return state;
	
	set_numeric(false);

	set_text(SubtitleTime((long int)get_value()).str());

	set_numeric(true);

	return true;
}

/*
 *
 */
void SpinButtonTiming::set_use_negative(bool use_negative)
{
	if(use_negative)
		set_range(-86399999, 86399999);
	else
		set_range(0, 86399999);
}

/*
 *
 */
bool SpinButtonTiming::on_scroll_event(GdkEventScroll *ev)
{
	int val = (ev->state & GDK_CONTROL_MASK) ? 1000 : 100;

	if(ev->direction == GDK_SCROLL_UP)
	{
		set_value(get_value() + val);
	}
	else if(ev->direction == GDK_SCROLL_DOWN)
	{
		set_value(get_value() -val);
	}

	return true;
}

void SpinButtonTiming::on_insert_text(const Glib::ustring &str, int *pos)
{
	Gtk::SpinButton::on_insert_text(str, pos);
/*
	if(str.size() == 1)
	{
		int position = *pos;
		if(position == 1 || position == 4 || position == 7)
		{
			delete_text(position, position + 1);
			*pos = *pos + 1;
		}
		else
			delete_text(position, position + 1);
	}
*/
}

/*
SubtitleTime SpinButtonTiming::get_time()
{
	return SubtitleTime((long int)get_value());
}
*/

namespace Gst
{

#include <gst/gst.h>
	
	/*
	 *	retourne le temps en string par rapport au temps nsecs (gstreamer)
	 */
	Glib::ustring time_to_string (gint64 time)
	{
		gchar *str = g_strdup_printf ("%u:%02u:%02u",
				(guint) (time / (GST_SECOND * 60 * 60)),
				(guint) ((time / (GST_SECOND * 60)) % 60),
				(guint) ((time / GST_SECOND) % 60));

		Glib::ustring res(str);
		g_free(str);
		return res;
	}
	
}

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
	 *	retourne le nombre de caractères par seconde
	 *	msec = SubtitleTime::totalmsecs
	 */
	int get_characters_per_second(const Glib::ustring &text, const long msecs)
	{
		if(msecs == 0)
			return 0;

		unsigned int len = text.size();

		if(len == 0)
			return 0;

		double dsX = len * 1000 / msecs;

		int ds = (int)(round(dsX*10) / 10);

		return ds;
	}

}//namespace utility


/*
 *
 */
namespace WidgetToConfig
{

void on_check_button(Gtk::CheckButton *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	Config::getInstance().set_value_bool(group, key, widget->get_active());
}

void on_font_button(Gtk::FontButton *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	Config::getInstance().set_value_string(group, key, widget->get_font_name());
}

void on_color_button(Gtk::ColorButton *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	Color color;
	color.getFromColorButton(*widget);

	Config::getInstance().set_value_color(group, key, color);
}

void on_range(Gtk::Range *range, const Glib::ustring &group, const Glib::ustring &key)
{
	Config::getInstance().set_value_double(group, key, range->get_value());
}

void on_entry(Gtk::Entry *spin, const Glib::ustring &group, const Glib::ustring &key)
{
	Config::getInstance().set_value_string(group, key, spin->get_text());
}

void on_spin_button(Gtk::SpinButton *spin, const Glib::ustring &group, const Glib::ustring &key)
{
	Config::getInstance().set_value_double(group, key, spin->get_value());
}


/*
 *
 */
void connect(Gtk::Widget *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	if(Gtk::CheckButton *check = dynamic_cast<Gtk::CheckButton*>(widget))
	{
		check->signal_toggled().connect(
			sigc::bind<Gtk::CheckButton*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_check_button), check, group, key));
	}
	else if(Gtk::Range *range = dynamic_cast<Gtk::Range*>(widget))
	{
		range->signal_value_changed().connect(
			sigc::bind<Gtk::Range*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_range), range, group, key));
	}
	else if(Gtk::SpinButton *spin = dynamic_cast<Gtk::SpinButton*>(widget))
	{
		spin->signal_value_changed().connect(
			sigc::bind<Gtk::SpinButton*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_spin_button), spin, group, key));
	}
	else if(Gtk::Entry *entry = dynamic_cast<Gtk::Entry*>(widget))
	{
		//entry->signal_activate().connect(
		entry->signal_changed().connect(
			sigc::bind<Gtk::Entry*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_entry), entry, group, key));
	}
	else if(Gtk::FontButton *font = dynamic_cast<Gtk::FontButton*>(widget))
	{
		font->signal_font_set().connect(
			sigc::bind<Gtk::FontButton*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_font_button), font, group, key));
	}
	else if(Gtk::ColorButton *color = dynamic_cast<Gtk::ColorButton*>(widget))
	{
		color->signal_color_set().connect(
			sigc::bind<Gtk::ColorButton*, Glib::ustring, Glib::ustring>(
				sigc::ptr_fun(&on_color_button), color, group, key));
	}
}

/*
 *
 */
void read_config(Gtk::Widget *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	Config &cfg = Config::getInstance();

	if(Gtk::CheckButton *check = dynamic_cast<Gtk::CheckButton*>(widget))
	{
		bool value = false;
		if(cfg.get_value_bool(group, key, value))
		{
			check->set_active(value);
		}
	}
	else if(Gtk::Range *range = dynamic_cast<Gtk::Range*>(widget))
	{
		double value = 0;
		if(cfg.get_value_double(group, key, value))
		{
			range->set_value(value);
		}
	}
	else if(Gtk::SpinButton *spin = dynamic_cast<Gtk::SpinButton*>(widget))
	{
		double value;
		if(cfg.get_value_double(group, key, value))
		{
			spin->set_value(value);
		}
	}
	else if(Gtk::Entry *entry = dynamic_cast<Gtk::Entry*>(widget))
	{
		Glib::ustring value;
		if(cfg.get_value_string(group, key, value))
		{
			entry->set_text(value);
		}
	}
	else if(Gtk::FontButton *font = dynamic_cast<Gtk::FontButton*>(widget))
	{
		Glib::ustring value;
		if(cfg.get_value_string(group, key, value))
		{
			font->set_font_name(value);
		}
	}
	else if(Gtk::ColorButton *colorbutton = dynamic_cast<Gtk::ColorButton*>(widget))
	{
		Color color;
		cfg.get_value_color(group, key, color);

		color.initColorButton(*colorbutton);
	}
}

/*
 *
 */
void read_config_and_connect(Gtk::Widget *widget, const Glib::ustring &group, const Glib::ustring &key)
{
	g_return_if_fail(widget);

	read_config(widget, group, key);
	connect(widget, group, key);
}

}//namespace WidgetToConfig

