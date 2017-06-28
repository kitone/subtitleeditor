/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	https://kitone.github.io/subtitleeditor/
 *	https://github.com/kitone/subtitleeditor/
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


#include "cfg.h"
#include <iostream>
#include <fstream>
#include <glibmm.h>
#include "utility.h"

void get_default_config(std::map<Glib::ustring, std::map<Glib::ustring, Glib::ustring> > &config);

Glib::ustring Config::m_config_file;

/*
 *
 */
Config& Config::getInstance()
{
	se_debug(SE_DEBUG_APP);

	static Config cfg;
	return cfg;
}

/*
 *
 */
Config::Config()
{
	se_debug(SE_DEBUG_APP);

	get_default_config(m_default_config);
	
	loadCfg();
}

/*
 *
 */
Config::~Config()
{
	se_debug(SE_DEBUG_APP);

	saveCfg();
}

/*
 *	by default (XDG) "~/.config/subtitleeditor/config"
 */
void Config::set_file(const Glib::ustring &file)
{
	se_debug_message(SE_DEBUG_APP, "file=%s", file.c_str());
#warning "FIXME: convert to full path"
	/*
	Glib::ustring dirname = Glib::path_get_dirname(file);
	Glib::ustring filename = Glib::path_get_basename(file);

	std::cout << dirname << std::endl;
	std::cout << filename << std::endl;

	if(dirname == "~")
	{
		dirname = Glib::get_home_dir();
	}
	else if(dirname == ".")
	{
	}

	m_config_file = Glib::build_filename(dirname, filename);
	*/
	m_config_file = file;
}


/*
 *
 */
bool Config::loadCfg()
{
	se_debug_message(SE_DEBUG_APP, "load config...");

	GError *error = NULL;
	m_keyFile = NULL;

	m_keyFile = g_key_file_new();

	Glib::ustring filename = get_config_dir("config");

	if(!g_key_file_load_from_file(m_keyFile, filename.c_str(), (GKeyFileFlags)(G_KEY_FILE_NONE | G_KEY_FILE_KEEP_COMMENTS), &error))
	{
		se_debug_message(SE_DEBUG_APP, "open <%s> failed : %s", filename.c_str(), error->message);

		std::cerr << "Config::Config > " << error->message << std::endl;
		g_error_free(error);
		return false;
	}

	se_debug_message(SE_DEBUG_APP, "load config <%s>", filename.c_str());

	return true;
}

/*
 *
 */
bool Config::saveCfg()
{
	se_debug_message(SE_DEBUG_APP, "save config...");

	GError *error = NULL;
	gsize size = 0;

	gchar *data = g_key_file_to_data(m_keyFile, &size, &error);
	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "save config failed : %s", error->message);

		std::cerr << "Config::~Config > " << error->message << std::endl;
		g_error_free(error);
		return false;
	}
	else
	{
		Glib::ustring filename = get_config_dir("config");

		std::ofstream file(filename.c_str());
		if(file)
		{
			file << data ;
			file.close();
		}
		g_free(data);
	}

	g_key_file_free(m_keyFile);

	return true;
}

/*
 *
 */
bool Config::set_default_value(const Glib::ustring &group, const Glib::ustring &key)
{
	Glib::ustring value;

	if(!get_default_value(group, key, value))
		return false;
	
	//g_key_file_set_string(m_keyFile, group.c_str(), key.c_str(), value.c_str());
	set_value_string(group, key, value);

	return true;
}

/*
 *
 */
bool Config::get_default_value(const Glib::ustring &group, const Glib::ustring &key, Glib::ustring &value)
{
	g_return_val_if_fail(m_keyFile, false);

	std::map<Glib::ustring, std::map<Glib::ustring, Glib::ustring> >::iterator g;

	g = m_default_config.find(group);

	if(g == m_default_config.end())
		return false;

	std::map<Glib::ustring, Glib::ustring>::iterator k = g->second.find(key);

	if(k == g->second.end())
		return false;

	value = k->second;
			
	se_debug_message(SE_DEBUG_APP, "[%s] %s=%s", group.c_str(), key.c_str(), value.c_str());

	return true;
}

/*
 *
 */
bool Config::check_the_key_or_put_default_value(const Glib::ustring &group, const Glib::ustring &key)
{
	if(has_key(group, key))
		return true;

	return set_default_value(group, key);
}

/*
 *
 */
bool Config::set_comment(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%s", group.c_str(), key.c_str(), comment.c_str());
	
	g_key_file_set_comment(m_keyFile, group.c_str(), key.c_str(), comment.c_str(), NULL);
	
	return true;
}

/*
 *
 */
bool Config::has_group(const Glib::ustring &group)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s]", group.c_str());

	bool value = (bool)g_key_file_has_group(m_keyFile, group.c_str());
	return value;

}

/*
 *
 */
