#ifndef _TreeViewExtensionManager_h
#define _TreeViewExtensionManager_h

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

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include "ExtensionManager.h"
#include <libglademm/xml.h>

/*
 * Help to create a view to manage the extension.
 */
class TreeViewExtensionManager : public Gtk::TreeView
{
public:

	/*
	 * Call automatically create_view().
	 */
	TreeViewExtensionManager(BaseObjectType *cobject, Glib::RefPtr<Gnome::Glade::Xml>&);

	/*
	 * Call automatically create_view().
	 */
	TreeViewExtensionManager();

	/*
	 * Filter the model and display only one categorie
	 * ExtensionInfo->categorie
	 */
	void set_filter(const Glib::ustring &categorie);

	/*
	 * Return the current extension selected or NULL.
	 */
	ExtensionInfo* get_selected_extension();

protected:
	
	/*
	 * Create column with cell toggle (active state) and text (label and description).
	 * All extensions are added to the model.
	 */
	void create_view();

	/*
	 * Try to update the active state of the extension.
	 */
	void on_active_toggled(const Glib::ustring &path);

	/*
	 * Used by the filter.
	 */
	bool on_filter_visible(const Gtk::TreeModel::const_iterator &iter, Glib::ustring categorie);

protected:
	Glib::RefPtr<Gtk::ListStore> m_model;
};

#endif//_TreeViewExtensionManager_h
