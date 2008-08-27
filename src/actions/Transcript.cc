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

#include "formats/SubtitleText.h"
#include "SubtitleSystem.h"
#include "gui/DialogFileChooser.h"

/*
 *
 */
class TranscriptPlugin : public Plugin
{
public:

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("TranscriptPlugin");

		action_group->add(
				Gtk::Action::create("import-transcript", _("_Import Transcript"), _("Create a new document with any text file")),
					sigc::mem_fun(*this, &TranscriptPlugin::on_import_transcript));

		action_group->add(
				Gtk::Action::create("export-transcript", _("_Export Transcript"), _("Export just a text in a file")),
					sigc::mem_fun(*this, &TranscriptPlugin::on_export_transcript));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		//ui->add_ui(ui_id, "/menubar/menu-edit/extend-XX", "open-transcript", "open-transcript");
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

		action_group->get_action("export-transcript")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_import_transcript()
	{
		se_debug(SE_DEBUG_PLUGINS);

		DialogImportText::auto_ptr ui = DialogImportText::create();

		if(ui->run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring filename = ui->get_filename();
			Glib::ustring charset = ui->get_encoding();

			try
			{
				Document *doc = new Document();
				doc->setCharset(charset);

				SubtitleText reader(doc);
				if(reader.open(filename))
				{
					Glib::ustring untitled = DocumentSystem::getInstance().create_untitled_name();
					
					doc->setName(untitled);

					DocumentSystem::getInstance().append(doc);
				}
				else
				{
					delete doc;
				}
			}
			catch(...)
			{
				//std::cerr << ex.what() << std::endl;
			}
		}
	}

	/*
	 *
	 */
	void on_export_transcript()
	{
	 se_debug(SE_DEBUG_PLUGINS);

		DialogExportText::auto_ptr ui = DialogExportText::create();

		if(ui->run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring filename = ui->get_filename();
			Glib::ustring charset = ui->get_encoding();
			Glib::ustring newline = ui->get_newline();

			try
			{
				Document *doc = get_current_document();

				if(doc != NULL)
				{
					Document copy(*doc);
					copy.setCharset(charset);
					copy.setNewLine(newline);
					copy.setFilename(filename);

					SubtitleText w(&copy);
					if(w.save(filename))
					{
					}
					else
						;//error
				}
			}
			catch(...)
			{
			}
		}
	}

	
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_PLUGIN(TranscriptPlugin)
