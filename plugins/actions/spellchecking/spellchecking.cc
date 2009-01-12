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
 
#include <enchant++.h>
#include <iostream>
#include <extension/action.h>
#include <utility.h>
#include <isocodes.h>
#include <gtkmm_utility.h>
#include "spellchecking.h"
#include <memory>

/*
 *	erreur de compilation si j'utilise enchant++.h dans le header :(
 */
static enchant::Dict* m_enchantDict = NULL;

/*
 *
 */
std::map<Glib::ustring, Glib::ustring> m_iso_to_lang;

/*
 *
 */
Glib::ustring get_language_by_abrev(const Glib::ustring &abrev)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "abrev=<%s>", abrev.c_str());

	if(m_iso_to_lang.find(abrev) != m_iso_to_lang.end())
	{
		return m_iso_to_lang[abrev];
	}
	
	Glib::ustring lang = isocodes::to_name(abrev);

	m_iso_to_lang[abrev] = lang;
	
	return lang;
}

/*
 *
 */
Glib::ustring get_abrev_by_language(const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "language=<%s>", name.c_str());

	std::map<Glib::ustring, Glib::ustring>::const_iterator it;

	for(it = m_iso_to_lang.begin(); it != m_iso_to_lang.end(); ++it)
	{
		if(it->second == name)
			return it->first;
	}
	return "";
}




/*
 *	callback utiliser pour recuperer les dicts
 */
void callback_list_dicts(const char *const lang_tag,
		const char *const provider_name,
		const char *const provider_desc,
		const char *const provider_file,
		void *user_data)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "%s %s %s %s", 
			lang_tag, provider_name, provider_desc, provider_file);

	std::list<Glib::ustring> *list = (std::list<Glib::ustring>*)user_data;

	Glib::ustring name = get_language_by_abrev(lang_tag);

	if(!name.empty())
		list->push_back(name);
}


/*
 *
 */
DialogSpellChecking::DialogSpellChecking(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
:Gtk::Dialog(cobject)
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	utility::set_transient_parent(*this);

	// get widgets
	refGlade->get_widget("textview", m_textview);

	refGlade->get_widget("entry-replace-with", m_entryReplaceWith);
	refGlade->get_widget("button-check-word", m_buttonCheckWord);
	
	refGlade->get_widget("treeview-suggestions", m_treeviewSuggestions);

	create_treeview_suggestions();

	refGlade->get_widget("button-replace", m_buttonReplace);
	refGlade->get_widget("button-ignore", m_buttonIgnore);
	refGlade->get_widget("button-ignore-all", m_buttonIgnoreAll);
	refGlade->get_widget("button-add-word", m_buttonAddWord);


	refGlade->get_widget_derived("combobox-dicts", m_comboboxDicts);

	refGlade->get_widget("label-completed-spell-checking", m_labelCompletedSpellChecking);

	// connect signal
	m_buttonCheckWord->signal_clicked().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_check_word));
	
	m_buttonReplace->signal_clicked().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_replace));
	
	m_buttonIgnore->signal_clicked().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_ignore));
	
	m_buttonIgnoreAll->signal_clicked().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_ignore_all));
	
	m_buttonAddWord->signal_clicked().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_add_word));

	m_treeviewSuggestions->get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_treeview_suggestions_selection_changed));


	// init ui sensitive
	m_textview->set_editable(false);
	m_textview->set_sensitive(false);
	Glib::RefPtr<Gtk::TextTag> bold = m_textview->get_buffer()->create_tag("word");
	bold->property_weight() = Pango::WEIGHT_BOLD;
	bold->property_foreground() = "red";
	//bold->property_underline() = Pango::UNDERLINE_ERROR;


	m_entryReplaceWith->set_sensitive(false);
	m_buttonCheckWord->set_sensitive(false);
	m_treeviewSuggestions->set_sensitive(false);
	m_buttonIgnore->set_sensitive(false);
	m_buttonIgnoreAll->set_sensitive(false);
	m_buttonReplace->set_sensitive(false);
	m_buttonAddWord->set_sensitive(false);

	m_labelCompletedSpellChecking->hide();


	// recupere la list des dicts
	std::list<Glib::ustring> list_dicts;
	enchant::Broker::instance()->list_dicts(callback_list_dicts, &list_dicts);

	list_dicts.sort();
	for(std::list<Glib::ustring>::iterator it = list_dicts.begin(); it != list_dicts.end(); ++it)
		m_comboboxDicts->append_text(*it);

	// config dicts
	Glib::ustring lang, tmp_lang;

	// lecture de la config
	if(Config::getInstance().get_value_string("spell-checking", "lang", tmp_lang))
	{
		if(set_dict(tmp_lang))
		{
			lang = tmp_lang;
		}
	}

	// s'il n'y a pas de config utiliser LANG
	if(lang.empty())
	{
		Glib::ustring tmp_lang = Glib::getenv("LANG");

		Glib::ustring::size_type p = tmp_lang.find(".");
		if( p != Glib::ustring::npos)
		{
			if(set_dict(tmp_lang.substr(0, p)))
				lang = tmp_lang.substr(0, p);
		}
		else
		{
			if(set_dict(tmp_lang))
				lang = tmp_lang;
		}
	}

	// init le combobox
	if(!lang.empty())
		m_comboboxDicts->set_active_text(get_language_by_abrev(lang));

	m_comboboxDicts->signal_changed().connect(
			sigc::mem_fun(*this, &DialogSpellChecking::on_combobox_dicts_changed));
}

