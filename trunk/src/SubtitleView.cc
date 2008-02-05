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
 

#include <iostream>
#include <gtkmm/cellrenderercombo.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/label.h>
#include <gtkmm/eventbox.h>
#include <gdk/gdkkeysyms.h>

#include <gtkmm/textview.h>
#include <gtkmm/celleditable.h>
#include <gtkmm/spinbutton.h>

#include "Config.h"
#include "debug.h"
#include "SubtitleModel.h"

#include "Subtitles.h"
#include "Document.h"
#include "SubtitleView.h"
#include "utility.h"
#include "ActionSystem.h"
#include <gdkmm/window.h>

/*
 *	une CellEditable deriver de TextView qui permet d'avoir plusieur ligne
 *	contrairement a GtkEntry (par defaut)
 */
class TextViewCell : public Gtk::TextView, public Gtk::CellEditable
{
public:

	/*
	 *
	 */
	TextViewCell()
	:	Glib::ObjectBase(typeid(TextViewCell)),
		Gtk::CellEditable()
	{
		se_debug(SE_DEBUG_VIEW);

		m_in_popup = false;

		bool center;
		Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center", center);
	
		if(center)
			set_justification(Gtk::JUSTIFY_CENTER);

		set_wrap_mode(Gtk::WRAP_NONE);
	}

	/*
	 *
	 */
	Glib::ustring get_text() 
	{
		se_debug(SE_DEBUG_VIEW);

		Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();

		Gtk::TextBuffer::iterator start, end;

		buffer->get_bounds(start,end);
		return buffer->get_text(start,end);
	}

	/*
	 *
	 */
	void set_text(const Glib::ustring &text)
	{
		se_debug_message(SE_DEBUG_VIEW, "text=<%s>", text.c_str());

		get_buffer()->set_text(text);
	}

protected:

	/*
	 *
	 */
	bool on_key_press_event(GdkEventKey* event)
	{
		se_debug(SE_DEBUG_VIEW);

		if(event->keyval == GDK_Escape)
		{
			remove_widget();
			return true;
		}
		else if( (
				 event->keyval == GDK_Return ||  
				 event->keyval == GDK_KP_Enter ||  
				 event->keyval == GDK_ISO_Enter ||  
				 event->keyval == GDK_3270_Enter 
				)	&& (event->state & GDK_CONTROL_MASK))
		{
			editing_done();
			remove_widget();
			return true;
		}
		
		Gtk::TextView::on_key_press_event(event);
		return true;
	}

	/*
	 *
	 */
	bool on_focus_out_event(GdkEventFocus *ev)
	{
		se_debug(SE_DEBUG_VIEW);

		// fix #10061 : Title editor field clears too easily
		editing_done();

		return Gtk::TextView::on_focus_out_event(ev);
	}

	/*
	 *
	 */
	void editing_done()
	{
		se_debug(SE_DEBUG_VIEW);

		if(m_in_popup)
			return;

		Gtk::CellEditable::editing_done();
	}
 
	/*
	 *
	 */
	void on_populate_popup (Gtk::Menu* menu)
	{
		se_debug(SE_DEBUG_VIEW);

		m_in_popup = true;

		menu->signal_unmap().connect(
				sigc::mem_fun(*this, &TextViewCell::unmap_popup));
	}

	/*
	 *
	 */
	void unmap_popup()
	{
		se_debug(SE_DEBUG_VIEW);

		m_in_popup = false;
	}

	/*
	 *
	 */
	/*
	bool on_event(GdkEvent *ev)
	{
		print_event("TextViewCell", ev->type);
		return false;
	}
	*/
protected:
	bool m_in_popup;
};

/*
 *
 */
class TimeCell : public Gtk::Entry
{
public:

	TimeCell()
	:Glib::ObjectBase(typeid(TimeCell)), Gtk::Entry()
	{
		se_debug(SE_DEBUG_VIEW);

	}

protected:
	/*
	 *
	 */
	/*
	bool on_event(GdkEvent *ev)
	{
		print_event("TimeCell", ev->type);
		if(ev->type == GDK_FOCUS_CHANGE)
			return true;
		return false;
	}
	*/

	/*
	 *
	 */
	bool on_scroll_event(GdkEventScroll *ev)
	{
		se_debug(SE_DEBUG_VIEW);

		SubtitleTime time(get_text());

		SubtitleTime val = (ev->state & GDK_CONTROL_MASK) ? SubtitleTime(1000) : SubtitleTime(100);

		if(ev->direction == GDK_SCROLL_UP)
		{
			time = time + val;
			set_text(time.str());
			return true;
		}
		else if(ev->direction == GDK_SCROLL_DOWN)
		{
			time = time - val;
			set_text(time.str());
			return true;
		}
		return false;
	}

};


