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
 

#include "CheckErrors.h"
#include "Config.h"

/*
 *
 */
CheckErrorPlugin::CheckErrorPlugin(
										const Glib::ustring &id,
										const Glib::ustring &name, 
										const Glib::ustring &description, 
										const Glib::ustring &default_color)
:m_id(id), m_name(name), m_description(description), m_color(default_color)
{
	Config::getInstance().get_value_string("check-error-plugins", id + "-color", m_color);
}

/*
 *
 */
CheckErrorPlugin::~CheckErrorPlugin()
{
}

/*
 *
 */
Glib::ustring CheckErrorPlugin::get_name()
{
	return m_name;
}

/*
 *
 */
Glib::ustring CheckErrorPlugin::get_description()
{
	return m_description;
}

/*
 *
 */
Glib::ustring CheckErrorPlugin::get_color()
{
	return m_color;
}