/*
 *
 */
DialogSpellChecking::~DialogSpellChecking()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	delete m_enchantDict;
	m_enchantDict = NULL;
}

/*
 *
 */
bool DialogSpellChecking::set_dict(const Glib::ustring &name)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "lang=<%s>", name.c_str());


	if(m_enchantDict)
		delete m_enchantDict;

	m_enchantDict = NULL;

	try
	{
		m_enchantDict = enchant::Broker::instance()->request_dict(name);

		Config::getInstance().set_value_string("spell-checking", "lang", name);

		//m_treeviewSuggestions->set_sensitive(true);

		return true;
	}
	catch(enchant::Exception &ex)
	{
		std::cerr << ex.what() << std::endl;
		m_enchantDict = NULL;
	}

	return false;
}

/*
 *
 */
void DialogSpellChecking::create_treeview_suggestions()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	m_listSuggestions = Glib::RefPtr<ListSuggestions>(new ListSuggestions);

	m_treeviewSuggestions->set_model(m_listSuggestions);

	Gtk::TreeViewColumn *column = NULL;
	Gtk::CellRendererText *renderer = NULL;

	column = manage(new Gtk::TreeViewColumn("Suggestions"));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_listSuggestions->m_column.string);
	
	m_treeviewSuggestions->append_column(*column);	
}

/*
 *
 */
void DialogSpellChecking::execute(Document *doc)
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	m_current_word = "";
	m_current_text = "";
	m_current_word_start = 0;
	m_current_next_word = 0;
	m_current_subtitle = doc->subtitles().get_first();

	if(m_current_subtitle)
		check_line(m_current_subtitle);

	doc->start_command(_("Spell Checking"));
	run();
	doc->finish_command();
}

/*
 *	verifie le mot "word"
 *	return false en cas d'erreur dans le mot
 */
bool DialogSpellChecking::check_word(const Glib::ustring &word)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "word=<%s>", word.c_str());

	try
	{
		if(m_enchantDict)
			return m_enchantDict->check(word);
	}
	catch(std::exception &ex) // enchant::Exception
	{
		se_debug_message(SE_DEBUG_SPELL_CHECKING, "exception '%s'", ex.what());
		dialog_error("Failed to check word", ex.what());
	}
	catch(...)
	{
		se_debug_message(SE_DEBUG_SPELL_CHECKING, "unknow exception");
		dialog_error("Failed to check word", "Unknow exception");
	}
	return false;
}

/*
 *	recupere le texte de l'iter
 *	init les m_current_xxx
 *	et appel check_text pour verifie le text
 */
bool DialogSpellChecking::check_line(Subtitle subtitle)
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	if(subtitle)
	{	
		m_current_word.clear();
		m_current_word_start = 0;
		m_current_next_word = 0;

		m_current_text = subtitle.get_text();

		return check_text();
	}
	
	return false;
}

