#ifndef _PreferencePage_h
#define _PreferencePage_h

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
#include <gtkmm.h>
#include <libglademm/xml.h>
#include <widget_config_utility.h>

class PreferencePage : public Gtk::VBox
{
public:
	
	/*
	 *
	 */
	PreferencePage(BaseObjectType *cobject)
	:Gtk::VBox(cobject)
	{
	}

	/*
	 * Get widget from glade::xml and init/connect with config.
	 */
	Gtk::Widget* init_widget( 
			const Glib::RefPtr<Gnome::Glade::Xml>& refGlade, 
			const Glib::ustring &widget_name, 
			const Glib::ustring &config_group,
			const Glib::ustring &config_key)
	{
		Gtk::Widget *widget =NULL;

		refGlade->get_widget(widget_name, widget);

		widget_config::read_config_and_connect(widget, config_group, config_key);

		return widget;
	}
	
	/*
	 * Get widget from glade::xml and init/connect with config.
	 */
	template<class W>
	W* init_widget_derived( 
			const Glib::RefPtr<Gnome::Glade::Xml>& refGlade, 
			const Glib::ustring &widget_name, 
			const Glib::ustring &config_group,
			const Glib::ustring &config_key)
	{
		W *widget =NULL;

		refGlade->get_widget_derived(widget_name, widget);

		widget_config::read_config_and_connect(widget, config_group, config_key);

		return widget;
	}
	
	
};

#endif//_PreferencePage_h
