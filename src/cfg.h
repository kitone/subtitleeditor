#ifndef _Config_h
#define _Config_h

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
 

#include <glibmm.h>
#include <glib.h>
#include <list>
#include <map>
#include <sigc++/sigc++.h>
#include "color.h"

/*
 *
 */
class Config
{
public:

	/*
	 *
	 */
	Config();

	/*
	 *
	 */
	~Config();
	
	/*
	 *	by default (XDG) "~/.config/subtitleeditor/config"
	 */
	static void set_file(const Glib::ustring &file);

	/*
	 *
	 */
	bool loadCfg();
	bool saveCfg();

	/*
	 *
	 */
	bool set_comment(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &comment);

	
	/*
	 *
	 */
	bool has_group(const Glib::ustring &group);
	bool has_key(const Glib::ustring &group, const Glib::ustring &key);
	
	bool get_keys(const Glib::ustring &group, std::list<Glib::ustring> &list);

	/*
	 *
	 */
	bool set_value_bool(const Glib::ustring &group, const Glib::ustring &key, const bool &value, const Glib::ustring &comment = Glib::ustring());
	bool get_value_bool(const Glib::ustring &group, const Glib::ustring &key, bool &value);

	/*
	 *
	 */
	bool set_value_int(const Glib::ustring &group, const Glib::ustring &key, const int &value, const Glib::ustring &comment = Glib::ustring());
	bool get_value_int(const Glib::ustring &group, const Glib::ustring &key, int &value);

	/*
	 *
	 */
	bool set_value_float(const Glib::ustring &group, const Glib::ustring &key, const float &value, const Glib::ustring &comment = Glib::ustring());
	bool get_value_float(const Glib::ustring &group, const Glib::ustring &key, float &value);

	/*
	 *
	 */
	bool set_value_double(const Glib::ustring &group, const Glib::ustring &key, const double &value, const Glib::ustring &comment = Glib::ustring());
	bool get_value_double(const Glib::ustring &group, const Glib::ustring &key, double &value);

	/*
	 *
	 */
	bool set_value_string(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &value, const Glib::ustring &comment = Glib::ustring());
	bool get_value_string(const Glib::ustring &group, const Glib::ustring &key, Glib::ustring &value);

	/*
	 *
	 */
	bool set_value_color(const Glib::ustring &group, const Glib::ustring &key, const Color &color, const Glib::ustring &comment = Glib::ustring());
	bool get_value_color(const Glib::ustring &group, const Glib::ustring &key, Color &color);

	/*
	 *
	 */
	bool set_value_string_list(const Glib::ustring &group, const Glib::ustring &key, const std::list<Glib::ustring> &list);
	bool get_value_string_list(const Glib::ustring &group, const Glib::ustring &key, std::list<Glib::ustring> &list);

	/*
	 *
	 */
	bool remove_group(const Glib::ustring &group);
	bool remove_key(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	static Config& getInstance();

	/*
	 *	permet de surveiller un groupe
	 *	fonction(key, value)
	 */
	sigc::signal<void, Glib::ustring, Glib::ustring>& signal_changed(const Glib::ustring &group);

	/*
	 *
	 */
	bool set_default_value(const Glib::ustring &group, const Glib::ustring &key);
	bool get_default_value(const Glib::ustring &group, const Glib::ustring &key, Glib::ustring &value);


	/*
	 *
	 */
	bool get_value_bool(const Glib::ustring &gorup, const Glib::ustring &key);

	/*
	 *
	 */
	int	get_value_int(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	float get_value_float(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	double get_value_double(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	Glib::ustring	get_value_string(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	Color get_value_color(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	std::list<Glib::ustring> get_value_string_list(const Glib::ustring &group, const Glib::ustring &key);

protected:
	
	/*
	 *
	 */
	bool check_the_key_or_put_default_value(const Glib::ustring &group, const Glib::ustring &key);

	/*
	 *
	 */
	void emit_signal_changed(const Glib::ustring &g, const Glib::ustring &k, const Glib::ustring &v);

	/*
	 *
	 */
	static Glib::ustring m_config_file;

	/*
	 *
	 */
	GKeyFile* m_keyFile;		

	// connect un group à des signaux
	std::map<Glib::ustring, sigc::signal<void, Glib::ustring, Glib::ustring> > m_signals;

	// configuration par defaut [group][key][value]
	std::map<Glib::ustring, std::map<Glib::ustring, Glib::ustring> > m_default_config;
};

#endif//_Config_h
