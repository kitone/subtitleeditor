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
#include "SubtitleEditorWindow.h"


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
	bool on_scroll_event(GdkEventScroll *ev)
	{
		se_debug(SE_DEBUG_VIEW);

		Glib::ustring text = get_text();
		long frame;

		if(SubtitleTime::validate(text)) // TIME
		{
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
		}
		else if(from_string(text, frame)) // FRAME
		{
			long val = (ev->state & GDK_CONTROL_MASK) ? 10 : 1;

			if(ev->direction == GDK_SCROLL_UP)
				frame += val;
			else if(ev->direction == GDK_SCROLL_DOWN)
				frame -= val;

			set_text(to_string(frame));
			
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
		
		// prepare widget
		if(Gtk::Entry *entry = dynamic_cast<Gtk::Entry*>(m_editable))
		{
			entry->set_has_frame(false);
			entry->set_alignment(property_xalign());
		}
		m_editable->set_text(property_text());


		// Begin/Finish editing (Fix #10494)
		// Disable actions during editing. Enable at the exit. 
		begin_editing();

		m_editable->signal_remove_widget().connect(
				sigc::mem_fun(*this, &CellRendererCustom::finish_editing));

		//
		if(m_document && !m_flash_message.empty())
			m_document->flash_message(m_flash_message.c_str());
		
		m_editable->show();

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
	 * Enable or disable all actions so as not to interfere with editing. 
	 * As a simple shorcuts.
	 */
	void set_action_groups_sensitives(bool state)
	{
		std::list< Glib::RefPtr<Gtk::ActionGroup> > actions = 
			SubtitleEditorWindow::get_instance()->get_ui_manager()->get_action_groups();

		std::list< Glib::RefPtr<Gtk::ActionGroup> >::iterator it;
		for(it = actions.begin(); it != actions.end(); ++it)
		{
			(*it)->set_sensitive(state);
		} 
	}

	/*
	 * Disable all actions.
	 */
	void begin_editing()
	{
		set_action_groups_sensitives(false);
	}

	/*
	 * Enable all actions.
	 */
	void finish_editing()
	{
		set_action_groups_sensitives(true);
	}

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
 * Represents a cell time.
 */
class CellRendererTime : public CellRendererCustom<TimeCell>
{
public:
	CellRendererTime(Document *doc)
	:CellRendererCustom<TimeCell>(doc)
	{
		property_editable() = true;
		property_yalign() = 0.0;
		property_xalign() = 1.0;
		property_alignment() = Pango::ALIGN_RIGHT;
	}
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

	// Update the columns size
	m_refDocument->get_signal("edit-timing-mode-changed").connect(
			sigc::mem_fun(*this, &Gtk::TreeView::columns_autosize));
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
	createColumnNum();
	createColumnLayer();
	createColumnStart();
	createColumnEnd();
	createColumnDuration();
	createColumnStyle();
	createColumnName();
	createColumnMarginR();
	createColumnMarginL();
	createColumnMarginV();
	createColumnEffect();
	createColumnText();
	createColumnCPS();
	createColumnTranslation();
	createColumnNote();

	update_columns_displayed_from_config();
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
void SubtitleView::create_column_time(
		const Glib::ustring &name, 
		const Glib::ustring &label, 
		const Gtk::TreeModelColumnBase& column_attribute,
		const sigc::slot<void, const Glib::ustring&, const Glib::ustring&> &slot, 
		const Glib::ustring &tooltips)
{
	se_debug_message(SE_DEBUG_VIEW, "name=%s label=%l tooltips=%s", 
			name.c_str(), label.c_str(), tooltips.c_str());


	CellRendererTime* renderer = manage(new CellRendererTime(m_refDocument));
	
	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(label));

	column->pack_start(*renderer);
	column->add_attribute(renderer->property_text(), column_attribute);
	
	renderer->signal_edited().connect(slot);

	append_column(*column);

	m_columns[name] = column;

	set_tooltips(column, tooltips);
}

/*
 *
 */
void SubtitleView::createColumnStart()
{
	create_column_time(
			"start", 
			_("start"), 
			m_column.start, 
			sigc::mem_fun(*this, &SubtitleView::on_edited_start),
			_("This time is the time when a subtitle appears on the screen."));
}

/*
 *
 */
void SubtitleView::createColumnEnd()
{
	create_column_time(
			"end", 
			_("end"), 
			m_column.end, 
			sigc::mem_fun(*this, &SubtitleView::on_edited_end),
			_("This time is the time when a subtitle disappears from the screen."));

}

/*
 *
 */
void SubtitleView::createColumnDuration()
{
	create_column_time(
			"duration", 
			_("duration"), 
			m_column.duration, 
			sigc::mem_fun(*this, &SubtitleView::on_edited_duration),
			 _("The duration of the subtitle."));
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
	
	append_column(*column);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_effect));

	column->set_resizable(true);

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

	if(subtitle.get("start") == newtext)
		return;

	if(m_refDocument->get_edit_timing_mode() == TIME)
	{
		if(!SubtitleTime::validate(newtext))
			return;

		m_refDocument->start_command(_("Editing start"));
		subtitle.set_start(newtext);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
	else // edit_mode == FRAME
	{
		long frame = 0;
		if(!from_string(newtext, frame))
			return;

		m_refDocument->start_command(_("Editing start"));
		subtitle.set_start_frame(frame);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
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

	if(subtitle.get("end") == newtext)
		return;

	if(m_refDocument->get_edit_timing_mode() == TIME)
	{
		if(!SubtitleTime::validate(newtext))
			return;

		m_refDocument->start_command(_("Editing end"));
		subtitle.set_end(newtext);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
	else // edit_mode == FRAME
	{
		long frame = 0;
		if(!from_string(newtext, frame))
			return;

		m_refDocument->start_command(_("Editing end"));
		subtitle.set_end_frame(frame);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
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

	if(subtitle.get("duration") == newtext)
		return;

	if(m_refDocument->get_edit_timing_mode() == TIME)
	{
		if(!SubtitleTime::validate(newtext))
			return;

		m_refDocument->start_command(_("Editing duration"));
		subtitle.set_duration(newtext);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
	else // edit_mode == FRAME
	{
		long frame = 0;
		if(!from_string(newtext, frame))
			return;

		m_refDocument->start_command(_("Editing duration"));
		subtitle.set_duration_frame(frame);
		m_refDocument->emit_signal("subtitle-time-changed");
		m_refDocument->finish_command();
	}
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
	if(key == "columns-displayed")
	{
		update_columns_displayed_from_config();
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

	se_debug_message(SE_DEBUG_VIEW, "column: %s return NULL", name.c_str());

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
 *	Get the columns displayed from the configuration and updates.
 */
void SubtitleView::update_columns_displayed_from_config()
{
	se_debug(SE_DEBUG_VIEW);

	Glib::ustring columns;

	if(!Config::getInstance().get_value_string("subtitle-view", "columns-displayed", columns))
	{
		g_warning("update_columns_displayed_from_config FAILED");
		return;
	}

	// get columns order
	std::vector<std::string> cols;
		
	utility::split(columns, ';', cols);

	// hide all columns
	std::map<Glib::ustring, Gtk::TreeViewColumn*>::iterator it;
	for( it = m_columns.begin(); it != m_columns.end(); ++it)
	{
		it->second->set_visible(false);
	}

	// reorder columns
	Gtk::TreeViewColumn *current_column = NULL;

	for(unsigned int i=0; i< cols.size(); ++i)
	{
		Glib::ustring name = cols[i];

		if(current_column)
		{
			Gtk::TreeViewColumn *tmp = get_column_by_name(name);
			if(tmp)
				move_column_after(*tmp, *current_column);
			current_column = tmp;
		}
		else	// it's the first, put at start
		{
			current_column = get_column_by_name(name);
			if(current_column)
				move_column_to_start(*current_column);
		}

		// display column
		if(current_column)
			current_column->set_visible(true);
	}
	
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

/*
 * This is a static function. 
 * Return the humain label by the internal name of the column.
 */
Glib::ustring SubtitleView::get_column_label_by_name(const Glib::ustring &name)
{
	std::map<Glib::ustring, Glib::ustring> columns_labels;

	columns_labels["cps"] = _("cps");
	columns_labels["duration"] = _("duration");
	columns_labels["effect"] = _("effect");
	columns_labels["end"] = _("end");
	columns_labels["layer"] = _("layer");
	columns_labels["margin-l"] = _("L");
	columns_labels["margin-r"] = _("R");
	columns_labels["margin-v"] = _("V");
	columns_labels["name"] = _("name");
	columns_labels["note"] = _("note");
	columns_labels["number"] = _("num");
	columns_labels["start"] = _("start");
	columns_labels["style"] = _("style");
	columns_labels["text"] = _("text");
	columns_labels["translation"] = _("translation");

	std::map<Glib::ustring, Glib::ustring>::iterator it = columns_labels.find(name);
	if(it != columns_labels.end())
		return it->second;

	return Glib::ustring("Invalid : ") + name;
}

