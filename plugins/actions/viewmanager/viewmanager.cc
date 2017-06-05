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

#include <extension/action.h>
#include <utility.h>
#include <gtkmm_utility.h>
#include <memory>

/*
 *
 */
class DialogViewEdit : public Gtk::Dialog
{
	class ColumnRecord : public Gtk::TreeModel::ColumnRecord
	{
	public:
		ColumnRecord()
		{
			add(display);
			add(name);
			add(label);
		}

		Gtk::TreeModelColumn<bool> display;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> label;
	};

public:

	DialogViewEdit(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);
		
		builder->get_widget("treeview-columns", m_treeview);

		create_treeview();
	}

	/*
	 * Update the treeview with the columns displayed.
	 * Add remaining columns that are not displayed.
	 * Run the dialog and update columns_displayed.
	 */
	void execute(Glib::ustring &columns_displayed)
	{
		std::vector<std::string> array;
		utility::split(columns_displayed, ';', array);

		for(unsigned int i=0; i< array.size(); ++i)
		{
			Gtk::TreeIter iter = m_liststore->append();
			(*iter)[m_column_record.name] = array[i];
			(*iter)[m_column_record.label] = SubtitleView::get_column_label_by_name(array[i]);
			(*iter)[m_column_record.display] = true;
		}

		// add other columns
		{
			std::list<Glib::ustring> all_columns;
			
			Config::getInstance().get_value_string_list("subtitle-view", "columns-list", all_columns);
			
			for(std::list<Glib::ustring>::const_iterator it = all_columns.begin(); it != all_columns.end(); ++it)
			{
				if(std::find(array.begin(), array.end(), *it) == array.end())
				{
					Gtk::TreeIter iter = m_liststore->append();
					(*iter)[m_column_record.name] = *it;
					(*iter)[m_column_record.label] = SubtitleView::get_column_label_by_name(*it);
					(*iter)[m_column_record.display] = false;
				}
			}
		}

		run();

		// get the new columns order
		{
			Glib::ustring columns_updated;

			Gtk::TreeNodeChildren rows = m_liststore->children();
		
			if(!rows.empty())
			{
				for(Gtk::TreeIter it=rows.begin(); it; ++it)
				{
					if((*it)[m_column_record.display] == true)
						columns_updated += (*it)[m_column_record.name] + ";";
				}
			}
			columns_displayed = columns_updated;
		}
	}

protected:
	
	/*
	 * Create the treeview with two columns : Display and Name
	 * Support DND .ui).
	 */
	void create_treeview()
	{
		m_liststore = Gtk::ListStore::create(m_column_record);
		m_treeview->set_model(m_liststore);
		
		// column display
		{
			Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("Display")));
			m_treeview->append_column(*column);

			Gtk::CellRendererToggle* toggle = manage(new Gtk::CellRendererToggle);
			column->pack_start(*toggle);
			column->add_attribute(toggle->property_active(), m_column_record.display);

			toggle->signal_toggled().connect(
					sigc::mem_fun(*this, &DialogViewEdit::on_display_toggled));
		}
		// column label
		{
			Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("Name")));
			m_treeview->append_column(*column);

			Gtk::CellRendererText* label = manage(new Gtk::CellRendererText);
			column->pack_start(*label);
			column->add_attribute(label->property_text(), m_column_record.label);
		}
	}

	/*
	 * Toggle the state of the displayed column
	 */
	void on_display_toggled(const Glib::ustring &path)
	{
		Gtk::TreeIter iter = m_liststore->get_iter(path);
		if(iter)
		{
			bool state = (*iter)[m_column_record.display];

			(*iter)[m_column_record.display] = !state;
		}
	}

protected:
	ColumnRecord m_column_record;
	Gtk::TreeView* m_treeview;
	Glib::RefPtr<Gtk::ListStore> m_liststore;
};

/*
 *
 */
