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

#include <extension/action.h>
#include <utility.h>

class AboutPlugin : public Action
{
public:

	AboutPlugin()
	{
		activate();
		//update_ui();
	}

	~AboutPlugin()
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
		action_group = Gtk::ActionGroup::create("AboutPlugin");

		action_group->add(
				Gtk::Action::create("about", Gtk::Stock::ABOUT),
					sigc::mem_fun(*this, &AboutPlugin::on_about));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-help/about", "about", "about");
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
	void on_about()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Gtk::AboutDialog dialog;
		utility::set_transient_parent(dialog);

		// name
		dialog.set_name("Subtitle Editor");
		// version
		dialog.set_version(VERSION);
		// copyright
		dialog.set_copyright("kitone (IDJAAD djamel)");
		// comments
		dialog.set_comments(_("a tool for subtitles edition"));
		// logo
		dialog.set_logo_icon_name("subtitleeditor");
		// authors
		std::list<Glib::ustring>	authors;
		authors.push_back("kitone (IDJAAD djamel)");
		
		dialog.set_authors(authors);

		// translator-credits
		dialog.set_translator_credits(_("translator-credits"));

		// website
		dialog.set_website("http://home.gna.org/subtitleeditor/");

		// license
		Glib::ustring license=
			"This program is free software; you can redistribute it and/or modify  \n"
			"it under the terms of the GNU General Public License as published by  \n"
			"the Free Software Foundation; either version 3 of the License, or	\n"
			"(at your option) any later version.	\n\n"
			"This program is distributed in the hope that it will be useful,	\n"
			"but WITHOUT ANY WARRANTY; without even the implied warranty of  \n"
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	\n"
			"GNU General Public License for more details.  \n\n"
			"You should have received a copy of the GNU General Public License	\n"
			"along with this program; if not, write to the Free Software	\n"
			"Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111 USA	\n\n"
			"See gpl.txt for more information regarding the GNU General Public License. \n";
		dialog.set_license(license);

		dialog.run();
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(AboutPlugin)
