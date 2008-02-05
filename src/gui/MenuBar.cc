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
 

#include "MenuBar.h"
#include "utility.h"
#include <gtkmm/accelmap.h>
#include "DocumentSystem.h"
#include "Config.h"

#include "ActionSystem.h"
#include "gui/PreferencesUI.h"

/*
 *
 */
class ActionGroup
{
public:
	ActionGroup(const Glib::ustring &name, Glib::RefPtr<Gtk::UIManager> ui)
	{
		ag = Gtk::ActionGroup::create(name);
		ui->insert_action_group(ag);
	}


	void menu_item(const Glib::ustring &name, const Glib::ustring &label = Glib::ustring())
	{
		ag->add(Gtk::Action::create(name, label));
	}


	void menu_item(const Glib::ustring &name, const Gtk::StockID &stock_id, const Glib::ustring &label = Glib::ustring())
	{
		ag->add(Gtk::Action::create(name, stock_id, label));
	}

	void item(	const Glib::ustring &name, 
							const Gtk::StockID &stock_id, 
							const Glib::ustring &label = Glib::ustring(), 
							const Glib::ustring &tooltip = Glib::ustring(), 
							const Glib::ustring &accel = Glib::ustring())
	{
		if(accel.empty())
			ag->add(Gtk::Action::create(name, stock_id, label, tooltip));
		else
			ag->add(Gtk::Action::create(name, stock_id, label, tooltip), Gtk::AccelKey(accel));
	}

	void item(	const Glib::ustring &name, 
							const Glib::ustring &label = Glib::ustring(), 
							const Glib::ustring &tooltip = Glib::ustring(), 
							const Glib::ustring &accel = Glib::ustring())
	{
		if(accel.empty())
			ag->add(Gtk::Action::create(name, label, tooltip));
		else
			ag->add(Gtk::Action::create(name, label, tooltip), Gtk::AccelKey(accel));
	}

protected:
	Glib::RefPtr<Gtk::ActionGroup> ag;
};


/*
 *
 */
MenuBar::MenuBar()
:Gtk::VBox(false, 0), m_statusbar(NULL)
{
	m_refUIManager = Gtk::UIManager::create();
}

/*
 *
 */
void MenuBar::connect_proxy(const Glib::RefPtr<Gtk::Action> &action, Gtk::Widget *widget)
{
	if(Gtk::MenuItem *item = dynamic_cast<Gtk::MenuItem*>(widget))
	{
		Glib::ustring tooltip = action->property_tooltip();

		item->signal_select().connect(
				sigc::bind(
					sigc::mem_fun(m_statusbar, &Statusbar::push_text), tooltip));
		item->signal_deselect().connect(
				sigc::mem_fun(m_statusbar, &Statusbar::pop_text));
	}
}

/*
 *
 */
