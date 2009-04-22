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
#include <glib/gregex.h>
#include <gtkmm_utility.h>
#include <widget_config_utility.h>
#include <memory>

/*
 * FIXME: Remove Me
 * Waiting the Glib::MatchInfo API in glibmm.
 */
bool regex_exec(const Glib::ustring &pattern, const Glib::ustring &string, bool caseless, Glib::ustring::size_type &start, Glib::ustring::size_type &len)
{
	bool found = false;
	GRegex *regex = NULL;
	GMatchInfo *match_info = NULL;
	GError *error = NULL;

	int compile_flags = (GRegexMatchFlags)0;
	
	if(caseless)
		compile_flags |= G_REGEX_CASELESS;

	regex = g_regex_new(pattern.c_str(), (GRegexCompileFlags)compile_flags, (GRegexMatchFlags)0, &error);
	if(error != NULL)
	{
		std::cerr << "regex_exec error: " << error->message << std::endl;
		g_error_free(error);
		return false;
	}

	if(g_regex_match(regex, string.c_str(), (GRegexMatchFlags)0, &match_info))
	{
		//while(g_match_info_matches(match_info))
		if(g_match_info_matches(match_info))
		{
			int start_pos, end_pos;
			// check the return
			if(g_match_info_fetch_pos(
							match_info, 
							0, //match_num 0 is full text of the match
							&start_pos,
							&end_pos))
			{
				start = start_pos;
				len = end_pos - start_pos;
				found = true;
			}
		}
	}

	g_match_info_free(match_info);
	g_regex_unref(regex);

	return found;
}


class SearchResult
{
public:
	SearchResult()
	{
		found = false;
		start = len = Glib::ustring::npos;
	}

	bool found;
	Glib::ustring::size_type start;
	Glib::ustring::size_type len;
};


	enum
	{
		USE_REGEX			= 1 << 1,
		IGNORE_CASE		= 1 << 2
	};

	/*
	 *
	 */
	bool find(const Glib::ustring &pattern, int flag, const Glib::ustring &text, SearchResult &info)
	{
		se_debug_message(SE_DEBUG_SEARCH, "pattern=<%s> text=<%s>", pattern.c_str(), text.c_str());

		info.start = info.len = Glib::ustring::npos;
		info.found = false;

		if(pattern.empty())
			return false;

		if(flag & USE_REGEX)
		{
			se_debug_message(SE_DEBUG_SEARCH, "Used regular expression");

			info.found = regex_exec(pattern, text, (flag & IGNORE_CASE), info.start, info.len);

			return info.found;
				
		}
		else
		{
			if(flag & IGNORE_CASE)
			{
				Glib::ustring pattern_lc = pattern.lowercase();
				Glib::ustring text_lc = text.lowercase();

				Glib::ustring::size_type res = text_lc.find(pattern_lc);
				if(res != Glib::ustring::npos)
				{
					info.found = true;
					info.start = res;
					info.len = pattern.size();

					return true;
				}
			}
			else // without ignore case
			{
				Glib::ustring::size_type res = text.find(pattern);
				if(res != Glib::ustring::npos)
				{
					info.found = true;
					info.start = res;
					info.len = pattern.size();

					return true;
				}
			}
		}
		
		return false;
	}


/*
 *	Dialog Find And Replace
 */
class DialogFindAndReplace : public Gtk::Dialog
{
public:
	// like to glade file
	enum RESPONSE
	{
		FIND = 1,
		REPLACE = 2,
		REPLACE_ALL = 3
	};