/*
 *
 */
template<class T>
class CellRendererCustom : public Gtk::CellRendererText
{
public:


	/*
	 *
	 */
	CellRendererCustom(Document *doc)
	:
		Glib::ObjectBase(typeid(CellRendererCustom)),
		Gtk::CellRendererText(),
		m_editable(NULL), m_document(doc)
	{
		se_debug(SE_DEBUG_VIEW);
	}
	
	/*
	 *
	 */
	virtual Gtk::CellEditable* start_editing_vfunc(
			GdkEvent* event, 
			Gtk::Widget &widget, 
			const Glib::ustring &path, 
			const Gdk::Rectangle& background_area,
			const Gdk::Rectangle& cell_area,
			Gtk::CellRendererState flags)
	{
		se_debug(SE_DEBUG_VIEW);

		if(!property_editable())
			return NULL;

		m_editable = manage(new T);
		m_editable->set_size_request(cell_area.get_width(), cell_area.get_height());

		m_editable->signal_editing_done().connect(
				sigc::bind(sigc::mem_fun(*this, &CellRendererCustom::cell_editing_done), path));
		
		m_editable->show();

		// prepare widget
		if(Gtk::Entry *entry = dynamic_cast<Gtk::Entry*>(m_editable))
		{
			entry->set_has_frame(false);
			entry->set_alignment(property_xalign());
		}
		m_editable->set_text(property_text());

		// grab
		m_editable->signal_map().connect(
				sigc::mem_fun(*m_editable, &Gtk::Widget::add_modal_grab));

		m_editable->signal_remove_widget().connect(
				sigc::mem_fun(*m_editable, &Gtk::Widget::remove_modal_grab));

		//
		if(m_document && !m_flash_message.empty())
			m_document->flash_message(m_flash_message.c_str());
		
		return m_editable;
	}

	/*
	 *	le texte a afficher a l'edition d'une cellule
	 */
	void set_flash_message(const Glib::ustring &text)
	{
		se_debug(SE_DEBUG_VIEW);

		m_flash_message = text;
	}
protected:

	/*
	 *
	 */
	void cell_editing_done(const Glib::ustring &path)
	{
		se_debug(SE_DEBUG_VIEW);

		if(m_editable == NULL)
			return;

		Glib::ustring text = m_editable->get_text();
			
		// pour eviter un doublon
		m_editable = NULL;

		edited(path, text);
	}

protected:
	T* m_editable;
	Document* m_document;
	Glib::ustring m_flash_message;
};


/*
 *
 */
SubtitleView::SubtitleView(Document &doc)
{
	m_refDocument = &doc;

	m_subtitleModel = m_refDocument->get_subtitle_model();
	m_styleModel = m_refDocument->m_styleModel;
	
	set_model(m_subtitleModel);

	createColumns();

	/*
	// the last column
	{
		Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn());
		append_column(*column);
	}
	*/

	set_rules_hint(true);
	set_enable_search(false);
	set_search_column(m_column.num);
	
	// config
	loadCfg();


	get_selection()->signal_changed().connect(
			sigc::mem_fun(*this, &SubtitleView::on_selection_changed));

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	ActionSystem::getInstance().signal_emit().connect(
			sigc::mem_fun(*this, &SubtitleView::on_execute_action));

	Config::getInstance().signal_changed("subtitle-view").connect(
			sigc::mem_fun(*this, &SubtitleView::on_config_subtitle_view_changed));

	// menu popup
	{
		Gtk::Menu::MenuList &list = m_menu_popup.items();

		list.push_back( Gtk::Menu_Helpers::MenuElem("_Text"));
		list.push_back( Gtk::Menu_Helpers::MenuElem("_Styles"));
		list.push_back( Gtk::Menu_Helpers::MenuElem("_Names"));
	}

	// DnD
	set_reorderable(true);
}

/*
 *
 */
SubtitleView::~SubtitleView()
{
}

/*
 *
 */
