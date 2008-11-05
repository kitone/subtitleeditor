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
 
#include <gtkmm/accelmap.h>
#include <libglademm/xml.h>
#include <gtk/gtk.h>
#include <libxml++/libxml++.h>
#include <extension/Action.h>
#include <utility.h>


/*
 *
 */
static gboolean accel_find_func (GtkAccelKey *key, GClosure *closure, gpointer data)
{
  return (GClosure *) data == closure;
}

/*
 *
 */
class DialogConfigureKeyboardShortcuts : public Gtk::Dialog
{
	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns()
		{
			add(can_be_modified);
			add(label);
			add(action);
			add(stock_id);
			add(shortcut);
			add(closure);
		}

		Gtk::TreeModelColumn<bool> can_be_modified;
		Gtk::TreeModelColumn< Glib::RefPtr<Gtk::Action> > action;
		Gtk::TreeModelColumn<Glib::ustring> stock_id;
		Gtk::TreeModelColumn<Glib::ustring> label;
		Gtk::TreeModelColumn<Glib::ustring> shortcut;
		Gtk::TreeModelColumn<GClosure*> closure;
	};

public:
	/*
	 *
	 */
	DialogConfigureKeyboardShortcuts(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		refGlade->get_widget("treeview", m_treeview);

		create_treeview();
	}
	
	/*
	 *
	 */
	void create_treeview()
	{
		m_store = Gtk::TreeStore::create(m_columns);

		m_treeview->set_model(m_store);

		// actions
		{
			Gtk::TreeViewColumn* column = NULL;
			Gtk::CellRendererPixbuf* pixbuf = NULL;
			Gtk::CellRendererText* text = NULL;

			column = manage(new Gtk::TreeViewColumn(_("Actions")));

			// pixbuf
			pixbuf = manage(new Gtk::CellRendererPixbuf);
			column->pack_start(*pixbuf, false);
			column->add_attribute(pixbuf->property_stock_id(), m_columns.stock_id);

			// label
			text = manage(new Gtk::CellRendererText);
			column->pack_start(*text, true);
			column->add_attribute(text->property_text(), m_columns.label);
			
			column->set_expand(true);

			m_treeview->append_column(*column);
		}

		// shortcut
		{
			Gtk::TreeViewColumn* column = NULL;
			Gtk::CellRendererAccel* accel = NULL;

			column = manage(new Gtk::TreeViewColumn(_("Shortcut")));

			// shortcut
			accel = manage(new Gtk::CellRendererAccel);
			accel->property_editable() = true;
			accel->signal_accel_edited().connect(
					sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::on_accel_edited));
			accel->signal_accel_cleared().connect(
					sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::on_accel_cleared));

			column->pack_start(*accel, false);
			column->add_attribute(accel->property_text(), m_columns.shortcut);

			m_treeview->append_column(*column);
		}
	}

	/*
	 *
	 */
	bool foreach_callback_label(const Gtk::TreePath &path, const Gtk::TreeIter &iter, const Glib::ustring &label, Gtk::TreeIter *result)
	{
		Glib::ustring ak = (*iter)[m_columns.shortcut];

		if(label == ak)
		{
			*result = iter;
			return true;
		}

		return false;
	}

	/*
	 *
	 */
	bool foreach_callback_closure(const Gtk::TreePath &path, const Gtk::TreeIter &iter, const GClosure *closure, Gtk::TreeIter *result)
	{
		GClosure *c = (*iter)[m_columns.closure];

		if(closure == c)
		{
			*result = iter;
			return true;
		}

		return false;
	}

	/*
	 *	search iterator by accelerator
	 */
	Gtk::TreeIter get_iter_by_accel(guint keyval, Gdk::ModifierType mods)
	{
		Glib::ustring label = Gtk::AccelGroup::get_label(keyval, mods);

		Gtk::TreeIter result;
		m_store->foreach(sigc::bind(sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::foreach_callback_label), label, &result));

		return result;
	}

	/*
	 *	search action by an accelerator
	 */
	Glib::RefPtr<Gtk::Action> get_action_by_accel(guint keyval, Gdk::ModifierType mods)
	{
		Gtk::TreeIter result = get_iter_by_accel(keyval, mods);

		Glib::RefPtr<Gtk::Action> res;

		if(result)
			res = (*result)[m_columns.action];

		return res;
	}

	/*
	 *
	 */
	bool on_accel_changed_foreach(const Gtk::TreePath &path, const Gtk::TreeIter &iter, GClosure* accel_closure)
	{
		GClosure *closure = (*iter)[m_columns.closure];

		if(accel_closure == closure)
		{
			guint key = 0;
			Gdk::ModifierType mods = (Gdk::ModifierType)0;
				
			GtkAccelKey *ak = gtk_accel_group_find(m_refUIManager->get_accel_group()->gobj(), accel_find_func, accel_closure);
				
			if(ak && ak->accel_key)
			{
				key = ak->accel_key;
				mods = (Gdk::ModifierType)ak->accel_mods;
			}

			(*iter)[m_columns.shortcut] = Gtk::AccelGroup::get_label(key, mods);

			return true;
		}

		return false;
	}

	/*
	 *
	 */
	void on_accel_changed(guint keyval, Gdk::ModifierType modifier, GClosure* accel_closure)
	{
		m_store->foreach(sigc::bind(sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::on_accel_changed_foreach), accel_closure));
	}

	/*
	 *
	 */
	void on_accel_edited(const Glib::ustring& path, guint key, Gdk::ModifierType mods, guint keycode)
	{
		Gtk::TreeIter iter = m_store->get_iter(path);

		if((*iter)[m_columns.can_be_modified] == false)
			return;

		Glib::RefPtr<Gtk::Action> action = (*iter)[m_columns.action];

		if(!action)
			return;

		if(!key)
		{
			dialog_error(_("Invalid shortcut."), "");
		}
		else if(!Gtk::AccelMap::change_entry(action->get_accel_path(), key, mods, false))
		{
			Glib::RefPtr<Gtk::Action> conflict_action = get_action_by_accel(key, mods);
			
			if(conflict_action == action)
				return;

			if(conflict_action)
			{
				Glib::ustring ak = Gtk::AccelGroup::get_label(key, mods);
				Glib::ustring label_conflict_action = conflict_action->property_label();


				Glib::ustring msg;

				msg += "<span weight=\"bold\" size=\"larger\">";
				msg += build_message(_("Shortcut \"%s\" is already taken by \"%s\"."), 
									ak.c_str(), label_conflict_action.c_str());
				msg += "</span>\n\n";
				msg += build_message(_("Reassigning the shortcut will cause it to be removed from \"%s\"."), label_conflict_action.c_str());

				Gtk::MessageDialog dialog(msg, true, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true);
				dialog.set_title(_("Conflicting Shortcuts"));

				if( dialog.run() == Gtk::RESPONSE_OK)
				{
					if(!Gtk::AccelMap::change_entry(action->get_accel_path(), key, mods, true))
					{
						dialog_error(_("Changing shortcut failed."), "");
					}
				}
			}
			else
			{
				dialog_error("Changing shortcut failed.", "");
			}
		}
	}

	/*
	 *
	 */
	void on_accel_cleared(const Glib::ustring &path)
	{
		Gtk::TreeIter iter = m_store->get_iter(path);

		if((*iter)[m_columns.can_be_modified] == false)
			return;

		Glib::RefPtr<Gtk::Action> action = (*iter)[m_columns.action];

		if(!action)
			return;

		if(Gtk::AccelMap::change_entry(action->get_accel_path(), 0, (Gdk::ModifierType)0, false))
		{
			(*iter)[m_columns.shortcut] = Glib::ustring();
		}
		else
			dialog_error(_("Removing shortcut failed."), "");
	}

	/*
	 *
	 */
	void execute(Glib::RefPtr<Gtk::UIManager> ui)
	{
		m_refUIManager = ui;

		ui->get_accel_group()->signal_accel_changed().connect(
				sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::on_accel_changed));

		load_ui();

		run();
	}

	/*
	 *
	 */
	void set_action(Gtk::TreeModel::Row row, Glib::RefPtr<Gtk::Action> action, bool can_be_modified)
	{
		// can be modified ?
		row[m_columns.can_be_modified] = can_be_modified;
		// action
		row[m_columns.action] = action;
		// stock id
		row[m_columns.stock_id] = Gtk::StockID(action->property_stock_id()).get_string();
		// label
		Glib::ustring label = Glib::ustring(action->property_label());
		utility::replace(label, "_", "");
		row[m_columns.label] = label;
		
		// shortcut
		GClosure *accel_closure = gtk_action_get_accel_closure (action->gobj());
		if(accel_closure)
		{
			// closure
			row[m_columns.closure] = accel_closure;

			GtkAccelKey *key = gtk_accel_group_find(m_refUIManager->get_accel_group()->gobj(), accel_find_func, accel_closure);
			if(key && key->accel_key)
			{
				row[m_columns.shortcut] = Gtk::AccelGroup::get_label(key->accel_key, (Gdk::ModifierType)key->accel_mods);
			}
		}
	}

	/*
	 *
	 */
	Glib::RefPtr<Gtk::Action> get_action(const Glib::ustring &name)
	{
		std::vector<Glib::RefPtr<Gtk::ActionGroup> > ags = m_refUIManager->get_action_groups();
		for(unsigned int i=0; i < ags.size(); ++i)
		{
			Glib::RefPtr<Gtk::Action> action = ags[i]->get_action(name);
			if(action)
				return action;
		}
		
		return Glib::RefPtr<Gtk::Action>(NULL);
	}
	/*
	 *
	 */
	void parse_node(const xmlpp::Node *node, Gtk::TreeModel::Row &root)
	{
		std::string name = node->get_name();

		if(name == "menuitem")
		{
			const xmlpp::Element *item = dynamic_cast<const xmlpp::Element*>(node);
		
			const xmlpp::Attribute *action = item->get_attribute("action");
			
			if(!action)
				return;
			
			std::string action_name = action->get_value();

			Gtk::TreeModel::Row row = (*m_store->append(root.children()));
				
			set_action(row, get_action(action_name), true);
		}
		else if(name == "menu")
		{
			const xmlpp::Element *item = dynamic_cast<const xmlpp::Element*>(node);
		
			const xmlpp::Attribute *action = item->get_attribute("action");
			
			if(!action)
				return;

			std::string action_name = action->get_value();

			Gtk::TreeModel::Row row_menu = (root) ? (*m_store->append(root.children())) : (*m_store->append());
	
			set_action(row_menu, get_action(action_name), false);

			const xmlpp::Node::NodeList list = node->get_children();
			xmlpp::Node::NodeList::const_iterator it;
	
			for(it = list.begin(); it != list.end(); ++it)
				parse_node(*it, row_menu);
		}
		else
		{
			const xmlpp::Node::NodeList list = node->get_children();
			xmlpp::Node::NodeList::const_iterator it;
	
			for(it = list.begin(); it != list.end(); ++it)
				parse_node(*it, root);
		}
	}
	
	/*
	 *
	 */
	void load_ui()
	{
		xmlpp::DomParser parser;
		parser.set_substitute_entities();
		parser.parse_file(get_share_dir("menubar.xml"));

		g_return_if_fail(parser);

		const xmlpp::Node* root = parser.get_document()->get_root_node();

		Gtk::TreeModel::Row it;
		parse_node(root, it);
	}
