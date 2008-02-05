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
 
#include <gtkmm.h>
#include "Document.h"
#include "Plugin.h"
#include "utility.h"
#include "gui/DialogFileChooser.h"

/*
 *
 */
class JoinDocumentPlugin : public Plugin
{
public:

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("JoinDocumentPlugin");

		action_group->add(
				Gtk::Action::create("join-document", Gtk::Stock::CONNECT, _("_Join Document"), _("Add subtitles from file")), 
					sigc::mem_fun(*this, &JoinDocumentPlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		//ui->add_ui(ui_id, "/menubar/menu-tools/extend-8", "join-document", "join-document");
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
	 *
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("join-document")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_execute()
	{
		se_debug(SE_DEBUG_PLUGINS);

		execute();
	}

	/*
	 *
	 */
	bool execute()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_val_if_fail(doc, false);

		DialogOpenDocument ui;
		ui.set_select_multiple(false);
	
		if(ui.run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring ofile = doc->getFilename();
			Glib::ustring oformat = doc->getFormat();
			Glib::ustring ocharset = doc->getCharset();

			Glib::ustring filename = ui.get_filename();
			Glib::ustring format = ui.getFormat();
			Glib::ustring encoding = ui.getEncoding();

			unsigned int subtitle_size = doc->subtitles().size();

			doc->start_command(_("Join document"));

			if(!doc->open(filename));
			{
				doc->finish_command();
				return false;
			}

			doc->setFilename(ofile);
			doc->setFormat(oformat);
			doc->setCharset(ocharset);
			doc->finish_command();

			unsigned int subtitles_added = doc->subtitles().size() - subtitle_size;

			doc->flash_message(_("%d subtitles has been added at this document."), subtitles_added);
		}

		return true;
	}
	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_PLUGIN(JoinDocumentPlugin)