void SubtitleView::loadCfg()
{
	se_debug(SE_DEBUG_VIEW);

	bool state = false;
	
	Config &cfg = Config::getInstance();

#define LOAD_CONFIG(name) \
	cfg.get_value_bool("subtitle-view", "show-column-"name, state);	set_column_visible(name, state);

	LOAD_CONFIG("number");
	LOAD_CONFIG("layer");
	LOAD_CONFIG("start");
	LOAD_CONFIG("end");
	LOAD_CONFIG("duration");
	LOAD_CONFIG("style");
	LOAD_CONFIG("name");
	LOAD_CONFIG("margin-l");
	LOAD_CONFIG("margin-r");
	LOAD_CONFIG("margin-v");
	LOAD_CONFIG("effect");
	LOAD_CONFIG("text");
	LOAD_CONFIG("cps");
	LOAD_CONFIG("translation");
	LOAD_CONFIG("note");
	
#undef LOAD_CONFIG

	cfg.get_value_bool("subtitle-view", "enable-rubberband-selection", state);

	set_rubber_banding(state);
}

/*
 *
 */
void SubtitleView::set_tooltips(Gtk::TreeViewColumn *column, const Glib::ustring &text)
{
	se_debug_message(SE_DEBUG_VIEW, "[%s]=%s", column->get_title().c_str(), text.c_str());
/*
	Gtk::EventBox *event = manage(new Gtk::EventBox);

	Gtk::Label* label = manage(new Gtk::Label(column->get_title()));
	event->add(*label);

	Gtk::Tooltips *tooltips = manage(new Gtk::Tooltips);
	tooltips->set_tip(*event, text);
	tooltips->enable();

	column->set_widget(*event);
	event->show_all();
*/
}

/*
 *
 */
void SubtitleView::createColumns()
{
	std::map<Glib::ustring, sigc::slot<void> > columns_creator;

	//
	{
#define set_column_callback(name, function) columns_creator[name] = sigc::mem_fun(*this, &SubtitleView::function);
		
		set_column_callback("number", createColumnNum);
		set_column_callback("layer", createColumnLayer);
		set_column_callback("start", createColumnStart);
		set_column_callback("end", createColumnEnd);
		set_column_callback("duration", createColumnDuration);
		set_column_callback("style", createColumnStyle);
		set_column_callback("name", createColumnName);
		set_column_callback("margin-r", createColumnMarginR);
		set_column_callback("margin-l", createColumnMarginL);
		set_column_callback("margin-v", createColumnMarginV);
		set_column_callback("effect", createColumnEffect);
		set_column_callback("text", createColumnText);
		set_column_callback("cps", createColumnCPS);
		set_column_callback("translation", createColumnTranslation);
		set_column_callback("note", createColumnNote);

#undef set_column_callback
	}


	// load columns order in config file
	std::list<Glib::ustring> columns_order;
	Config::getInstance().get_value_string_list("subtitle-view", "columns", columns_order);

	// il manque des colonnes dans la config
	// on les ajoutes
	if(columns_order.size() != columns_creator.size())
	{
		std::map<Glib::ustring, sigc::slot<void> >::iterator it;
		for(it = columns_creator.begin(); it != columns_creator.end(); ++it)		
		{
			if(std::find(columns_order.begin(), columns_order.end(), it->first) == columns_order.end())
			{
				columns_order.push_back(it->first);
			}
		}
	}

	// creation des colonnes dans l'ordre
	for(std::list<Glib::ustring>::iterator it = columns_order.begin(); it != columns_order.end(); ++it)
	{
		columns_creator[(*it)]();
	}
	
	signal_columns_changed().connect(
			sigc::mem_fun(*this, &SubtitleView::update_columns_order));
}


/*
 *
 */
void SubtitleView::update_columns_order()
{
	se_debug(SE_DEBUG_VIEW);

	std::vector<Gtk::TreeViewColumn*> cols = get_columns();

	// s'il en manque c'est qu'elles sont detruites
	// d'ou l'emission du signal "columns_order"
	// on ne prend donc pas en compte
	if(cols.size() != m_columns.size())
		return;
	
	Glib::ustring new_order;

	for(unsigned int i=0; i<cols.size(); ++i)
	{
		new_order += get_name_of_column(cols[i]) + ";";
	}

	Config::getInstance().set_value_string("subtitle-view", "columns", new_order);
}

/*
 *	create columns
 */
void SubtitleView::createColumnNum()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("num")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.num);
	column->set_reorderable(true);

	append_column(*column);
	//append_column_numeric_editable("num", m_column.num, "%d");
	renderer->property_editable() = false;
	renderer->property_yalign() = 0;
	
	//renderer->property_visible() = false;

	m_columns["number"] = column;

	set_tooltips(column, _("This number column"));
}

/*
 *
 */
void SubtitleView::createColumnLayer()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("layer")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.layer);
	column->set_reorderable(true);

	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_layer));

	append_column(*column);

	m_columns["layer"] = column;

	set_tooltips(column, _("Layer number."));
}