void MenuBar::create(Gtk::Window &window, Statusbar &statusbar)
{
	m_statusbar = &statusbar;

	m_refActionGroup = Gtk::ActionGroup::create("default");

	// menu-file
	{
		ActionGroup ag("file", m_refUIManager);

		ag.item("menu-file", _("_File"));

		ag.item("properties", _("_Properties"),	"");
	}
	
	// menu-edit
	{
		ActionGroup ag("edit", m_refUIManager);

		ag.item("menu-edit", _("_Edit"));
	
		//ag.item("cut", Gtk::Stock::CUT, "", _(""));
		//ag.item("copy", Gtk::Stock::COPY, "", _(""));
		//ag.item("paste", Gtk::Stock::PASTE, "", _(""));
	}

	// timings
	{
			ActionGroup ag("timings", m_refUIManager);

			ag.item("menu-timings", _("_Timings"));
	}

	// menu-tools
	{
		ActionGroup ag("tools", m_refUIManager);

		ag.item("menu-tools", _("T_ools"));
	
		//ag.item("find-and-replace", Gtk::Stock::FIND_AND_REPLACE, _("_Find And Replace"), 
		//		_("Search for and replace text"), "<Control>F");
		ag.item("check-errors", Gtk::Stock::YES, _("_Check Errors"),
				_("Launch the errors checking"));
	}

	// menu-video
	{
		ActionGroup ag("video-player", m_refUIManager);

		ag.item("menu-video", _("_Video"));
	
		ag.item("video-player/open", Gtk::Stock::OPEN, _("_Open Movie"), 
				_("Open the video"), "<Shift><Control>M");
		ag.item("video-player/play", Gtk::Stock::MEDIA_PLAY,
				_("_Play the video"));
		ag.item("video-player/pause", Gtk::Stock::MEDIA_PAUSE, 
				_("_Make a pause"));
		ag.item("video-player/play-pause", Gtk::Stock::MEDIA_PLAY, 
				_("_Play / Pause"), _("Play or make a pause"));

		ag.item("video-player/seek-to-selection", _("_Seek To Selection"),
				_("Seek to the first selected subtitle"));

		ag.item("video-player/play-previous-subtitle", Gtk::Stock::MEDIA_PREVIOUS, _("Play _Previous Subtitle"), 
				_("Play previous subtitle from the first selected subtitle"));
		ag.item("video-player/play-current-subtitle", Gtk::Stock::MEDIA_PLAY, _("Play _Selection"), 
				_("Play the selected subtitle"));
		ag.item("video-player/play-next-subtitle", Gtk::Stock::MEDIA_NEXT, _("Play _Next Subtitle"), 
				_("Play next subtitle from the first selected subtitle"));
	
		ag.item("video-player/play-previous-second", _("Play Previous Second"));
		ag.item("video-player/play-first-second", _("Play First Second"));
		ag.item("video-player/play-last-second", _("Play Last Second"));
		ag.item("video-player/play-next-second", _("Play Next Second"));

		ag.item("video-player/set-subtitle-start", _("Set Subtitle _Start"));
		ag.item("video-player/set-subtitle-end", _("Set Subtitle _End"));

		ag.item("video-player/menu-backwards-jump", Gtk::Stock::MEDIA_REWIND, _("_Backards Jump"));
		ag.item("video-player/very-short-backwards-jump", _("Very Short"));
		ag.item("video-player/short-backwards-jump", _("Short"));
		ag.item("video-player/medium-backwards-jump", _("Medium"));
		ag.item("video-player/long-backwards-jump", _("Long"));

		ag.item("video-player/menu-forward-jump", Gtk::Stock::MEDIA_FORWARD, _("_Forward Jump"));
		ag.item("video-player/very-short-forward-jump", _("Very Short"));
		ag.item("video-player/short-forward-jump", _("Short"));
		ag.item("video-player/medium-forward-jump", _("Medium"));
		ag.item("video-player/long-forward-jump", _("Long"));
	}

	// menu-waveform
	{
		ActionGroup ag("waveform", m_refUIManager);

		ag.item("menu-waveform", _("_Waveform"));
	
		ag.item("waveform/open", Gtk::Stock::OPEN, _("_Open Waveform"), _("Open wavefrom from a file or create from a video"), "<Control><Alt>O");
		ag.item("waveform/save", Gtk::Stock::SAVE, _("_Save Waveform"), _("Save waveform to file"), "<Control><Alt>S");

		ag.item("waveform/zoom-in", Gtk::Stock::ZOOM_IN, _("Zoom _In"));
		ag.item("waveform/zoom-out", Gtk::Stock::ZOOM_OUT, _("Zoom _Out"));
		ag.item("waveform/zoom-selection", Gtk::Stock::ZOOM_FIT, _("Zoom _Selection"));
		ag.item("waveform/zoom-all", Gtk::Stock::ZOOM_100, _("Zoom _All"));

		ag.item("waveform/center-with-selected-subtitle", _("_Center With Selected Subtitle"));

		addToggleAction("waveform/scrolling-with-cursor", _("Scrolling With _Cursor"), "waveform", "scrolling-with-cursor");
		addToggleAction("waveform/scrolling-with-selection", _("Scrolling With _Selection"), "waveform", "scrolling-with-selection");
	}

	// menu-view
	{
		ActionGroup ag("view", m_refUIManager);

		ag.item("menu-view", _("V_iew"));

		ag.item("view/simple", _("_Simple"));
		ag.item("view/advanced", _("_Advanced"));
		ag.item("view/translation", _("_Translation"));
		ag.item("view/timing", _("T_iming"));

		addToggleAction("display-video-player", _("_Video Player"), "interface", "display-video-player");
		addToggleAction("display-waveform", _("_Waveform"), "interface", "display-waveform");

		ag.item("menu-columns", _("Columns"));

		addToggleActionColumn("number", _("Number"));
		addToggleActionColumn("layer", _("Layer"));
		addToggleActionColumn("start", _("Start"));
		addToggleActionColumn("end", _("End"));
		addToggleActionColumn("duration", _("Duration"));
		addToggleActionColumn("style", _("Style"));
		addToggleActionColumn("name", _("Name"));
		addToggleActionColumn("margin-r", _("Margin Left"));
		addToggleActionColumn("margin-l", _("Margin Right"));
		addToggleActionColumn("margin-v", _("Margin Vertical"));
		addToggleActionColumn("effect", _("Effect"));
		addToggleActionColumn("text", _("Text"));
		addToggleActionColumn("translation", _("Translation"));
		addToggleActionColumn("note", _("Note"));
		addToggleActionColumn("cps", _("Characters Per Second"));
	}
	
	// menu-option
	{
		ActionGroup ag("options", m_refUIManager);

		ag.item("menu-options", _("_Options"));
		ag.item("preferences", Gtk::Stock::PREFERENCES, "",
				_("Configure Subtitle Editor"));
		ag.item("configure-keyboard-shortcuts", _("Configure _Keyboard Shortcuts"),
				_("Configure Keyboard Shortcuts"));
	}

	// menu-help
	{
		ActionGroup ag("help", m_refUIManager);

		ag.item("menu-help", _("_Help"));
	}

	show_all();

	// UIManager

	// on connect tous les groups au callback
	std::vector<Glib::RefPtr<Gtk::ActionGroup> > ags = m_refUIManager->get_action_groups();
	for(unsigned int i=0; i < ags.size(); ++i)
	{
		ags[i]->signal_pre_activate().connect(
				sigc::mem_fun(*this, &MenuBar::action_activate));
	}


	m_refUIManager->signal_connect_proxy().connect(
			sigc::mem_fun(*this, &MenuBar::connect_proxy));

	m_refUIManager->insert_action_group(m_refActionGroup);

	window.add_accel_group(m_refUIManager->get_accel_group());

	create_ui_from_file();

	Config::getInstance().signal_changed("subtitle-view").connect(
			sigc::mem_fun(*this, &MenuBar::on_config_subtitle_view_changed));

	ActionSystem::getInstance().signal_sensitive_changed().connect(
			sigc::mem_fun(*this, &MenuBar::set_sensitive));

#warning "FIXME: properties"
	set_sensitive("properties", false);
}

