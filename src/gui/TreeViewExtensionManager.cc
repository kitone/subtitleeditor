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

#include "TreeViewExtensionManager.h"
#include <gtkmm/treemodelfilter.h>

/*
 *
 */
class ColumnExtension : public Gtk::TreeModel::ColumnRecord
{
public:
	ColumnExtension()
	{
		add(active);
		add(label);
		add(info);
	}

	Gtk::TreeModelColumn<bool> active;
	Gtk::TreeModelColumn<Glib::ustring> label;
	Gtk::TreeModelColumn<ExtensionInfo*> info;
};

/*
 * Call automatically create_view().
 */
TreeViewExtensionManager::TreeViewExtensionManager(BaseObjectType *cobject, Glib::RefPtr<Gnome::Glade::Xml>&)
:Gtk::TreeView(cobject)
{
	create_view();
}

/*
 * Call automatically create_view().
 */
TreeViewExtensionManager::TreeViewExtensionManager()
{
	create_view();
}

/*
 * Create column with cell toggle (active state) and text (label and description).
 * All extensions are added to the model.
 */
void TreeViewExtensionManager::create_view()
{
	ColumnExtension m_column;

	set_headers_visible(false);

	m_model = Gtk::ListStore::create(m_column);
	set_model(m_model);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererToggle* cell_toggle = NULL;
	Gtk::CellRendererText* cell_text = NULL;

	// active
	column = manage(new Gtk::TreeViewColumn);
	append_column(*column);

	cell_toggle = manage(new Gtk::CellRendererToggle);
	cell_toggle->signal_toggled().connect(
			sigc::mem_fun(*this, &TreeViewExtensionManager::on_active_toggled));
	column->pack_start(*cell_toggle, false);
	column->add_attribute(cell_toggle->property_active(), m_column.active);


	// label
	column = manage(new Gtk::TreeViewColumn);
	append_column(*column);

	cell_text = manage(new Gtk::CellRendererText);
	column->pack_start(*cell_text, true);
	column->add_attribute(cell_text->property_markup(), m_column.label);


	// property
	set_rules_hint(true);
	

	std::list<ExtensionInfo*> list = ExtensionManager::instance().get_extension_info_list();
	for(std::list<ExtensionInfo*>::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		if((*it)->get_hidden())
			continue;

		Gtk::TreeIter iter = m_model->append();
		(*iter)[m_column.info] = (*it);
		(*iter)[m_column.active] = (*it)->get_active();
		(*iter)[m_column.label] = Glib::ustring::compose("<b>%1</b>\n%2", (*it)->get_label(), (*it)->get_description());
	}
}

/*
 * Filter the model and display only one categorie
 * ExtensionInfo->categorie
 */
void TreeViewExtensionManager::set_filter(const Glib::ustring &categorie)
{
	Glib::RefPtr<Gtk::TreeModelFilter> filter = Gtk::TreeModelFilter::create(get_model());

	filter->set_visible_func(
			sigc::bind(
				sigc::mem_fun(*this, &TreeViewExtensionManager::on_filter_visible), categorie));

	set_model(filter);
}

/*
 * Try to update the active state of the extension.
 */
void TreeViewExtensionManager::on_active_toggled(const Glib::ustring &path)
{
	ColumnExtension m_column;

	Gtk::TreeIter it = m_model->get_iter(path);

	ExtensionInfo* info = (*it)[m_column.info];
	if(info)
	{
		bool active = !info->get_active();

		// Only if the extension manager success
		if(ExtensionManager::instance().set_extension_active(info->get_name(), active))
			(*it)[m_column.active] = active;
	}
}

/*
 * Used by the filter.
 */
bool TreeViewExtensionManager::on_filter_visible(const Gtk::TreeModel::const_iterator &iter, Glib::ustring categorie)
{
	static ColumnExtension column;

	ExtensionInfo *info = (*iter)[column.info];
	if(info)
	{
		if(info->get_categorie() == categorie)
			return true;
	}
	return false;
}

/*
 * Return the current extension selected or NULL.
 */
ExtensionInfo* TreeViewExtensionManager::get_selected_extension()
{
	Gtk::TreeIter it = get_selection()->get_selected();
	if(!it)
		return NULL;
	
	ColumnExtension column;
	return (*it)[column.info];
}