class DialogViewManager : public Gtk::Dialog
{
	class ColumnRecord : public Gtk::TreeModel::ColumnRecord
	{
	public:
		ColumnRecord()
		{
			add(name);
			add(columns);
		}

		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> columns;
	};

public:

	DialogViewManager(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		builder->get_widget("treeview", m_treeview);
		builder->get_widget("button-add", m_buttonAdd);
		builder->get_widget("button-remove", m_buttonRemove);
		builder->get_widget("button-edit", m_buttonEdit);

		m_buttonAdd->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogViewManager::on_add));
		m_buttonRemove->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogViewManager::on_remove));
		m_buttonEdit->signal_clicked().connect(
				sigc::mem_fun(*this, &DialogViewManager::on_edit));

		create_treeview();
		init_treeview();
	}

	/*
	 *
	 */
	void execute()
	{
		run();

		// save to the configuration
		save_to_config();
	}

protected:

	/*
	 *
	 */
	void create_treeview()
	{
		m_liststore = Gtk::ListStore::create(m_column_record);
		m_treeview->set_model(m_liststore);

		Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("Name")));
		m_treeview->append_column(*column);

		Gtk::CellRendererText* name = manage(new Gtk::CellRendererText);
		column->pack_start(*name);
		column->add_attribute(name->property_text(), m_column_record.name);

		name->property_editable() = true;
		name->signal_edited().connect(
				sigc::mem_fun(*this, &DialogViewManager::on_name_edited));

		m_treeview->get_selection()->signal_changed().connect(
				sigc::mem_fun(*this, &DialogViewManager::on_selection_changed));
	}

	/*
	 *
	 */
	void init_treeview()
	{
		std::list<Glib::ustring> keys;

		Config::getInstance().get_keys("view-manager", keys);

		std::list<Glib::ustring>::const_iterator it;
		for(it = keys.begin(); it != keys.end(); ++it)
		{
			Glib::ustring columns = Config::getInstance().get_value_string("view-manager", *it);
			
			Gtk::TreeIter iter = m_liststore->append();

			(*iter)[m_column_record.name] = *it;
			(*iter)[m_column_record.columns] = columns;
		}

		Gtk::TreeIter iter = m_liststore->get_iter("0");
		if(iter)
			m_treeview->get_selection()->select(iter);
		else
			on_selection_changed();
	}

	/*
	 *
	 */
	void on_name_edited(const Glib::ustring &path, const Glib::ustring &text)
	{
		Gtk::TreeIter iter = m_liststore->get_iter(path);

		(*iter)[m_column_record.name] = text;
	}

	/*
	 *
	 */
	void on_selection_changed()
	{
		bool state = m_treeview->get_selection()->get_selected();

		m_buttonRemove->set_sensitive(state);
		m_buttonEdit->set_sensitive(state);
	}

	/*
	 *
	 */
	void on_add()
	{
		Gtk::TreeIter iter = m_liststore->append();

		(*iter)[m_column_record.name] = _("Untitled");

		m_treeview->set_cursor(m_liststore->get_path(*iter), *m_treeview->get_column(0), true);
	}

	/*
	 *
	 */
	void on_remove()
	{
		Gtk::TreeIter selected = m_treeview->get_selection()->get_selected();
		if(selected)
		{
			Glib::ustring name = (*selected)[m_column_record.name];
			selected = m_liststore->erase(selected);
			if(selected)
				m_treeview->get_selection()->select(selected);
		}
	}

	/*
	 * Edit the selected item, launch the dialog edit
	 */
	void on_edit()
	{
		Gtk::TreeIter selected = m_treeview->get_selection()->get_selected();
		if(selected)
		{
			std::unique_ptr<DialogViewEdit> dialog(
					gtkmm_utility::get_widget_derived<DialogViewEdit>(
							SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
							"dialog-view-manager.ui", 
							"dialog-view-edit"));
			
			Glib::ustring columns = (*selected)[m_column_record.columns];

			dialog->execute(columns);
			// updated with the new columns displayed
			(*selected)[m_column_record.columns] = columns;
		}
	}

	/*
	 * Delete the group "view-manager" and create with the new values
	 */
	void save_to_config()
	{
		Config::getInstance().remove_group("view-manager");

		Gtk::TreeNodeChildren rows = m_liststore->children();
		
		if(!rows.empty())
		{
			for(Gtk::TreeIter it = rows.begin(); it; ++it)
			{
				Glib::ustring name = (*it)[m_column_record.name];
				Glib::ustring columns = (*it)[m_column_record.columns];

				Config::getInstance().set_value_string("view-manager", name, columns);
			}
		}
	}