/*
 *
 */
void SubtitleView::createColumnStart()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("start")));
	CellRendererCustom<TimeCell>* renderer = manage(new CellRendererCustom<TimeCell>(m_refDocument));
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.start);
	column->set_reorderable(true);

	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_start));

	append_column(*column);

	m_columns["start"] = column;

	set_tooltips(column, _("This time is the time when a subtitle appears on the screen."));
}

/*
 *
 */
void SubtitleView::createColumnEnd()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("end")));
	CellRendererCustom<TimeCell>* renderer = manage(new CellRendererCustom<TimeCell>(m_refDocument));
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.end);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_end));

	append_column(*column);

	m_columns["end"] = column;

	set_tooltips(column, _("This time is the time when a subtitle disappears from the screen."));
}

/*
 *
 */
void SubtitleView::createColumnDuration()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("duration")));
	CellRendererCustom<TimeCell>* renderer = manage(new CellRendererCustom<TimeCell>(m_refDocument));
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.duration);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_duration));

	append_column(*column);

	m_columns["duration"] = column;
	
	set_tooltips(column, _("The duration of the subtitle."));
}

/*
 *
 */
void SubtitleView::createColumnStyle()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererCombo* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("style")));
	renderer = manage(new Gtk::CellRendererCombo);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.style);
	column->set_reorderable(true);
	
	renderer->property_model() =	m_styleModel;
	renderer->property_text_column() = 0;
	renderer->property_editable() = true;
	renderer->property_has_entry() = false;
	renderer->property_yalign() = 0;
	
	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_style));

	append_column(*column);

	m_columns["style"] = column;
}

/*
 *
 */
void SubtitleView::createColumnName()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("name")));

	CellRendererCustom<TextViewCell>* renderer = manage(new CellRendererCustom<TextViewCell>(m_refDocument));

	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.name);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_name));
	
	append_column(*column);

	m_columns["name"] = column;
}

/*
 *
 */
void SubtitleView::createColumnCPS()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("cps")));

	Gtk::CellRendererText* renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.characters_per_second_text);
	column->set_reorderable(true);

	renderer->property_yalign() = 0;
	renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
	
	append_column(*column);

	m_columns["cps"] = column;
}

/*
 *
 */
void SubtitleView::createColumnText()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	column = manage(new Gtk::TreeViewColumn(_("text")));
	column->set_reorderable(true);

	append_column(*column);

	// text
	{
		CellRendererCustom<TextViewCell>* renderer = manage(new CellRendererCustom<TextViewCell>(m_refDocument));

		column->pack_start(*renderer, true);//false
		column->add_attribute(renderer->property_text(), m_column.text);
	
		renderer->set_flash_message(_("Use Ctrl+Return for exit and Return for line-break"));
		renderer->property_editable() = true;
		renderer->property_yalign() = 0;
	
		renderer->signal_edited().connect(
				sigc::mem_fun(*this, &SubtitleView::on_edited_text));

		bool center;
		Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center", center);
	
		if(center)
		{
			renderer->property_xalign() = 0.5;
			renderer->property_alignment() = Pango::ALIGN_CENTER;
		}

	}
	// cpl
	{
		Gtk::CellRendererText* renderer = NULL;
		renderer = manage(new Gtk::CellRendererText);
	
		column->pack_start(*renderer, false);
		column->add_attribute(renderer->property_text(), m_column.characters_per_line_text);
		renderer->property_yalign() = 0;
		//renderer->property_style() = Pango::STYLE_ITALIC;
		renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
		//renderer->property_attributes() = 

		bool show=true;
		
		Config::getInstance().get_value_bool("subtitle-view", "show-character-per-line", show);

		renderer->property_visible() = show;
	}
	/*
	// cps
	{
		Gtk::CellRendererText* renderer = NULL;
		renderer = manage(new Gtk::CellRendererText);
	
		column->pack_start(*renderer, false);
		column->add_attribute(renderer->property_text(), m_column.characters_per_second_text);
		renderer->property_yalign() = 0;
		renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;

		//bool show=true;
		//Config::getInstance().get_value_bool("subtitle-view", "show-character-per-line", show);
		//renderer->property_visible() = show;
	}
	*/


	column->set_resizable(true);
	//column->set_expand(true);

	m_columns["text"] = column;
}

/*
 *
 */