/*
 *
 */
void MenuBar::create_ui_from_file()
{
	m_refUIManager->add_ui_from_file(get_share_dir("menubar.xml"));

	pack_start(*m_refUIManager->get_widget("/menubar"), false, false);

	show_all();
}

/*
 *
 */
MenuBar::~MenuBar()
{
}


/*
 *
 */
void MenuBar::addToggleActionColumn(const Glib::ustring &name, const Glib::ustring &label	)
{
	bool default_value = false;

	Glib::RefPtr<Gtk::Action> action = Gtk::ToggleAction::create(name, label);
	Glib::RefPtr<Gtk::ToggleAction> toggle_action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action);

	if(Config::getInstance().get_value_bool("subtitle-view", Glib::ustring("show-column-") + name, default_value))
		toggle_action->set_active(default_value);

	sigc::slot<void> slot = sigc::bind<Glib::ustring>(
			sigc::mem_fun(*this, &MenuBar::on_setup_view_toggled), name);

	m_connections[name] = action->signal_activate().connect(slot);

	m_refActionGroup->add(action);
}

/*
 *
 */
void MenuBar::addToggleAction(const Glib::ustring &name, const Glib::ustring &label, const Glib::ustring &group, const Glib::ustring &key )
{
	Glib::RefPtr<Gtk::Action> action = Gtk::ToggleAction::create(name, label);
	Glib::RefPtr<Gtk::ToggleAction> toggle_action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action);

	bool value = false;
	if(Config::getInstance().get_value_bool(group, key, value))
		toggle_action->set_active(value);

	/*m_connections[name] = */action->signal_activate().connect(
				sigc::bind<Glib::ustring>(sigc::mem_fun(this, &MenuBar::execute), name));

	m_refActionGroup->add(action);

}