/*
 * recupere le mot suivant a partir des info de m_current_word_
 */
bool DialogSpellChecking::check_text()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	if(!m_current_text.empty())
	{
		for(unsigned int i=m_current_next_word; i<=m_current_text.size(); ++i)
		{
			if(is_end_char(m_current_text[i]))
			{
				m_current_word_start = m_current_next_word;

				m_current_word = m_current_text.substr(m_current_word_start, i-m_current_next_word);

				m_current_next_word = i+1;

				if(!m_current_word.empty())
				{
					if(check_word(m_current_word) == false)
					{
						init_with_word(m_current_text, m_current_word);
						return true;
					}
				}
			}
		}
	}

	return check_next_line();
}

/*
 *	passe a la ligne suivante grace a l'iter (m_current_subtitle)
 *	s'il n'y a pas de ligne suivante alors on init l'interface 
 *	set_sensitive(false) et affiche le message "Completed spell checking."
 */
bool DialogSpellChecking::check_next_line()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	if(m_current_subtitle)
	{
		++m_current_subtitle;
	
		if(m_current_subtitle)
			return check_line(m_current_subtitle);
	}
	
	m_textview->get_buffer()->set_text("");
	m_listSuggestions->clear();

	m_textview->set_sensitive(false);
	m_entryReplaceWith->set_sensitive(false);
	m_buttonCheckWord->set_sensitive(false);
	m_treeviewSuggestions->set_sensitive(false);
	m_buttonReplace->set_sensitive(false);
	m_buttonIgnore->set_sensitive(false);
	m_buttonIgnoreAll->set_sensitive(false);
	m_buttonAddWord->set_sensitive(false);
	
	m_labelCompletedSpellChecking->show();
	
	return false;
}

/*
 *	verifie les mots suivant
 */
bool DialogSpellChecking::check_next_word()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	return check_text();
}

/*
 *	verifie la lettre pour savoir si c'est un nouveau mot
 *	, . " * 123456789, ...
 */
bool DialogSpellChecking::is_end_char(gchar c)
{
	switch(c)
	{
	case ' ':
	//case '\'':
	case '\0':
	case '\n':
	case '.':
	case ',':
	case ':':
	case ';':
	case '*':
	case '+':
	case '=':
	case '\t':
	case '(':
	case ')':
	case '!':
	case '|':
	case '?':
	case '-':
	case '"':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return true;
	default:
	 return false;
	}
	return false;
}

/*
 *	init l'interface avec ce mot
 *	textview select
 *	list suggestion
 *	...
 */
void DialogSpellChecking::init_with_word(const Glib::ustring &text, const Glib::ustring &word)
{
	se_debug_message(SE_DEBUG_SPELL_CHECKING, "text=<%s> word=<%s>", text.c_str(), word.c_str());

	m_textview->set_sensitive(true);
	m_treeviewSuggestions->set_sensitive(true);
	
	m_buttonCheckWord->set_sensitive(true);
	m_entryReplaceWith->set_sensitive(true);

	m_buttonReplace->set_sensitive(true);
	m_buttonIgnore->set_sensitive(true);
	m_buttonIgnoreAll->set_sensitive(true);

	m_buttonAddWord->set_sensitive(true	);

	m_entryReplaceWith->set_text("");

	m_textview->get_buffer()->set_text(text);
	/*
	std::vector<std::string> suggestions;

	if(m_enchantDict)
		m_enchantDict->suggest(word, suggestions);

	m_listSuggestions->clear();

	for(unsigned int i=0; i<suggestions.size(); ++i)
		m_listSuggestions->add(suggestions[i]);
	*/
	init_suggestions(word);

	Gtk::TextIter ins, bound;

	Glib::RefPtr<Gtk::TextBuffer> buffer = m_textview->get_buffer();

	ins = buffer->get_iter_at_offset(m_current_word_start);
	bound = buffer->get_iter_at_offset(m_current_word_start + m_current_word.size());

	//buffer->select_range(ins, bound);
	buffer->apply_tag_by_name("word", ins, bound);
}

/*
 *	init la list des suggestions par rapport au mot "word"
 */