void SubtitleView::createColumnTranslation()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	column = manage(new Gtk::TreeViewColumn(_("translation")));
	column->set_reorderable(true);

	//translation
	{
		CellRendererCustom<TextViewCell>* renderer = manage(new CellRendererCustom<TextViewCell>(m_refDocument));

		column->pack_start(*renderer, true);
		column->add_attribute(renderer->property_text(), m_column.translation);
	
		append_column(*column);
	
		renderer->set_flash_message(_("Use Ctrl+Return for exit and Return for line-break"));
		renderer->property_editable() = true;
		renderer->property_yalign() = 0;

		bool center;
		Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center", center);

		if(center)
		{
			renderer->property_xalign() = 0.5;
			renderer->property_alignment() = Pango::ALIGN_CENTER;
		}

		renderer->signal_edited().connect(
			sigc::mem_fun(*this, &SubtitleView::on_edited_translation));
	}
	// cpl
	{
		Gtk::CellRendererText* renderer = NULL;
		renderer = manage(new Gtk::CellRendererText);
	
		column->pack_end(*renderer, false);
		column->add_attribute(renderer->property_text(), m_column.characters_per_line_translation);
		renderer->property_yalign() = 0;
		//renderer->property_style() = Pango::STYLE_ITALIC;
		renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
		//renderer->property_attributes() = 
		bool show=true;
		Config::getInstance().get_value_bool("subtitle-view", "show-character-per-line", show);

		renderer->property_visible() = show;
	}

	column->set_resizable(true);
//	column->set_expand(true);

	m_columns["translation"] = column;
}

/*
 *
 */
void SubtitleView::createColumnNote()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("note")));;

	CellRendererCustom<TextViewCell>* renderer = manage(new CellRendererCustom<TextViewCell>(m_refDocument));

	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.note);
	column->set_reorderable(true);
	
	append_column(*column);
	
	renderer->set_flash_message(_("Use Ctrl+Return for exit and Return for line-break"));
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_note));

	column->set_resizable(true);

	m_columns["note"] = column;
}

/*
 *
 */
void SubtitleView::createColumnEffect()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("effect")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.effect);
	column->set_reorderable(true);
	
	append_column(*column);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_effect));

	column->set_resizable(true);
//	column->set_expand(true);

	m_columns["effect"] = column;
}

/*
 *
 */
void SubtitleView::createColumnMarginR()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("R")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginR);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_r));

	append_column(*column);

	m_columns["margin-r"] = column;
}

/*
 *
 */
void SubtitleView::createColumnMarginL()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("L")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginL);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_l));

	append_column(*column);

	m_columns["margin-l"] = column;
}

/*
 *
 */
void SubtitleView::createColumnMarginV()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = manage(new Gtk::TreeViewColumn(_("V")));
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginV);
	column->set_reorderable(true);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_v));

	append_column(*column);

	m_columns["margin-v"] = column;
}

/*
 *	retourne l'item select ou NULL
 */
Gtk::TreeIter SubtitleView::getSelected()
{
	se_debug(SE_DEBUG_VIEW);
	
	Glib::RefPtr<Gtk::TreeSelection> selection = get_selection();
	
	std::vector<Gtk::TreeModel::Path> rows = selection->get_selected_rows();

	if(rows.size() > 0)
	{
		return selection->get_model()->get_iter(rows[0]);
	}

	Gtk::TreeIter null;
	return null;
}

/*
 *
 */
void SubtitleView::on_selection_changed()
{
	se_debug(SE_DEBUG_VIEW);

	m_refDocument->emit_signal("subtitle-selection-changed");
}

/*
 * 
 */
void SubtitleView::on_edited_layer( const Glib::ustring &path, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), value.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		unsigned int val = 0;
		if(from_string(value, val))
		{
			m_refDocument->start_command(_("Editing layer"));
			subtitle.set_layer(value);
			m_refDocument->finish_command();
		}
	}
}


/*
 *	callback utiliser pour modifier le temps directement depuis la list (treeview)
 */
void SubtitleView::on_edited_start( const Glib::ustring &path, 
																		const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(!subtitle)
		return;

	if(!SubtitleTime::validate(newtext))
		return;

	if(subtitle.get("start") == newtext)
		return;

	m_refDocument->start_command(_("Editing start"));
	subtitle.set_start(newtext);
	m_refDocument->emit_signal("subtitle-time-changed");
	m_refDocument->finish_command();
}

/*
 *	callback utiliser pour modifier le temps directement depuis la list (treeview)
 */
void SubtitleView::on_edited_end( const Glib::ustring &path, 
																	const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(!subtitle)
		return;

	if(!SubtitleTime::validate(newtext))
		return;

	if(subtitle.get("end") == newtext)
		return;

	m_refDocument->start_command(_("Editing end"));
	subtitle.set_end(newtext);
	m_refDocument->emit_signal("subtitle-time-changed");
	m_refDocument->finish_command();
}