protected:
	Columns			m_columns;
	Gtk::TreeView* m_treeview;
	Glib::RefPtr<Gtk::TreeStore> m_store;
	Glib::RefPtr<Gtk::UIManager> m_refUIManager;
};


/*
 *
 */
class ConfigureKeyboardShortcuts : public Action
{
public:

	ConfigureKeyboardShortcuts()
	{
		activate();
		update_ui();
	}

	~ConfigureKeyboardShortcuts()
	{
		deactivate();
	}

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("ConfigureKeyboardShortcuts");

		action_group->add(
				Gtk::Action::create("configure-keyboard-shortcuts", _("Configure _Keyboard Shortcuts"), _("Configure Keyboard Shortcuts")), 
					sigc::mem_fun(*this, &ConfigureKeyboardShortcuts::on_configure));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-options/configure-keyboard-shortcuts", "configure-keyboard-shortcuts", "configure-keyboard-shortcuts");
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

protected:

	/*
	 *
	 */
	void on_configure()
	{
		se_debug(SE_DEBUG_PLUGINS);

		DialogConfigureKeyboardShortcuts *dialog = 
			utility::get_widget_derived<DialogConfigureKeyboardShortcuts>(
					(Glib::getenv("SE_DEV") == "") ? SE_PLUGIN_PATH_GLADE : SE_PLUGIN_PATH_DEV,
					"dialog-configure-keyboard-shortcuts.glade", 
					"dialog-configure-keyboard-shortcuts");

		dialog->execute(get_ui_manager());

		delete dialog;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(ConfigureKeyboardShortcuts)