bool Config::has_key(const Glib::ustring &group, const Glib::ustring &key)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s", group.c_str(), key.c_str());

	GError *error = NULL;
	bool value = (bool)g_key_file_has_key(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "has not key [%s] %s : %s", group.c_str(), key.c_str(), error->message);
		
		g_error_free(error);
		return false;
	}
	return value;

}

/*
 *
 */
bool Config::get_keys(const Glib::ustring &group, std::list<Glib::ustring> &list)
{
	g_return_val_if_fail(m_keyFile, false);

	
	GError *error = NULL;

	gsize size = 0;

	gchar** value = g_key_file_get_keys(m_keyFile, group.c_str(), &size, &error);
	
	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] failed : %s", group.c_str(), error->message);
	
		g_error_free(error);
		return false;
	}

	for(unsigned int i=0; i<size; ++i)
	{
		list.push_back(Glib::ustring(value[i]));
	}

	g_strfreev(value);

	se_debug_message(SE_DEBUG_APP, "[%s]", group.c_str());

	return true;
}


/*
 *
 */
bool Config::set_value_bool(const Glib::ustring &group, const Glib::ustring &key, const bool &value, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%i", group.c_str(), key.c_str(), value);

	g_key_file_set_boolean(m_keyFile, group.c_str(), key.c_str(), (gboolean)value);

	if(!comment.empty())
		set_comment(group, key, comment);

	emit_signal_changed(group, key, to_string(value));
	return true;
}

/*
 *
 */
bool Config::get_value_bool(const Glib::ustring &group, const Glib::ustring &key, bool &value)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);
	
	GError *error = NULL;
	bool tmp = g_key_file_get_boolean(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);
		g_error_free(error);
		return false;
	}
	
	value = tmp;

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%i", group.c_str(), key.c_str(), value);

	return true;
}

/*
 *
 */
bool Config::set_value_int(const Glib::ustring &group, const Glib::ustring &key, const int &value, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%i", group.c_str(), key.c_str(), value);

	g_key_file_set_integer(m_keyFile, group.c_str(), key.c_str(), value);

	if(!comment.empty())
		set_comment(group, key, comment);

	emit_signal_changed(group, key, to_string(value));
	
	return true;
}

/*
 *
 */
bool Config::get_value_int(const Glib::ustring &group, const Glib::ustring &key, int &value)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	GError *error = NULL;
	
	int tmp = g_key_file_get_integer(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);

		g_error_free(error);
		return false;
	}

	value = tmp;

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%i", group.c_str(), key.c_str(), value);
	
	return true;
}

/*
 *
 */
bool Config::set_value_float(const Glib::ustring &group, const Glib::ustring &key, const float &value, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%f", group.c_str(), key.c_str(), value);

	g_key_file_set_double(m_keyFile, group.c_str(), key.c_str(), (double)value);

	if(!comment.empty())
		set_comment(group, key, comment);

	emit_signal_changed(group, key, to_string(value));
	
	return true;
}

/*
 *
 */
bool Config::get_value_float(const Glib::ustring &group, const Glib::ustring &key, float &value)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	GError *error = NULL;
	
	gdouble tmp = g_key_file_get_double(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);

		g_error_free(error);
		return false;
	}

	value = (float)tmp;

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%f", group.c_str(), key.c_str(), value);
	
	return true;
}

/*
 *
 */
bool Config::set_value_double(const Glib::ustring &group, const Glib::ustring &key, const double &value, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%f", group.c_str(), key.c_str(), value);

	g_key_file_set_double(m_keyFile, group.c_str(), key.c_str(), value);

	if(!comment.empty())
		set_comment(group, key, comment);

	emit_signal_changed(group, key, to_string(value));
	
	return true;
}

/*
 *
 */
bool Config::get_value_double(const Glib::ustring &group, const Glib::ustring &key, double &value)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	GError *error = NULL;
	
	gdouble tmp = g_key_file_get_double(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);

		g_error_free(error);
		return false;
	}

	value = tmp;

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%f", group.c_str(), key.c_str(), value);
	
	return true;
}

/*
 *
 */
bool Config::set_value_string(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &value, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%s", group.c_str(), key.c_str(), value.c_str());

	g_key_file_set_string(m_keyFile, group.c_str(), key.c_str(), value.c_str());

	if(!comment.empty())
		set_comment(group, key, comment);

	emit_signal_changed(group, key, value);
	return true;
}

/*
 *
 */
bool Config::get_value_string(const Glib::ustring &group, const Glib::ustring &key, Glib::ustring &str)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	GError *error = NULL;
	gchar* value = g_key_file_get_string(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);
		g_error_free(error);
		return false;
	}
		
	str = value;
		
	g_free(value);
		
	se_debug_message(SE_DEBUG_APP, "[%s] %s=%s", group.c_str(), key.c_str(), str.c_str());

	return true;
}

/*
 *
 */
bool Config::set_value_color(const Glib::ustring &group, const Glib::ustring &key, const Color &color, const Glib::ustring &comment)
{
	g_return_val_if_fail(m_keyFile, false);

	return set_value_string(group, key, color.to_string(), comment);
}