/*
 *	callback utiliser pour modifier le temps directement depuis la list (treeview)
 */
void SubtitleView::on_edited_duration( const Glib::ustring &path, 
																	const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(!subtitle)
		return;

	if(!SubtitleTime::validate(newtext))
		return;

	if(subtitle.get("duration") == newtext)
		return;

	m_refDocument->start_command(_("Editing duration"));
	subtitle.set_duration(newtext);
	m_refDocument->emit_signal("subtitle-time-changed");
	m_refDocument->finish_command();
}

/*
 *	callback utiliser pour modifier le texte
 */
void SubtitleView::on_edited_text( const Glib::ustring &path, 
																		const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("text") != newtext)
		{
			m_refDocument->start_command(_("Editing text"));
		
			subtitle.set_text(newtext);

			m_refDocument->finish_command();
		}
	}
}

/*
 *	callback utiliser pour modifier le texte
 */
void SubtitleView::on_edited_translation( const Glib::ustring &path, 
																		const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("translation") != newtext)
		{
			m_refDocument->start_command(_("Editing translation"));
			subtitle.set_translation(newtext);
			m_refDocument->finish_command();
		}
	}
}

/*
 *	callback utiliser pour modifier le texte
 */
void SubtitleView::on_edited_note( const Glib::ustring &path, 
																		const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("note") != newtext)
		{
			m_refDocument->start_command(_("Editing note"));
			subtitle.set_note(newtext);
			m_refDocument->finish_command();
		}
	}
}

/*
 */
void SubtitleView::on_edited_effect( const Glib::ustring &path, 
																		const Glib::ustring &newtext)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newtext.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("effect") != newtext)
		{
			m_refDocument->start_command(_("Editing effect"));
			subtitle.set_effect(newtext);
			m_refDocument->finish_command();
		}
	}
}


/*
 *	callback utiliser pour modifier le style a partir d'un menu
 */
void SubtitleView::on_edited_style( const Glib::ustring &path, 
																		const Glib::ustring &newstyle)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newstyle.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("style") != newstyle)
		{
			m_refDocument->start_command(_("Editing style"));
			subtitle.set_style(newstyle);
			m_refDocument->finish_command();
		}
	}
}

/*
 *	callback utiliser pour modifier le nom
 */
void SubtitleView::on_edited_name( const Glib::ustring &path, 
																		const Glib::ustring &newname)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), newname.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		if(subtitle.get("name") != newname)
		{
			m_refDocument->start_command(_("Editing name"));
			subtitle.set_name(newname);
			m_refDocument->finish_command();
		}
	}
}

/*
 * 
 */
void SubtitleView::on_edited_margin_l( const Glib::ustring &path, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), value.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		unsigned int val = 0;
		if(from_string(value, val))
		{
			m_refDocument->start_command(_("Editing margin-l"));
			subtitle.set_margin_l(value);
			m_refDocument->finish_command();
		}
	}
}

/*
 * 
 */
void SubtitleView::on_edited_margin_r( const Glib::ustring &path, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), value.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		unsigned int val = 0;
		if(from_string(value, val))
		{
			m_refDocument->start_command(_("Editing margin-r"));
			subtitle.set_margin_r(value);
			m_refDocument->finish_command();
		}
	}
}

/*
 * 
 */
void SubtitleView::on_edited_margin_v( const Glib::ustring &path, const Glib::ustring &value)
{
	se_debug_message(SE_DEBUG_VIEW, "%s %s", path.c_str(), value.c_str());

	Subtitle subtitle(m_refDocument, path);
	if(subtitle)
	{
		unsigned int val = 0;
		if(from_string(value, val))
		{
			m_refDocument->start_command(_("Editing margin-v"));
			subtitle.set_margin_v(value);
			m_refDocument->finish_command();
		}
	}
}

/*
 *
 */
void SubtitleView::select_and_set_cursor(const Gtk::TreeIter &iter)
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn *column = NULL;
	Gtk::TreeModel::Path path;
	get_cursor(path, column);

	if(column == NULL)
		column = m_columns["text"];

	get_selection()->select(iter);
	set_cursor(m_subtitleModel->get_path(iter), *column, false);
}




/*
 *
 */
bool SubtitleView::on_key_press_event(GdkEventKey *event)
{
	bool is_num = utility::is_num(event->string);

	// seulement si c'est different on met à jour
	if(is_num != get_enable_search())
		set_enable_search(is_num);
	
	return Gtk::TreeView::on_key_press_event(event);
}

