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

#include <memory>
#include <extension/action.h>
#include <i18n.h>
#include <debug.h>
#include <gtkmm_utility.h>
#include "interfacepage.h"
#include "documentpage.h"
#include "videoplayerpage.h"
#include "timingpage.h"
#include "waveformpage.h"
#include "extensionpage.h"

/*
 *
 */
class DialogPreferences : public Gtk::Dialog
{
public:

	/*
	 *
	 */
	DialogPreferences(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	:Gtk::Dialog(cobject)
	{
		utility::set_transient_parent(*this);

		InterfacePage *interface = NULL;
		DocumentPage *document = NULL;
		WaveformPage *waveform = NULL;
		VideoPlayerPage *videoplayer = NULL;
		TimingPage *timing = NULL;
		ExtensionPage* extension = NULL;

		xml->get_widget_derived("vbox-interface", interface);
		xml->get_widget_derived("vbox-document", document);
		xml->get_widget_derived("vbox-waveform", waveform);
		xml->get_widget_derived("vbox-video-player", videoplayer);
		xml->get_widget_derived("vbox-timing", timing);
		xml->get_widget_derived("vbox-extension", extension);
	}

	/*
	 *
	 */
	static void create()
	{
		std::auto_ptr<DialogPreferences> dialog(
				gtkmm_utility::get_widget_derived<DialogPreferences>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_GLADE, SE_PLUGIN_PATH_DEV),
						"dialog-preferences.glade", 
						"dialog-preferences"));

		dialog->run();
	}
};

/*
 * Error Checking Plugin
 */
class PreferencesPlugin : public Action
{
public:

	PreferencesPlugin()
	{
		activate();
		update_ui();
	}

	~PreferencesPlugin()
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
		action_group = Gtk::ActionGroup::create("PreferencesPlugin");

		action_group->add(
				Gtk::Action::create("preferences", Gtk::Stock::PREFERENCES, "", _("Configure Subtitle Editor")),
					sigc::mem_fun(*this, &PreferencesPlugin::on_preferences));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-options/preferences", "preferences", "preferences");
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
	void on_preferences()
	{
		DialogPreferences::create();
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(PreferencesPlugin)