	/*
	 *	constructor
	 */
	DialogFindAndReplace(BaseObjectType *cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Dialog(cobject), m_document(NULL)
	{
		utility::set_transient_parent(*this);
		
		refGlade->get_widget("textview", m_textview);
	
		refGlade->get_widget("entry-pattern", m_entryPattern);
		refGlade->get_widget("entry-replace-with", m_entryReplaceWith);
		refGlade->get_widget("check-ignore-case", m_checkIgnoreCase);
		refGlade->get_widget("check-used-regular-expression", m_checkUsedRegularExpression);
		refGlade->get_widget("button-replace", m_buttonReplace);
		refGlade->get_widget("button-replace-all", m_buttonReplaceAll);

		widget_config::read_config_and_connect(m_entryPattern, "dialog-find-and-replace", "find");
		widget_config::read_config_and_connect(m_entryReplaceWith, "dialog-find-and-replace", "replace-with");
		widget_config::read_config_and_connect(m_checkIgnoreCase, "dialog-find-and-replace", "ignore-case");
		widget_config::read_config_and_connect(m_checkUsedRegularExpression, "dialog-find-and-replace", "used-regular-expression");

		m_entryPattern->grab_focus();
		m_entryPattern->signal_activate().connect(
				sigc::bind(sigc::mem_fun(*this, &DialogFindAndReplace::on_response), FIND));
		
		set_default_response(Gtk::RESPONSE_CLOSE);


		Glib::RefPtr<Gtk::TextTag> found = m_textview->get_buffer()->create_tag("found");
		found->property_weight() = Pango::WEIGHT_BOLD;
		found->property_foreground() = "blue";

		hide();
}

	/*
	 *
	 */
	bool find_in_subtitle(Subtitle &sub, SearchResult &info)
	{
		se_debug(SE_DEBUG_SEARCH);

		if(!sub)
			return false;

		Glib::ustring pattern = m_entryPattern->get_text();
		Glib::ustring text = sub.get_text();

		Glib::ustring::size_type beginning = 0;

		if(info.start != Glib::ustring::npos && info.len != Glib::ustring::npos)
			beginning = info.start + info.len;

		text = text.substr(beginning, text.size());
		
		SearchResult old_info = info;
		
		int flag = 0;
		
		if(m_checkIgnoreCase->get_active())
			flag = IGNORE_CASE;
		if(m_checkUsedRegularExpression->get_active())
			flag = USE_REGEX;

		if(find(pattern, flag, text, info))
		{
			info.start += beginning;
			return true;
		}
		
		return false;
	}

	/*
	 *
	 */
	bool find_forwards(Subtitle &sub, SearchResult &info)
	{
		se_debug(SE_DEBUG_SEARCH);

		if(!sub)
			return false;

		// search again in the subtitle
		if(find_in_subtitle(sub, info))
		{
			return true;
		}
		// next subtitle
		++sub;
		
		if(!sub)
			return false;
		// init
		info = SearchResult();

		return find_forwards(sub, info);
	}

	/*
	 *
	 */
	bool replace(Subtitle &sub, SearchResult &info)
	{
		se_debug(SE_DEBUG_SEARCH);

		if(!sub)
			return false;

		if(info.start == 0 && info.len == 0)
			return false;

		Glib::ustring text = sub.get_text();

		if(text.empty())
			return false;

		Glib::ustring replace_with = m_entryReplaceWith->get_text();
		
		text.replace(info.start, info.len, replace_with);

		// update lenght of info
		info.len = replace_with.size();

		m_document->start_command(_("Replace text"));
		sub.set_text(text);
		m_document->subtitles().select(sub);	// for undo/redo
		m_document->finish_command();
		return true;
	}

	/*
	 *
	 */
	bool replace_all()
	{
		m_subtitle = m_document->subtitles().get_first();

		while(m_subtitle)
		{
			while(find_forwards(m_subtitle, m_info))
			{
				replace(m_subtitle, m_info);
			}
		}

		return true;
	}

	/*
	 *
	 */
	void execute(Document *doc)
	{
		m_document = doc;

		Subtitles subtitles = doc->subtitles();

		if(subtitles.size() == 0)
		{
			doc->flash_message(_("The document is empty"));
			return;
		}

		m_subtitle = subtitles.get_first_selected();

		if(!m_subtitle)
			m_subtitle = subtitles.get_first();


		update_ui();

		show();

		while(true)
		{
			int response = run();

			if(response == Gtk::RESPONSE_CLOSE || response == Gtk::RESPONSE_DELETE_EVENT)
				break;
		}
	}

	/*
	 *
	 */
	void on_response(int response)
	{
		if(response == FIND)
		{
			if(find_forwards(m_subtitle, m_info))
				m_document->subtitles().select(m_subtitle);
			else
			{
				m_document->subtitles().unselect_all();
				m_info = SearchResult();
				m_subtitle = m_document->subtitles().get_first();

				if(find_forwards(m_subtitle, m_info))
					m_document->subtitles().select(m_subtitle);
			}

			update_ui();
		}
		else if(response == REPLACE)
		{
			replace(m_subtitle, m_info);
			// next
			Gtk::Dialog::response(FIND);
		}
		else if(response == REPLACE_ALL)
		{
			replace_all();
		}
	}
	
	/*
	 *	update sensitive and textview
	 */
	void update_ui()
	{
		Glib::RefPtr<Gtk::TextBuffer> buffer = m_textview->get_buffer();

		bool state = m_info.found;
		m_textview->set_sensitive(state);
		
		m_buttonReplace->set_sensitive(state);
		//m_buttonReplaceAll->set_sensitive(state);

		if(m_info.found && m_info.start != Glib::ustring::npos && m_info.len != Glib::ustring::npos)
		{
			Glib::ustring text = m_subtitle.get_text();
			buffer->set_text(text);
	
			Gtk::TextIter ins = buffer->get_iter_at_offset(m_info.start);
			Gtk::TextIter bound = buffer->get_iter_at_offset(m_info.start + m_info.len);
		
			buffer->apply_tag_by_name("found", ins, bound);
		}
		else
		{
			buffer->set_text("");
		}
	}

protected:
	Document* m_document;
	Subtitle	m_subtitle;
	SearchResult	m_info;

	Gtk::TextView* m_textview;
	Gtk::Entry*	m_entryPattern;
	Gtk::Entry* m_entryReplaceWith;
	Gtk::CheckButton* m_checkIgnoreCase;
	Gtk::CheckButton* m_checkUsedRegularExpression;
	Gtk::Button* m_buttonReplace;
	Gtk::Button* m_buttonReplaceAll;
	Gtk::Button* m_buttonFind;
};

/*
 *	Plugin
 */
class FindAndReplacePlugin : public Action
{
public:

	FindAndReplacePlugin()
	{
		activate();
		update_ui();
	}

	~FindAndReplacePlugin()
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
		action_group = Gtk::ActionGroup::create("FindAndReplacePlugin");

		action_group->add(
				Gtk::Action::create("find-and-replace", Gtk::Stock::FIND_AND_REPLACE, _("_Find And Replace"), _("Search and replace text")), Gtk::AccelKey("<Control>F"),
					sigc::mem_fun(*this, &FindAndReplacePlugin::on_search_and_replace));
		
		action_group->add(
				Gtk::Action::create("find-next",_("Find Ne_xt"), _("Search forwards for the same text")), Gtk::AccelKey("<Control>G"),
					sigc::mem_fun(*this, &FindAndReplacePlugin::on_find_next));
		action_group->add(
				Gtk::Action::create("find-previous",_("Find Pre_vious"), _("Search backwards for the same text")), Gtk::AccelKey("<Shift><Control>G"),
					sigc::mem_fun(*this, &FindAndReplacePlugin::on_find_previous));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-tools' action='menu-tools'>"
			"			<placeholder name='find-and-replace'>"
			"				<menuitem action='find-and-replace'/>"
			"				<menuitem action='find-next'/>"
			"				<menuitem action='find-previous'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);
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

		action_group->get_action("find-and-replace")->set_sensitive(visible);
		action_group->get_action("find-next")->set_sensitive(visible);
		action_group->get_action("find-previous")->set_sensitive(visible);
	}

protected:

	/*
	 *	
	 */
	bool find_in_subtitle(const Subtitle &sub)
	{
		se_debug(SE_DEBUG_PLUGINS);

		bool used_regex = get_config().get_value_bool("dialog-find-and-replace", "used-regular-expression");
		bool ignore_case = get_config().get_value_bool("dialog-find-and-replace", "ignore-case");
		
		Glib::ustring pattern = get_config().get_value_string("dialog-find-and-replace", "find");
		Glib::ustring text = sub.get_text();
		
		int flag = 0;
		if(used_regex)
			flag |= USE_REGEX;
		if(ignore_case)
			flag |= IGNORE_CASE;

		SearchResult info;
		return find(pattern, flag, text, info);
	}


	/*
	 *
	 */
	void on_search_and_replace()
	{
		se_debug(SE_DEBUG_PLUGINS);

		std::auto_ptr<DialogFindAndReplace> dialog(
				gtkmm_utility::get_widget_derived<DialogFindAndReplace>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_GLADE, SE_PLUGIN_PATH_DEV),
						"dialog-find-and-replace.glade", 
						"dialog-find-and-replace"));

		Document *doc = get_current_document();
		dialog->execute(doc);
	}

	/*
	 *
	 */
	void on_find_next()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		Subtitles subtitles = doc->subtitles();

		if(subtitles.size() == 0)
		{
			doc->flash_message(_("The document is empty"));
			return;
		}

		// try to start the search from the subtitle selected
		Subtitle sub = subtitles.get_first_selected();

		if(sub)
		{
			++sub; // start from the next subtitle
			while(sub)
			{
				if(find_in_subtitle(sub))
					break;	// found

				++sub;
			}

			// if not found search again from the beginning
			if(!sub)
			{
				sub = subtitles.get_first();
				while(sub)
				{
					if(find_in_subtitle(sub))
						break;

					++sub;
				}
			}
		}
		else // start the search from the beginning
		{
			sub = subtitles.get_first();
			while(sub)
			{
				if(find_in_subtitle(sub))
					break;
				++sub;
			}
		}


		if(sub)	// found
		{
			doc->subtitles().select(sub);
		}
		else	// not found
		{
			doc->subtitles().unselect_all();
			doc->flash_message(_("Not found"));
		}
	}

	/*
	 *
	 */
	void on_find_previous()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();

		Subtitles subtitles = doc->subtitles();

		if(subtitles.size() == 0)
		{
			doc->flash_message(_("The document is empty"));
			return;
		}

		// try to start the search from the subtitle selected
		Subtitle sub = subtitles.get_first_selected();

		if(sub)
		{
			sub = subtitles.get_previous(sub); // start from the next subtitle
			while(sub)
			{
				if(find_in_subtitle(sub))
					break;	// found

				sub = subtitles.get_previous(sub);
			}

			// if not found search again from the last
			if(!sub)
			{
				sub = subtitles.get_last();
				while(sub)
				{
					if(find_in_subtitle(sub))
						break;

					sub = subtitles.get_previous(sub);
				}
			}
		}
		else // start the search from the last
		{
			sub = subtitles.get_last();
			while(sub)
			{
				if(find_in_subtitle(sub))
					break;
				sub = subtitles.get_previous(sub);
			}
		}


		if(sub)	// found
		{
			doc->subtitles().select(sub);
		}
		else	// not found
		{
			doc->subtitles().unselect_all();
			doc->flash_message(_("Not found"));
		}
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(FindAndReplacePlugin)