/*
 *
 */
bool SubtitleView::on_button_press_event(GdkEventButton *ev)
{
	if( (ev->type == GDK_BUTTON_PRESS) && (ev->button == 3) )
	{
		Gtk::Menu::MenuList &list = m_menu_popup.items();

		// prepare styles menu
		{
			list[1].remove_submenu();

			Gtk::Menu* menu = manage(new Gtk::Menu);

			// "Default"
			menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Default",
							sigc::bind(sigc::mem_fun(*this, &SubtitleView::on_set_style_to_selection), "Default")));
			// separator
			menu->items().push_back(Gtk::Menu_Helpers::SeparatorElem());
			// styles...
			for(Style style = m_refDocument->styles().first(); style; ++style)
			{
				menu->items().push_back(Gtk::Menu_Helpers::MenuElem(style.get("name"),
							sigc::bind(sigc::mem_fun(*this, &SubtitleView::on_set_style_to_selection), style.get("name"))));
			}

			list[1].set_submenu(*menu);
		}

		m_menu_popup.popup(ev->button, ev->time);

		return true;
	}

	return Gtk::TreeView::on_button_press_event(ev);
}

/*
 *
 */
void SubtitleView::on_config_subtitle_view_changed(const Glib::ustring &key, const Glib::ustring &value)
{
	if(key.find("show-column-") != Glib::ustring::npos)
	{
		bool state;
	
		Glib::ustring column = key;
		column.erase(0, Glib::ustring("show-column-").size());

		if(from_string(value, state))
		{
			set_column_visible(column, state);
		}
	}
	else if(key == "property-alignment-center")
	{
		bool state;
		if(from_string(value, state))
		{
			Gtk::CellRendererText *renderer = NULL;
			
			renderer = dynamic_cast<Gtk::CellRendererText*>(m_columns["text"]->get_first_cell_renderer());
			renderer->property_xalign() = state ? 0.5 : 0.0;
			renderer->property_alignment() = state ? Pango::ALIGN_CENTER : Pango::ALIGN_LEFT;

			renderer = dynamic_cast<Gtk::CellRendererText*>(m_columns["translation"]->get_first_cell_renderer());
			renderer->property_xalign() = state ? 0.5 : 0.0;
			renderer->property_alignment() = state ? Pango::ALIGN_CENTER : Pango::ALIGN_LEFT;
		}

		queue_draw();
	}
	else if(key == "show-character-per-line")
	{
		bool state;
		if(from_string(value, state))
		{
			std::vector<Gtk::CellRenderer*> cells;
			
			cells = m_columns["text"]->get_cell_renderers();
			cells[1]->property_visible() = state;

			cells = m_columns["translation"]->get_cell_renderers();
			cells[1]->property_visible() = state;
		}
		queue_draw();
	}
	else if(key == "enable-rubberband-selection")
	{
		set_rubber_banding(utility::string_to_bool(value));
	}
}

/*
 *
 */
void SubtitleView::on_set_style_to_selection(const Glib::ustring &name)
{
	std::vector<Subtitle> selection = m_refDocument->subtitles().get_selection();

	if(selection.empty())
		return;

	m_refDocument->start_command(_("Set style to selection"));
	for(unsigned int i=0; i<selection.size(); ++i)
	{
		selection[i].set("style", name);
	}
	m_refDocument->finish_command();
}

/*
 *
 */
