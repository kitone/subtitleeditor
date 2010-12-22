/*
 *	subtitleeditor -- a tool to create or edit subtitle
 *
 *	http://home.gna.org/subtitleeditor/
 *	https://gna.org/projects/subtitleeditor/
 *
 *	Copyright @ 2005-2010, kitone
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
#include <gtkmm/builder.h>
#include <gtk/gtk.h>
#include <extension/action.h>
#include <utility.h>
#include <gtkmm_utility.h>
#include <memory>

/*
 */
static gboolean accel_find_func (GtkAccelKey *key, GClosure *closure, gpointer data)
{
  return (GClosure *) data == closure;
}

/*
 */
class DialogConfigureKeyboardShortcuts : public Gtk::Dialog
{
	class Columns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		Columns()
		{
			add(label);
			add(action);
			add(stock_id);
			add(shortcut);
			add(closure);
		}

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
	DialogConfigureKeyboardShortcuts(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder>& builder)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		builder->get_widget("treeview", m_treeview);

		create_treeview();
	}
	
	/*
	 * Create columns Actions and Shortcut.
	 */
	void create_treeview()
	{
		m_store = Gtk::ListStore::create(m_columns);

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

		// tooltip
		m_treeview->set_has_tooltip(true);
		m_treeview->signal_query_tooltip().connect(
				sigc::mem_fun(*this, &DialogConfigureKeyboardShortcuts::on_query_tooltip));
	}

	/*
	 * Create all items (action) from the action_group.
	 * The action with menu in the name are ignored.
	 */
	void create_items()
	{
		std::vector< Glib::RefPtr<Gtk::ActionGroup> > group = m_refUIManager->get_action_groups();
		for(unsigned int i=0; i < group.size(); ++i)
		{
			std::vector<Glib::RefPtr<Gtk::Action> > actions = group[i]->get_actions();

			for(unsigned int j=0; j < actions.size(); ++j)
			{
				if(actions[j]->get_name().find("menu") != Glib::ustring::npos)
					continue;

				add_action(actions[j]);
			}
		}
	}

	/*
	 * Add an action in the model.
	 */
	void add_action(Glib::RefPtr<Gtk::Action> action)
	{
		Gtk::TreeModel::Row row = *m_store->append();

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
	 * Show tooltip.
	 */
	bool on_query_tooltip(int x, int y, bool keyboard_tooltip, const Glib::RefPtr<Gtk::Tooltip>& tooltip)
	{
		Gtk::TreeIter iter;
		if(m_treeview->get_tooltip_context_iter(x,y, keyboard_tooltip, iter) == false)
			return false;

		Glib::RefPtr<Gtk::Action> ptr = (*iter)[m_columns.action];
		if(!ptr)
			return false;

		Glib::ustring tip = ptr->property_tooltip();
	
		tooltip->set_markup(tip);
		
		Gtk::TreePath path = m_store->get_path(iter);

		m_treeview->set_tooltip_row(tooltip, path);
		return true;
	}

	/*
	 *
	 */
	bool foreach_callback_label(const Gtk::TreePath &path, const Gtk::TreeIter &iter, const Glib::ustring &label, Gtk::TreeIter *result)
	{
		Glib::ustring ak = (*iter)[m_columns.shortcut];

		if(label != ak)
			return false;

		*result = iter;
		return true;
	}

	/*
	 *
	 */
	bool foreach_callback_closure(const Gtk::TreePath &path, const Gtk::TreeIter &iter, const GClosure *closure, Gtk::TreeIter *result)
	{
		GClosure *c = (*iter)[m_columns.closure];

		if(closure != c)
			return false;

		*result = iter;
		return true;
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
	 * Try to changed the shortcut with conflict support.
	 */
	void on_accel_edited(const Glib::ustring& path, guint key, Gdk::ModifierType mods, guint keycode)
	{
		Gtk::TreeIter iter = m_store->get_iter(path);

		Glib::RefPtr<Gtk::Action> action = (*iter)[m_columns.action];

		if(!action)
			return;

		if(!key)
		{
			dialog_error(_("Invalid shortcut."), "");
			return;
		}

		if(Gtk::AccelMap::change_entry(action->get_accel_path(), key, mods, false) == false)
		{
			// We try to find if there's already an another action with the same shortcut
			Glib::RefPtr<Gtk::Action> conflict_action = get_action_by_accel(key, mods);
			
			if(conflict_action == action)
				return;

			if(conflict_action)
			{
				Glib::ustring shortcut = Gtk::AccelGroup::get_label(key, mods);
				Glib::ustring label_conflict_action = conflict_action->property_label();

				utility::replace(label_conflict_action, "_", "");

				Glib::ustring message = Glib::ustring::compose(
						Glib::ustring(_("Shortcut \"%1\" is already taken by \"%2\".")), 
						shortcut, label_conflict_action);
				
				Glib::ustring secondary = Glib::ustring::compose(	
						Glib::ustring(_("Reassigning the shortcut will cause it to be removed from \"%1\".")), 
							label_conflict_action);

				Gtk::MessageDialog dialog(*this, message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true);
				dialog.set_title(_("Conflicting Shortcuts"));
				dialog.set_secondary_text(secondary);
						
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
	 * Remove the shortcut.
	 */
	void on_accel_cleared(const Glib::ustring &path)
	{
		Gtk::TreeIter iter = m_store->get_iter(path);

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

		create_items();

		run();
	}

protected:
	Columns			m_columns;
	Gtk::TreeView* m_treeview;
	Glib::RefPtr<Gtk::ListStore> m_store;
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

		std::auto_ptr<DialogConfigureKeyboardShortcuts> dialog(
				gtkmm_utility::get_widget_derived<DialogConfigureKeyboardShortcuts>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_UI, SE_PLUGIN_PATH_DEV),
						"dialog-configure-keyboard-shortcuts.ui", 
						"dialog-configure-keyboard-shortcuts"));

		dialog->execute(get_ui_manager());
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(ConfigureKeyboardShortcuts)
