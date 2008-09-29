#ifndef _utility_h
#define _utility_h

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
 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glibmm/ustring.h>
#include <gdkmm/color.h>
#include <sstream>
#include <iostream>

#include "debug.h"


#ifdef ENABLE_NLS
	#include <libintl.h>
	#include <glib/gi18n.h>
	/*
	#define _(String) gettext(String)
	#ifdef gettext_noop
	#define N_(String) gettext_noop(String)
	#else
	#define N_(String) (String)
	#endif
	*/
#else /* NLS is disabled */
	#define _(String) (String)
	#define N_(String) (String)
	#define textdomain(String) (String)
	#define gettext(String) (String)
	#define ngettext(String,StringPlural,Number) \
		(((Number)==1)?(String):(StringPlural))
	#define dgettext(Domain,String) (String)
	#define dcgettext(Domain,String,Type) (String)
	#define bindtextdomain(Domain,Directory) (Domain) 
	#define bind_textdomain_codeset(Domain,Codeset) (Codeset) 
#endif /* ENABLE_NLS */


/*
 *
 */
Glib::ustring get_iso_name_for_lang_code(const Glib::ustring &code);

/*
 *	
 */
Glib::ustring get_share_dir(const Glib::ustring &file);

/*
 *	the profile name for the config dir
 *	~/config/subtitleeditor/{profile}
 */
void set_profile_name(const Glib::ustring &profile);

/*
 *	~/.config/subtitleeditor/{profile}/{file}
 *	XDG Base Directory Specification
 */
Glib::ustring get_config_dir(const Glib::ustring &file);

/*
 *
 */
Glib::ustring check_end_char(const Glib::ustring &str);

/*
 *	convertir str en n'importe quel type
 */
template<class T>
bool from_string(const std::string &src, T& dest)
{
	std::istringstream s(src);
	// return s >> dest != 0;

	bool state = s >> dest != 0;

	if(!state)
		se_debug_message(SE_DEBUG_UTILITY, "string:'%s'failed.", src.c_str());

#ifdef DEBUG
	g_return_val_if_fail(state, false);
#endif
	return state;
}

/*
 *	convertir str en n'importe quel type
 */
template<class T>
bool from_string(const Glib::ustring &src, T& dest)
{
	std::istringstream s(src);
	// return s >> dest != 0;

	bool state = s >> dest != 0;

	if(!state)
		se_debug_message(SE_DEBUG_UTILITY, "string:'%s'failed.", src.c_str());

#ifdef DEBUG
	g_return_val_if_fail(state, false);
#endif
	return state;
}

/*
 *	convertir n'importe quoi en string
 */
template<class T>
std::string to_string(const T &src)
{
	std::ostringstream oss;
	oss << src;
	
	return oss.str();
}

/*
 *
 */
Glib::ustring build_message(const gchar *str, ...);

/*
 *
 */
void dialog_warning(const Glib::ustring &primary_text, const Glib::ustring &secondary_text);

/*
 *
 */
void dialog_error(const Glib::ustring &primary_text, const Glib::ustring &secondary_text);


/*
 *	devired with glade 
 */

#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>
#include <libglademm/xml.h>

class ComboBoxText : public Gtk::ComboBoxText
{
public:
	ComboBoxText(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::ComboBoxText(cobject)
	{
	}
};

namespace utility
{
	/*
	 *
	 */

	template<class T>
	inline void clamp(T &val, const T &min, const T &max)
	{
		val = CLAMP(val, min, max);
		//if(val < min)
		//	val = min;
		//else if(val > max)
		//	val = max;
	}

	/*
	 *
	 */
	bool string_to_bool(const std::string &str);
	
	/*
	 *
	 */
	int string_to_int(const std::string &str);

	/*
	 *
	 */
	int string_to_long(const std::string &str);

	/*
	 *
	 */
	double string_to_double(const std::string &str);

	/*
	 *
	 */
	void split(const std::string &str, const char &c, std::vector<std::string> &array, int max=-1);

	/*
	 * Split with best utf8 support...
	 */
	void usplit(const Glib::ustring &str, const Glib::ustring::value_type &delimiter, std::vector<Glib::ustring> &container);

	/*
	 * Search and replace function. 
	 */
	void replace(Glib::ustring &text, const Glib::ustring &pattern, const Glib::ustring &replace_by);
	
	/*
	 * Search and replace function. 
	 */
	void replace(std::string &text, const std::string &pattern, const std::string &replace_by);

	/*
	 *	transforme test/file.srt en /home/toto/test/file.srt 
	 */
	Glib::ustring create_full_path(const Glib::ustring &path);

	/*
	 *	est ce un chiffre
	 */
	bool is_num(const Glib::ustring &str);

	/*
	 *	retourne le nombre de caractères par seconde
	 *	msec = SubtitleTime::totalmsecs
	 */
	int get_characters_per_second(const Glib::ustring &text, const long msecs);

	/*
	 *	get number of characters for each line in the text
	 */
	std::vector<int> get_num_characters(const Glib::ustring &text);

	/*
	 * get a text stripped from tags
	 */
	Glib::ustring get_stripped_text(const Glib::ustring &text);

	/*
	 *	crée et retourne un widget à partir d'un fichier glade
	 */
	template<class T>
	T* get_widget_derived(const Glib::ustring &glade_file, const Glib::ustring &name)
	{
		se_debug_message(SE_DEBUG_UTILITY, "glade_file=<%s> name=<%s>", glade_file.c_str(), name.c_str());

		T *dialog = NULL;

		Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(get_share_dir("glade/" + glade_file));

		refXml->get_widget_derived(name, dialog);

		return dialog;
	}

	/*
	 *
	 */
	void set_transient_parent(Gtk::Window &window);
	
}

namespace Gst
{
	/*
	 *	retourne le temps en string par rapport au temps nsecs (gstreamer)
	 */
	Glib::ustring time_to_string (gint64 time);

	/*
	 * Display a message for missing plugins.
	 */
	void dialog_missing_plugins(const std::list<Glib::ustring> &missings);

	/*
	 * Checks if the element exists and whether its version is at least the version required.
	 * Display a dialog error if failed.
	 */
	bool check_registry(const Glib::ustring &name, int min_major, int min_minor, int min_micro);
}

/*
 *
 */
namespace WidgetToConfig
{
	/*
	 *
	 */
	void connect(Gtk::Widget *widget, const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	void read_config(Gtk::Widget *widget, const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	void read_config_and_connect(
													Gtk::Widget *widget, 
													const Glib::ustring &group, 
													const Glib::ustring &key);
}


#endif//_utility_h