void DialogSpellChecking::init_suggestions(const Glib::ustring &word)
{
	m_listSuggestions->clear();

	if(m_enchantDict)
	{
		std::vector<std::string> suggestions;
	
		m_enchantDict->suggest(word, suggestions);

		for(unsigned int i=0; i<suggestions.size(); ++i)
			m_listSuggestions->add(suggestions[i]);
	}
}


/*
 *	callback 
 */

/*
 *	remplace le mot par celui de la list des suggestions (m_entryReplaceWith)
 */
void DialogSpellChecking::on_replace()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	Glib::ustring new_word = m_entryReplaceWith->get_text();

	m_current_text.replace(m_current_word_start, m_current_word.size(), new_word);

	m_current_subtitle.set_text(m_current_text);

	m_current_next_word += (new_word.size() - m_current_word.size());

	check_next_word();
}

/*
 *	passe simplement au mot suivant
 */
void DialogSpellChecking::on_ignore()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	check_next_word();
}

/*
 *	ajoute le mot dans le dico de la session
 */
void DialogSpellChecking::on_ignore_all()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	if(!m_current_word.empty() && m_enchantDict)
		m_enchantDict->add_to_session(m_current_word);
	
	check_next_word();
}

/*
 *	ajoute le mot dans le dico perso
 */
void DialogSpellChecking::on_add_word()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	if(!m_current_word.empty() && m_enchantDict)
		m_enchantDict->add_to_pwl(m_current_word);

	check_next_word();
}

/*
 *	la selection a changer dans la list des suggestions
 *	init l'interface selon le choix
 */
void DialogSpellChecking::on_treeview_suggestions_selection_changed()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	Gtk::TreeIter iter = m_treeviewSuggestions->get_selection()->get_selected();
	if(iter)
	{
		//m_buttonReplace->set_sensitive(true);
		//m_entryReplaceWith->set_sensitive(true);

		Glib::ustring word = (*iter)[m_listSuggestions->m_column.string];

		m_entryReplaceWith->set_text(word);
	}
	else
	{
		//m_buttonReplace->set_sensitive(false);
		//m_entryReplaceWith->set_sensitive(false);
	}
}

/*
 *
 */
void DialogSpellChecking::on_combobox_dicts_changed()
{
	se_debug(SE_DEBUG_SPELL_CHECKING);

	Glib::ustring name = m_comboboxDicts->get_active_text();

	// on recupere le code du dico
	Glib::ustring abrev = get_abrev_by_language(name);

	if(!abrev.empty())
	{
		set_dict(abrev);
	}
	check_next_word();
}

/*
 *	quand on utilise le bouton "Check Word" (m_buttonCheckWord)
 *	verifie le mot dans "Replace With:" (m_entryReplaceWith)
 */
void DialogSpellChecking::on_check_word()
{
	Glib::ustring word = m_entryReplaceWith->get_text();

	if(!word.empty())
	{
		//if(check_word(word) == false)
		{
			init_suggestions(word);
		}
	}
}


/*
 *
 */
class SpellCheckingPlugin : public Action
{
public:

	SpellCheckingPlugin()
	{
		activate();
		update_ui();
	}

	~SpellCheckingPlugin()
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
		action_group = Gtk::ActionGroup::create("SpellCheckingPlugin");

		action_group->add(
				Gtk::Action::create("spell-checking", Gtk::Stock::SPELL_CHECK, _("_Spell Check"), _("Launch the spell checking")), Gtk::AccelKey("F7"),
					sigc::mem_fun(*this, &SpellCheckingPlugin::on_execute));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		ui->add_ui(ui_id, "/menubar/menu-tools/checking", "spell-checking", "spell-checking");
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

		action_group->get_action("spell-checking")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_execute()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Document *doc = get_current_document();
		g_return_if_fail(doc);

		// create dialog
		std::auto_ptr<DialogSpellChecking> dialog(
				gtkmm_utility::get_widget_derived<DialogSpellChecking>(
						SE_DEV_VALUE(SE_PLUGIN_PATH_GLADE, SE_PLUGIN_PATH_DEV),
						"dialog-spell-checking.glade", 
						"dialog-spell-checking"));

		dialog->execute(doc);
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_EXTENSION(SpellCheckingPlugin)