/*
 *
 */
void MenuBar::action_activate(const Glib::RefPtr<Gtk::Action> action)
{
	execute(action->get_name());
}

/*
 *
 */
void MenuBar::execute(const Glib::ustring &name)
{
	//std::cout << name << std::endl;

	ActionSystem::getInstance().execute(name);

	if(name == "preferences")
		on_preferences();
	else if(name == "configure-keyboard-shortcuts")
		dialog_configure_keyboard_shortcuts();
	else if(name == "display-video-player")
	{
		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(name));

		bool value = action->get_active();
		Config::getInstance().set_value_bool("interface", "display-video-player", value);
	}
	else if(name == "display-waveform")
	{
		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(name));

		bool value = action->get_active();
		
		Config::getInstance().set_value_bool("interface", "display-waveform", value);
	}
	else if(name == "waveform/scrolling-with-cursor")
	{
		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(name));

		bool value = action->get_active();

		Config::getInstance().set_value_bool("waveform", "scrolling-with-cursor", value);
	}
	else if(name == "waveform/scrolling-with-selection")
	{
		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(name));

		bool value = action->get_active();

		Config::getInstance().set_value_bool("waveform", "scrolling-with-selection", value);
	}
}


// FILE

/*
 *
 */
void MenuBar::on_setup_view_toggled(const Glib::ustring &column)
{
	Glib::RefPtr<Gtk::ToggleAction> action = 
		Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(column));

	if(action)
	{
		sigc::connection connection = m_connections[column];

		bool state = action->get_active();
	
		connection.block();
	
		Config::getInstance().set_value_bool("subtitle-view", Glib::ustring("show-column-")+column, state);
		
		connection.unblock();
	}
}

/*
 *
 */
void MenuBar::set_toggle_column(const Glib::ustring &column, bool state)
{
	try
	{
		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(column));

		if(action)
		{
			Config::getInstance().set_value_bool("subtitle-view", Glib::ustring("show-column-")+column, state);
		}
	}
	catch(Glib::Error &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}

/*
 *
 */
void MenuBar::on_preferences()
{
	PreferencesUI*	dialog = utility::get_widget_derived<PreferencesUI>("dialog-preferences.glade", "dialog-preferences");

	dialog->run();

	delete dialog;
}

/*
 *
 */
void MenuBar::on_config_subtitle_view_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	//std::cout << build_message("%s %s %s %s", __FILE__, __FUNCTION__, key.c_str(), value.c_str()) << std::endl;
	
	if(key.find("show-column-") != Glib::ustring::npos)
	{
		Glib::ustring column = key;
		column.erase(0, Glib::ustring("show-column-").size());

		Glib::RefPtr<Gtk::ToggleAction> action = 
			Glib::RefPtr<Gtk::ToggleAction>::cast_static(m_refActionGroup->get_action(column));

		bool state;
		if(from_string(value, state))
		{
			m_connections[column].block();
			action->set_active(state);
			m_connections[column].unblock();
		}
	}
}

/*
 *
 */
void MenuBar::set_sensitive(const Glib::ustring &name, bool state)
{
	std::vector<Glib::RefPtr<Gtk::ActionGroup> > ags = m_refUIManager->get_action_groups();
	for(unsigned int i=0; i < ags.size(); ++i)
	{
		Glib::RefPtr<Gtk::Action> action = ags[i]->get_action(name);
		if(action)
		{
			action->set_sensitive(state);
			return;
		}
	}
}




#include <gtkmm.h>
#include <libglademm/xml.h>
#include <gtk/gtk.h>

#include <libxml++/libxml++.h>



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
		find_and_replace(label, "_", "");
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
void MenuBar::dialog_configure_keyboard_shortcuts()
{
	DialogConfigureKeyboardShortcuts *dialog = 
		utility::get_widget_derived<DialogConfigureKeyboardShortcuts>(
				"dialog-configure-keyboard-shortcuts.glade", 
				"dialog-configure-keyboard-shortcuts");

	dialog->execute(m_refUIManager);

	delete dialog;
}