/*
 *
 */
bool Config::get_value_color(const Glib::ustring &group, const Glib::ustring &key, Color &color)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	Glib::ustring value;
	if(get_value_string(group, key, value))
	{
		color = Color(value);
		return true;
	}

	return false;
}

/*
 *
 */
bool Config::set_value_string_list(const Glib::ustring &group, const Glib::ustring &key, const std::list<Glib::ustring> &list)
{
	g_return_val_if_fail(m_keyFile, false);

	Glib::ustring text;
	std::list<Glib::ustring>::const_iterator it;
	for(it=list.begin(); it!=list.end(); ++it)
	{
		text+=*it;
		text+=";";
	}

	se_debug_message(SE_DEBUG_APP, "[%s] %s=%s", group.c_str(), key.c_str(), text.c_str());

	return set_value_string(group,key,text);
}

/*
 *
 */
bool Config::get_value_string_list(const Glib::ustring &group, const Glib::ustring &key, std::list<Glib::ustring> &list)
{
	g_return_val_if_fail(m_keyFile, false);

	check_the_key_or_put_default_value(group, key);

	GError *error = NULL;

	gsize size;

	gchar** value = g_key_file_get_string_list(m_keyFile, group.c_str(), key.c_str(), &size, &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] %s failed : %s", group.c_str(), key.c_str(), error->message);
		g_error_free(error);
		return false;
	}
	
	for(unsigned int i=0; i<size; ++i)
	{
		list.push_back(Glib::ustring(value[i]));
	}
	
	g_strfreev(value);
		
	return true;
}


/*
 *
 */
bool Config::remove_group(const Glib::ustring &group)
{
	g_return_val_if_fail(m_keyFile, false);

	GError *error = NULL;
	
	g_key_file_remove_group(m_keyFile, group.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "[%s] failed : %s", group.c_str(), error->message);
		g_error_free(error);
		return false;
	}
	se_debug_message(SE_DEBUG_APP, "remove group [%s]", group.c_str());
	return true;
}

/*
 *
 */
bool Config::remove_key(const Glib::ustring &group, const Glib::ustring &key)
{
	g_return_val_if_fail(m_keyFile, false);

	GError *error = NULL;
	
	g_key_file_remove_key(m_keyFile, group.c_str(), key.c_str(), &error);

	if(error)
	{
		se_debug_message(SE_DEBUG_APP, "remove [%s] %s failed : %s", group.c_str(), key.c_str(), error->message);

		g_error_free(error);
		return false;
	}

	se_debug_message(SE_DEBUG_APP, "remove [%s] %s", group.c_str(), key.c_str());
	
	return true;
}

void Config::emit_signal_changed(const Glib::ustring &g, const Glib::ustring &k, const Glib::ustring &v)
{
	m_signals[g](k,v);
}


/*
 *	permet de surveiller un groupe
 *	fonction(key, value)
 */
sigc::signal<void, Glib::ustring, Glib::ustring>& Config::signal_changed(const Glib::ustring &group)
{
	return m_signals[group];
}




/*
 *
 */
bool Config::get_value_bool(const Glib::ustring &group, const Glib::ustring &key)
{
	bool value;

	bool state = get_value_bool(group, key, value);
	
	g_return_val_if_fail(state, false);

	return value;
}

/*
 *
 */
int Config::get_value_int(const Glib::ustring &group, const Glib::ustring &key)
{
	int value;

	bool state = get_value_int(group, key, value);
	
	g_return_val_if_fail(state, 0);

	return value;
}

/*
 *
 */
float Config::get_value_float(const Glib::ustring &group, const Glib::ustring &key)
{
	float value;

	bool state = get_value_float(group, key, value);
	
	g_return_val_if_fail(state, 0);

	return value;
}

/*
 *
 */
double Config::get_value_double(const Glib::ustring &group, const Glib::ustring &key)
{
	double value;

	bool state = get_value_double(group, key, value);
	
	g_return_val_if_fail(state, 0.0);

	return value;
}

/*
 *
 */
Glib::ustring	Config::get_value_string(const Glib::ustring &group, const Glib::ustring &key)
{
	Glib::ustring value;

	bool state = get_value_string(group, key, value);
	
	g_return_val_if_fail(state, Glib::ustring());

	return value;
}

/*
 *
 */
Color	Config::get_value_color(const Glib::ustring &group, const Glib::ustring &key)
{
	Color value;

	bool state = get_value_color(group, key, value);
	
	g_return_val_if_fail(state, Color());

	return value;
}

/*
 *
 */
std::list<Glib::ustring> Config::get_value_string_list(const Glib::ustring &group, const Glib::ustring &key)
{
	std::list<Glib::ustring> value;

	bool state = get_value_string_list(group, key, value);
	
	g_return_val_if_fail(state, value);

	return value;
}

