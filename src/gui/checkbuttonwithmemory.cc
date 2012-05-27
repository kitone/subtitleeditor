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

#include "checkbuttonwithmemory.h"
#include "cfg.h"

/*
 * Constructor
 * - accepts identifiers used to retreive and store the button state in Config 
 */
CheckButtonWithMemory::CheckButtonWithMemory( const Glib::ustring &group, const Glib::ustring &key )
:group_name(group), key_name(key)
{
	init_state();

	// m_button_toggled 
	m_button_toggled = signal_toggled().connect(
			sigc::mem_fun(*this, &CheckButtonWithMemory::on_button_toggled));
}

/*
 * Constructor
 */
CheckButtonWithMemory::CheckButtonWithMemory(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
:Gtk::CheckButton(cobject), group_name(), key_name()
{
	// m_button_toggled
	m_button_toggled = signal_toggled().connect(
			sigc::mem_fun(*this, &CheckButtonWithMemory::on_button_toggled));
}

void CheckButtonWithMemory::on_button_toggled()
{
	bool state = CheckButton::get_active();
/*
	printf("duh boottain is ");
	if( state )
		printf("oowon.\n");
	else
		printf("wuuooff.\n");
*/
	Config::getInstance().set_value_bool(group_name, key_name, state);
}

/*
 * set config group and key names
 */
void CheckButtonWithMemory::link_to_cfg(const Glib::ustring &group, const Glib::ustring &key, bool def_state )
{
	group_name = group;
	key_name = key;
	Config &cfg = Config::getInstance();

	//printf("linking to group %s, key %s\n", group_name.c_str(), key_name.c_str() );

	if( !cfg.has_key( group_name, key_name ) )
	{
		//set default value
		//printf("setting default value.");
		cfg.set_value_bool( group_name, key_name, def_state );
	};
}

/*
 * Sets current state - overloading a ToggleButton method
 */
void CheckButtonWithMemory::set_active(bool state)
{
	printf("setting state to ");
	if( state )
		printf("on.\n");
	else
		printf("off.\n");

	CheckButton::set_active(state);
	state = CheckButton::get_active();
	Config::getInstance().set_value_bool(group_name, key_name, state);
}

/*
 * Recall stored state from the Config
 */
void CheckButtonWithMemory::init_state()
{

	bool state = Config::getInstance().get_value_bool( group_name, key_name );
	CheckButton::set_active( state );
}