protected:
	ColumnRecord m_column_record;
	Gtk::TreeView* m_treeview;
	Glib::RefPtr<Gtk::ListStore> m_liststore;
	Gtk::Button* m_buttonAdd;
	Gtk::Button* m_buttonRemove;
	Gtk::Button* m_buttonEdit;
};

/*
 *
 */
class ViewManagerPlugin : public Action
{
public:

	ViewManagerPlugin()
	{
		activate();
		update_ui();
	}

	~ViewManagerPlugin()
	{
		deactivate();
	}
	
	/*
	 * First check if the user has any preferences
	 */
	void check_config()
	{
		std::list<Glib::ustring> keys;

		if(get_config().get_keys("view-manager", keys) && !keys.empty())
				return;

		Config &cfg = get_config();

		cfg.set_value_string("view-manager", _("Simple"), "number;start;end;duration;text");
		cfg.set_value_string("view-manager", _("Advanced"), "number;start;end;duration;style;name;text");
		cfg.set_value_string("view-manager", _("Translation"), "number;text;translation");
		cfg.set_value_string("view-manager", _("Timing"), "number;start;end;duration;cps;text");
	}

	/*
	 *
	 */
	void activate()
	{
		check_config();

		action_group = Gtk::ActionGroup::create("ViewManagerPlugin");

		std::list<Glib::ustring> keys;

		// user settings
		get_config().get_keys("view-manager", keys);

		for(std::list<Glib::ustring>::const_iterator it = keys.begin(); it != keys.end(); ++it)
		{
			Glib::ustring name = *it;

			action_group->add(
				Gtk::Action::create(name, name, _("Switches to this view")),
					sigc::bind( sigc::mem_fun(*this, &ViewManagerPlugin::on_set_view), name));			
		}

		// Set View...
		action_group->add(
			Gtk::Action::create("view-manager-preferences", Gtk::Stock::PREFERENCES, _("View _Manager"), _("Manage the views")),
					sigc::mem_fun(*this, &ViewManagerPlugin::on_view_manager));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();
		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-view' action='menu-view'>"
			"			<placeholder name='view-manager'>"
			"				<placeholder name='placeholder'/>"
			"				<menuitem action='view-manager-preferences'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";
		
		ui_id = get_ui_manager()->add_ui_from_string(submenu);

		// create items for the user view
		for(std::list<Glib::ustring>::const_iterator it = keys.begin(); it != keys.end(); ++it)
		{
			ui->add_ui(ui_id, "/menubar/menu-view/view-manager/placeholder", *it, *it, Gtk::UI_MANAGER_AUTO, false);
		}
		get_ui_manager()->ensure_update();
	}

	/*
	 *
	 */
	void deactivate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->remove_ui(ui_id);
		ui->remove_action_group(action_group);
	}

	/*
	 * Updates the configuration with the columns to display.
	 */
	void on_set_view(const Glib::ustring &name)
	{
		Glib::ustring columns = get_config().get_value_string("view-manager", name);

		get_config().set_value_string("subtitle-view", "columns-displayed", columns);
	}

	/*
	 *
	 */
	void on_view_manager()
	{
		std::unique_ptr<DialogViewManager> dialog(
				gtkmm_utility::get_widget_derived<DialogViewManager>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
						"dialog-view-manager.ui", 
						"dialog-view-manager"));

		dialog->execute();

		deactivate();
		activate();
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(ViewManagerPlugin)
