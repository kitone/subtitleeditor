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

#include <extension/Action.h>
#include <utility.h>
#include <gui/DialogFileChooser.h>
#include <vector>

/*
 *
 */
class DialogAskToSaveOnExit : public Gtk::MessageDialog
{
public:
	DialogAskToSaveOnExit()
	:Gtk::MessageDialog("", false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE)
	{
		utility::set_transient_parent(*this);
		
		add_button(_("Close _without Saving"), Gtk::RESPONSE_NO);
		add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_YES);
	}

	int run(Document *doc)
	{
		Glib::ustring document_name = doc->getName();
		Glib::ustring primary_text = build_message(_("Save the changes to document \"%s\" before closing?"), document_name.c_str());
		Glib::ustring secondary_text = _("If you don't save, the last changes will be permanently lost.");

		set_message(primary_text);
		set_secondary_text(secondary_text);

		return Gtk::Dialog::run();
	}
};

/*
 *
 */
class DocumentManagementPlugin : public Action
{
public:

	DocumentManagementPlugin()
	{
		activate();
		update_ui();
	}

	~DocumentManagementPlugin()
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
		action_group = Gtk::ActionGroup::create("DocumentManagementPlugin");

		// new document
		action_group->add(
				Gtk::Action::create("new-document", Gtk::Stock::NEW, "", _("Create a new document")),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_new));

		// open & save document
		action_group->add(
				Gtk::Action::create("open-document", Gtk::Stock::OPEN, "", _("Open a file")), 
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_open));

		action_group->add(
				Gtk::Action::create("save-document", Gtk::Stock::SAVE, "", _("Save the current file")), Gtk::AccelKey("<Control>S"),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_save));

		action_group->add(
				Gtk::Action::create("save-as-document", Gtk::Stock::SAVE_AS, "", _("Save the current file with a different name")), Gtk::AccelKey("<Shift><Control>S"),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_save_as));

		action_group->add(
				Gtk::Action::create("save-all-documents", Gtk::Stock::SAVE_AS, _("Save _All"), _("Save all open files")),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_save_all_documents));

		// open & save translation
		action_group->add(
				Gtk::Action::create("open-translation", Gtk::Stock::OPEN, _("Open _Translation"), _("Open translation from file")), Gtk::AccelKey("<Control>T"),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_open_translation));

		action_group->add(
				Gtk::Action::create("save-translation", Gtk::Stock::SAVE, _("Save Trans_lation"), _("Save translation to file")), Gtk::AccelKey("<Shift><Control>T"),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_save_translation));

		// recent files
		Glib::RefPtr<Gtk::RecentAction> recentAction = Gtk::RecentAction::create("menu-recent-open-document", _("Open _Recent"));

		Gtk::RecentFilter filter;
		filter.set_name("subtitleeditor");
		filter.add_group("subtitleeditor");
		recentAction->set_filter(filter);

		recentAction->set_show_icons(false);
		recentAction->set_show_numbers(true);
		recentAction->set_show_tips(true);
		//recentAction->set_show_not_found(false);
		recentAction->set_sort_type(Gtk::RECENT_SORT_MRU);

		recentAction->signal_item_activated().connect(
				sigc::mem_fun(*this, &DocumentManagementPlugin::on_recent_item_activated));
		action_group->add(recentAction);
		
		// close
		action_group->add(
				Gtk::Action::create("close-document", Gtk::Stock::CLOSE, "", _("Close the current file")),
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_close));

		// quit the program
		action_group->add(
				Gtk::Action::create("exit", Gtk::Stock::QUIT, _("E_xit"), _("Quit the program")), 
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_exit));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);


		DocumentSystem::getInstance().signal_document_create().connect(
				sigc::mem_fun(*this, &DocumentManagementPlugin::on_document_create));

		Gtk::Window *window = dynamic_cast<Gtk::Window*>(get_subtitleeditor_window());
		
		g_return_if_fail(window);
		{
			window->signal_delete_event().connect(
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_subtitleeditor_window_delete_event));
		}

		// config connection
		m_config_interface_connection = get_config().signal_changed("interface").connect(
			sigc::mem_fun(*this, &DocumentManagementPlugin::on_config_interface_changed));

		init_autosave();

		ui_id = ui->new_merge_id();

		#define ADD_UI(name)	ui->add_ui(ui_id, "/menubar/menu-file/"name, name, name);
		
		ADD_UI("new-document");
		ADD_UI("open-document");
		ADD_UI("open-translation");
		ADD_UI("menu-recent-open-document");
		ADD_UI("save-document");
		ADD_UI("save-as-document");
		ADD_UI("save-all-documents");
		ADD_UI("save-translation");
		ADD_UI("close-document");
		ADD_UI("exit");
		
		#undef ADD_UI
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

		m_config_interface_connection.disconnect();
		m_autosave_timeout.disconnect();
	}

	/*
	 *
	 */
	void update_ui()
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool visible = (get_current_document() != NULL);

		action_group->get_action("open-translation")->set_sensitive(visible);
		action_group->get_action("save-document")->set_sensitive(visible);
		action_group->get_action("save-as-document")->set_sensitive(visible);
		action_group->get_action("save-all-documents")->set_sensitive(visible);
		action_group->get_action("save-translation")->set_sensitive(visible);
		action_group->get_action("close-document")->set_sensitive(visible);
	}