void SubtitleView::on_execute_action(const Glib::ustring &action)
{
	if(action == "cut")
		clipboard_cut();
	else if(action == "copy")
		clipboard_copy();
	else if(action == "paste")
		clipboard_paste();
	else if(action.find("view/") != Glib::ustring::npos)
	{
		Config &cfg = Config::getInstance();

#define SET_COLUMN(name, value) if(get_column_visible(name) != value) cfg.set_value_bool("subtitle-view", "show-column-"name, value);

		if(action == "view/simple")
		{
			SET_COLUMN("number", true);
			SET_COLUMN("layer", false);
			SET_COLUMN("start", true);
			SET_COLUMN("end", true);
			SET_COLUMN("duration", true);
			SET_COLUMN("style", false);
			SET_COLUMN("name", false);
			SET_COLUMN("margin-r", false);
			SET_COLUMN("margin-l", false);
			SET_COLUMN("margin-v", false);
			SET_COLUMN("effect", false);
			SET_COLUMN("text", true);
			SET_COLUMN("translation", false);
			SET_COLUMN("note", false);
			SET_COLUMN("cps", false);
		}
		else if(action == "view/advanced")
		{
			SET_COLUMN("number", true);
			SET_COLUMN("layer", true);
			SET_COLUMN("start", true);
			SET_COLUMN("end", true);
			SET_COLUMN("duration", true);
			SET_COLUMN("style", true);
			SET_COLUMN("name", true);
			SET_COLUMN("margin-r", false);
			SET_COLUMN("margin-l", false);
			SET_COLUMN("margin-v", false);
			SET_COLUMN("effect", false);
			SET_COLUMN("text", true);
			SET_COLUMN("translation", false);
			SET_COLUMN("note", false);
			SET_COLUMN("cps", false);
		}
		else if(action == "view/translation")
		{
			SET_COLUMN("number", true);
			SET_COLUMN("layer", false);
			SET_COLUMN("start", false);
			SET_COLUMN("end", false);
			SET_COLUMN("duration", false);
			SET_COLUMN("style", false);
			SET_COLUMN("name", false);
			SET_COLUMN("margin-r", false);
			SET_COLUMN("margin-l", false);
			SET_COLUMN("margin-v", false);
			SET_COLUMN("effect", false);
			SET_COLUMN("text", true);
			SET_COLUMN("translation", true);
			SET_COLUMN("note", false);
			SET_COLUMN("cps", false);
		}
		else if(action == "view/timing")
		{
			SET_COLUMN("number", true);
			SET_COLUMN("layer", false);
			SET_COLUMN("start", true);
			SET_COLUMN("end", true);
			SET_COLUMN("duration", true);
			SET_COLUMN("style", false);
			SET_COLUMN("name", false);
			SET_COLUMN("margin-r", false);
			SET_COLUMN("margin-l", false);
			SET_COLUMN("margin-v", false);
			SET_COLUMN("effect", false);
			SET_COLUMN("text", true);
			SET_COLUMN("translation", false);
			SET_COLUMN("note", false);
			SET_COLUMN("cps", true);
		}

#undef SET_COLUMN

	}
}


/*
 *	COLUMN
 */

/*
 *	retourne la colonne par rapport a son nom (interne)
 */
Gtk::TreeViewColumn* SubtitleView::get_column_by_name(const Glib::ustring &name)
{
	std::map<Glib::ustring, Gtk::TreeViewColumn*>::iterator it = m_columns.find(name);

	if(it != m_columns.end())
		return it->second;

	return NULL;
}

/*
 *	retourne le nom utiliser en interne de la column
 */
Glib::ustring SubtitleView::get_name_of_column(Gtk::TreeViewColumn *column)
{
	std::map<Glib::ustring, Gtk::TreeViewColumn*>::iterator it;
	for(it = m_columns.begin(); it != m_columns.end(); ++it)
	{
		if(it->second == column)
			return it->first;
	}
	
	return Glib::ustring();
}

/*
 *
 */
void SubtitleView::set_column_visible(const Glib::ustring& name, bool state)
{
	se_debug_message(SE_DEBUG_VIEW, "%s=%s", name.c_str(), state ? "true" : "false");

	Gtk::TreeViewColumn *column = get_column_by_name(name);

	g_return_if_fail(column);

	column->set_visible(state);
}

/*
 *
 */
bool SubtitleView::get_column_visible(const Glib::ustring &name)
{
	Gtk::TreeViewColumn *column = get_column_by_name(name);

	g_return_val_if_fail(column, false);

	se_debug_message(SE_DEBUG_VIEW, "<%s> = %s", name.c_str(), column->get_visible() ? "true" : "false");

	return column->get_visible();
}


/*
 *
 */
void SubtitleView::clipboard_cut()
{
}

/*
 *
 */
void SubtitleView::clipboard_copy()
{
	/*
	Gtk::TreeViewColumn *column = NULL;
	Gtk::TreeModel::Path path;
	get_cursor(path, column);

	if(column == NULL)
		column = m_columns["text"];

	Subtitle sub(m_refDocument, m_subtitleModel->get_iter(path));

	g_return_if_fail(sub);

	//set_cursor(m_subtitleModel->get_path(iter), *column, false);
	*/
	Glib::ustring key = "text";

	Glib::ustring text;

	std::vector<Subtitle> selection = m_refDocument->subtitles().get_selection();
	for(unsigned int i=0; i< selection.size(); ++i) 
	{
		text += selection[i].get(key);
	}

	Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
	clipboard->set_text(text);
}

/*
 * 
 */
void SubtitleView::clipboard_paste()
{
}