protected:

	/*
	 *	create a new document with unique name
	 */
	void on_new()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = new Document();

		g_return_if_fail(doc);

		doc->setFilename(DocumentSystem::getInstance().create_untitled_name());

		DocumentSystem::getInstance().append(doc);
	}

	/*
	 *	launch a filechooser dialog
	 *	and open a document
	 */
	void on_open()
	{
		se_debug(SE_DEBUG_PLUGINS);

		DialogOpenDocument::auto_ptr dialog = DialogOpenDocument::create();

		dialog->show();
		
		if(dialog->run() != Gtk::RESPONSE_OK)
			return;

		dialog->hide();

		Glib::ustring charset = dialog->get_encoding();

		std::list<Glib::ustring> uris = dialog->get_uris();

		for(std::list<Glib::ustring>::const_iterator it=uris.begin();
				it != uris.end(); ++it)
		{
			open_document(*it, charset);
		}

		Glib::ustring video_uri = dialog->get_video_uri();
		if(video_uri.empty() == false)
		{
			// TODO
			// check and ask if already exist ?
			SubtitleEditorWindow::get_instance()->get_player()->open(video_uri);
		}
	}

	/*
	 *
	 */
	bool open_document(const Glib::ustring &uri, const Glib::ustring &charset)
	{
		se_debug_message(SE_DEBUG_PLUGINS, "uri=%s charset=%s", uri.c_str(), charset.c_str());

		Glib::ustring filename = Glib::filename_from_uri(uri);

		// check if is not already open
		Document* already = DocumentSystem::getInstance().getDocument(filename);

		if(already)
		{
			//DocumentSystem::getInstance().setCurrentDocument(already);
			already->flash_message(_("I am already open"));
		}
		else
		{
			Document *doc = Document::create_from_file(uri, charset);
			if(doc)
			{
				DocumentSystem::getInstance().append(doc);
				return true;
			}
		}

		return false;
	}

	/*
	 *	Save a document. If file doesn't exist use save_as
	 */
	bool save_document(Document *doc)
	{
		se_debug(SE_DEBUG_PLUGINS);

		g_return_val_if_fail(doc, false);

		if(Glib::file_test(doc->getFilename(), Glib::FILE_TEST_EXISTS))
		{
			Glib::ustring filename = doc->getFilename();
			Glib::ustring format = doc->getFormat();
			Glib::ustring charset = doc->getCharset();
			Glib::ustring newline = doc->getNewLine();

			if(doc->save(filename))
			{
				// "Saving file FILENAME (FORMAT, CHARSET, NEWLINE)."
				doc->flash_message(_("Saving file %s (%s, %s, %s)."), 
							filename.c_str(), format.c_str(), charset.c_str(), newline.c_str());

				return true;
			}
			else
			{
				// "The file FILENAME (FORMAT, CHARSET, NEWLINE) has not been saved."
				doc->message(_("The file %s (%s, %s, %s) has not been saved."), 
							filename.c_str(), format.c_str(), charset.c_str(), newline.c_str());
			}
		}
		else
			return save_as_document(doc);

		return false;
	}

	/*
	 *
	 */
	bool save_as_document(Document *doc)
	{
		se_debug(SE_DEBUG_PLUGINS);

		g_return_val_if_fail(doc, false);

		DialogSaveDocument::auto_ptr dialog = DialogSaveDocument::create();

		if(Glib::file_test(doc->getFilename(), Glib::FILE_TEST_EXISTS))
			dialog->set_filename(doc->getFilename());
		else
			dialog->set_current_name(doc->getName());

		dialog->set_format(doc->getFormat());
		dialog->set_encoding(doc->getCharset());
		dialog->set_newline(doc->getNewLine());
		dialog->set_do_overwrite_confirmation(true);
		dialog->show();

		int response = dialog->run();
		
		dialog->hide();

		if(response == Gtk::RESPONSE_OK)
		{
			Glib::ustring filename = dialog->get_filename();
			Glib::ustring format = dialog->get_format();
			Glib::ustring encoding = dialog->get_encoding();
			Glib::ustring newline = dialog->get_newline();


			doc->setFormat(format);
			doc->setCharset(encoding);
			doc->setNewLine(newline);

			if(doc->save(filename))
			{
				doc->flash_message(_("Saving file %s (%s, %s, %s)."), 
						filename.c_str(), format.c_str(), encoding.c_str(), newline.c_str());
				// update in recent manager
				add_document_in_recent_manager(doc);

				return true;
			}
			else
				doc->message(_("The file %s (%s, %s, %s) has not been saved."), 
						filename.c_str(), format.c_str(), encoding.c_str(), newline.c_str());
		}

		return false;
	}

	/*
	 *	Save a document. If file doesn't exist use on_save_as
	 */
	void on_save()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		save_document(doc);
	}

	/*
	 *	Save document with new name
	 */
	void on_save_as()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		g_return_if_fail(doc);

		save_as_document(doc);
	}

	/*
	 * Save all open files
	 */
	void on_save_all_documents()
	{
		se_debug(SE_DEBUG_PLUGINS);

		DocumentList list = get_subtitleeditor_window()->get_documents();
		
		for(DocumentList::const_iterator it = list.begin(); it != list.end(); ++it)
		{
			save_document(*it);
		}
	}
	
	/*
	 *	Open translation from file.
	 *	Create a new document with a translation
	 *	and move the text of this new document (trans) to the current document
	 *	at the column "translation". After that delete the new document (trans)
	 */
	void on_open_translation()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *current = get_current_document();

		g_return_if_fail(current);

		DialogOpenDocument::auto_ptr ui = DialogOpenDocument::create();

		ui->show_video(false);
		ui->set_select_multiple(false);
		ui->show();
		
		if(ui->run() == Gtk::RESPONSE_OK)
		{
			ui->hide();
	
			Glib::ustring encoding = ui->get_encoding();
			Glib::ustring uri = ui->get_uri();

			try
			{
				Document *doc = Document::create_from_file(uri, encoding);
		
				if(doc)
				{
					current->start_command(_("Open translation"));

					Subtitle s1 = current->subtitles().get_first();
					Subtitle s2 = doc->subtitles().get_first();

					while(s1 && s2)
					{
						s1.set_translation(s2.get_text());

						++s1;
						++s2;
					}

					// create new subtitle
					if(s2)
					{
						int size = doc->subtitles().size() - current->subtitles().size();

						while(s2)
						{
							s1 = current->subtitles().append();

							s1.set_translation(s2.get_text());
							s1.set_start_and_end(s2.get_start(), s2.get_end());
							++s2;
						}

						current->flash_message(ngettext(
								"1 subtitle was added with the translation",
								"%d subtitles were added with the translation",
								size), size);
					}

					current->finish_command();

					delete doc;
				}
			}
			catch(...)
			{
			}
		}
		
		ui->hide();
	}

	/*
	 *	Save the current translation in a new document
	 */
	void on_save_translation()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *current = get_current_document();

		g_return_if_fail(current);

		DialogSaveDocument::auto_ptr dialog = DialogSaveDocument::create();

		if(dialog.get() == NULL)
			dialog = DialogSaveDocument::create();

		dialog->show();
		if(dialog->run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring filename = dialog->get_filename();
			Glib::ustring format = dialog->get_format();
			Glib::ustring encoding = dialog->get_encoding();
			Glib::ustring newline = dialog->get_newline();

			try
			{
				Document doc_translation(*current);

				doc_translation.setFilename(filename);
				doc_translation.setFormat(format);
				doc_translation.setCharset(encoding);
				doc_translation.setNewLine(newline);

				// apply translation
				Subtitle sub = doc_translation.subtitles().get_first();
				while(sub)
				{
					sub.set_text(sub.get_translation());

					++sub;
				}
				//
				if(doc_translation.save(filename))
					current->flash_message(_("Saving translation file %s (%s, %s, %s)."), 
							filename.c_str(), format.c_str(), encoding.c_str(), newline.c_str());
				else
					current->message(_("The translation file %s (%s, %s, %s) has not been saved."), 
							filename.c_str(), format.c_str(), encoding.c_str(), newline.c_str()); 
			}
			catch(...)
			{
			}
		}

		dialog->hide();
	}

	/*
	 *	
	 */
	bool on_subtitleeditor_window_delete_event(GdkEventAny *ev)
	{
		bool state = false;

		while(get_current_document() != NULL)
		{
			state = close_current_document();

			if(state == false)
				return true;	// true to stop the closing
		}

		return false;
	}

	/*
	 *
	 */
	bool close_current_document()
	{
		Document *doc = get_current_document();
		
		g_return_val_if_fail(doc, false);

		if(get_config().get_value_bool("interface", "ask-to-save-on-exit") == false)
		{
			DocumentSystem::getInstance().remove(doc);
		}
		else if(!doc->get_document_changed())
		{
			DocumentSystem::getInstance().remove(doc);
		}
		else
		{
			DialogAskToSaveOnExit dialog;
		
			int response = dialog.run(doc);

			if(response == Gtk::RESPONSE_YES)
			{
				on_save();

				//if(doc->get_document_changed() == false)
					DocumentSystem::getInstance().remove(doc);
			}
			else if(response == Gtk::RESPONSE_NO)
			{
				DocumentSystem::getInstance().remove(doc);
			}
			else if(response == Gtk::RESPONSE_CANCEL)
			{
				// nothing
				return false;
			}
		}

		return true;
	}

	/*
	 *	remove the current document
	 */
	void on_close()
	{
		se_debug(SE_DEBUG_PLUGINS);

		close_current_document();
	}

	/*
	 *	quit the program
	 *	close all document with support to ask to save if is enable
	 */
	void on_exit()
	{
		while(get_current_document() != NULL)
		{
			if(!close_current_document())
				return;
		}

		Gtk::Main::quit();
	}


	/*
	 *	a new document has been create, update the recent manager 
	 */
	void on_document_create(Document *doc)
	{
		se_debug(SE_DEBUG_PLUGINS);

		add_document_in_recent_manager(doc);
	}

	/*
	 *
	 */
	void add_document_in_recent_manager(Document *doc)
	{
		se_debug(SE_DEBUG_PLUGINS);

		if(doc)
		{
			Glib::ustring filename = doc->getFilename();

			if(!Glib::file_test(filename, Glib::FILE_TEST_EXISTS))
				return;

			Glib::ustring uri = Glib::filename_to_uri(filename);

			se_debug_message(SE_DEBUG_PLUGINS, "uri=%s", uri.c_str());

			Gtk::RecentManager::Data data;
			//data.mime_type = "subtitle/";
			data.app_name = Glib::get_application_name();
			data.app_exec = Glib::get_prgname();
			data.groups.push_back("subtitleeditor");
			data.is_private = false;

			Gtk::RecentManager::get_default()->add_item(uri, data);
		}
	}

	/*
	 *	open a recent document
	 */
	void on_recent_item_activated()
	{
		Glib::RefPtr<Gtk::Action> action = action_group->get_action("menu-recent-open-document");

		Glib::RefPtr<Gtk::RecentAction> recentAction = Glib::RefPtr<Gtk::RecentAction>::cast_static(action);

		Glib::RefPtr<Gtk::RecentInfo> cur = recentAction->get_current_item();
		
		if(cur)
		{
			se_debug_message(SE_DEBUG_PLUGINS, "uri=%s", cur->get_uri().c_str());

			open_document(cur->get_uri(), "");
		}
	}

	/*
	 * Only for "used-autosave" and "autosave-minutes".
	 */
	void on_config_interface_changed(const Glib::ustring &key, const Glib::ustring &value)
	{
		if(key == "used-autosave" || key == "autosave-minutes")
			init_autosave();
	}

	/*
	 *
	 */
	void init_autosave()
	{
		se_debug(SE_DEBUG_PLUGINS);

		m_autosave_timeout.disconnect();

		bool used_autosave = Config::getInstance().get_value_bool("interface", "used-autosave");
		if(used_autosave)
		{
			int autosave_minutes = Config::getInstance().get_value_int("interface", "autosave-minutes");

			long mseconds = SubtitleTime(0, autosave_minutes, 0, 0).totalmsecs;

			m_autosave_timeout = Glib::signal_timeout().connect(
					sigc::mem_fun(*this, &DocumentManagementPlugin::on_autosave_files), mseconds);

			se_debug_message(SE_DEBUG_PLUGINS, "save files every %d minutes", autosave_minutes);
		}
	}

	/*
	 * Save files every "auto-save-minutes" value.
	 */
	bool on_autosave_files()
	{
		se_debug(SE_DEBUG_PLUGINS);

		on_save_all_documents();
		
		return true;
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	sigc::connection m_config_interface_connection;
	sigc::connection m_autosave_timeout;
};

REGISTER_EXTENSION(DocumentManagementPlugin)
