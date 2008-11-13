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

		m_canceled = false;
		m_in_popup = false;

		m_used_ctrl_enter_to_confirm_change = Config::getInstance().get_value_bool("subtitle-view", "used-ctrl-enter-to-confirm-change");

		if(Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center"))
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
			m_canceled = true;
			remove_widget();
			return true;
		}
		
		bool st_enter = (
				 event->keyval == GDK_Return ||  
				 event->keyval == GDK_KP_Enter ||  
				 event->keyval == GDK_ISO_Enter ||  
				 event->keyval == GDK_3270_Enter );

		bool st_ctrl = (event->state & GDK_CONTROL_MASK);

		if((m_used_ctrl_enter_to_confirm_change ? (st_enter && st_ctrl) : (st_enter && !st_ctrl)))
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
		if(!m_canceled)
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
	bool m_canceled;
	bool m_used_ctrl_enter_to_confirm_change;
};

/*
 *
 */
class TimeCell : public Gtk::TextView, public Gtk::CellEditable
{
public:

	TimeCell()
	:Glib::ObjectBase(typeid(TimeCell)), Gtk::CellEditable()
	{
		se_debug(SE_DEBUG_VIEW);
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
			//m_canceled = true;
			remove_widget();
			return true;
		}
		
		bool st_enter = (
				 event->keyval == GDK_Return ||  
				 event->keyval == GDK_KP_Enter ||  
				 event->keyval == GDK_ISO_Enter ||  
				 event->keyval == GDK_3270_Enter );

		if(st_enter)
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
	bool on_scroll_event(GdkEventScroll *ev)
	{
		se_debug(SE_DEBUG_VIEW);

		Glib::ustring text = get_text();
		long frame;

		if(SubtitleTime::validate(text)) // TIME
		{
			SubtitleTime time(get_text());

			long step = 100;

			if(ev->state & GDK_SHIFT_MASK && ev->state & GDK_CONTROL_MASK)
				step *= 100;
			else if(ev->state & GDK_CONTROL_MASK)
				step *= 10;
			
			SubtitleTime val(step);

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
			long step = 1;

			if(ev->state & GDK_SHIFT_MASK && ev->state & GDK_CONTROL_MASK)
				step *= 100;
			else if(ev->state & GDK_CONTROL_MASK)
				step *= 10;

			if(ev->direction == GDK_SCROLL_UP)
				frame += step;
			else if(ev->direction == GDK_SCROLL_DOWN)
				frame -= step;

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

		// display flash message
		if(m_document)
			on_flash_message();

		m_editable->show();

		return m_editable;
	}

	/*
	 * If need to display flash message.
	 */
	virtual void on_flash_message()
	{
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
class CellRendererTextMultiline : public CellRendererCustom<TextViewCell>
{
public:
	CellRendererTextMultiline(Document *doc)
	:CellRendererCustom<TextViewCell>(doc)
	{
		property_editable() = true;
		property_yalign() = 0.0;

		if(Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center"))
		{
			property_xalign() = 0.5;
			property_alignment() = Pango::ALIGN_CENTER;
		}
	}

	/*
	 * Need to display a flash message for the behavior of line-break and exit.
	 */
	void on_flash_message()
	{
		if(Config::getInstance().get_value_bool("subtitle-view", "used-ctrl-enter-to-confirm-change"))
			m_document->flash_message(_("Use Ctrl+Return for exit and Return for line-break"));
		else
			m_document->flash_message(_("Use Return for exit and Ctrl+Return for line-break"));
	}
};

/*
 * SubtitleView Constructor
 */
SubtitleView::SubtitleView(Document &doc)
{
	m_currentColumn = NULL;

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

	Gtk::Widget* widget = column->get_widget();
	if(widget)
		widget->set_tooltip_text(text);
}

/*
 * Return a new column (already manage) with Gtk::Label in title.
 */
Gtk::TreeViewColumn* SubtitleView::create_treeview_column(const Glib::ustring &name)
{
	Glib::ustring title = get_column_label_by_name(name);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn);

	Gtk::Label* label = manage(new Gtk::Label(title, Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, false));
	label->show();
	column->set_widget(*label);

	m_columns[name] = column;

	return column;
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
	
	column = create_treeview_column("number");
	renderer = manage(new Gtk::CellRendererText);
	renderer->property_editable() = false;
	renderer->property_yalign() = 0;
	renderer->property_xalign() = 1.0;
	renderer->property_alignment() = Pango::ALIGN_RIGHT;
	
	column->pack_start(*renderer);
	column->add_attribute(renderer->property_text(), m_column.num);

	append_column(*column);

	set_tooltips(column, _("The line number"));
}

/*
 *
 */
void SubtitleView::createColumnLayer()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = create_treeview_column("layer");
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.layer);

	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_layer));

	append_column(*column);

	//set_tooltips(column, _("Layer number."));
}

/*
 *
 */
void SubtitleView::create_column_time(
		const Glib::ustring &name, 
		const Gtk::TreeModelColumnBase& column_attribute,
		const sigc::slot<void, const Glib::ustring&, const Glib::ustring&> &slot, 
		const Glib::ustring &tooltips)
{
	se_debug_message(SE_DEBUG_VIEW, "name=%s tooltips=%s", 
			name.c_str(), tooltips.c_str());


	CellRendererTime* renderer = manage(new CellRendererTime(m_refDocument));
	
	Gtk::TreeViewColumn* column = create_treeview_column(name);

	column->pack_start(*renderer);
	column->add_attribute(renderer->property_text(), column_attribute);
	
	renderer->signal_edited().connect(slot);

	append_column(*column);

	set_tooltips(column, tooltips);
}

/*
 *
 */
void SubtitleView::createColumnStart()
{
	create_column_time(
			"start", 
			m_column.start, 
			sigc::mem_fun(*this, &SubtitleView::on_edited_start),
			_("When a subtitle appears on the screen."));
}

/*
 *
 */
void SubtitleView::createColumnEnd()
{
	create_column_time(
			"end", 
			m_column.end, 
			sigc::mem_fun(*this, &SubtitleView::on_edited_end),
			_("When a subtitle disappears from the screen."));

}

/*
 *
 */
void SubtitleView::createColumnDuration()
{
	create_column_time(
			"duration", 
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
	
	column = create_treeview_column("style");
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
}

/*
 *
 */
void SubtitleView::createColumnName()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = create_treeview_column("name");

	CellRendererCustom<TextViewCell>* renderer = manage(new CellRendererCustom<TextViewCell>(m_refDocument));

	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.name);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_name));
	
	append_column(*column);
}

/*
 *
 */
void SubtitleView::createColumnCPS()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = create_treeview_column("cps");

	Gtk::CellRendererText* renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer);
	column->add_attribute(renderer->property_text(), m_column.characters_per_second_text);

	renderer->property_yalign() = 0;
	renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
	renderer->property_xalign() = 1.0;
	renderer->property_alignment() = Pango::ALIGN_RIGHT;
	
	append_column(*column);

	set_tooltips(column, _("The number of characters per second"));
}

/*
 *
 */
void SubtitleView::createColumnText()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = create_treeview_column("text");

	append_column(*column);

	// text
	{
		CellRendererTextMultiline* renderer = manage(new CellRendererTextMultiline(m_refDocument));

		column->pack_start(*renderer, true);
		column->add_attribute(renderer->property_text(), m_column.text);
	
		renderer->signal_edited().connect(
				sigc::mem_fun(*this, &SubtitleView::on_edited_text));
	}
	// cpl
	{
		Gtk::CellRendererText* renderer = NULL;
		renderer = manage(new Gtk::CellRendererText);
	
		column->pack_start(*renderer, false);
		column->add_attribute(renderer->property_text(), m_column.characters_per_line_text);
		renderer->property_yalign() = 0;
		renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
		renderer->property_xalign() = 1.0;
		renderer->property_alignment() = Pango::ALIGN_RIGHT;

		bool show=true;
		
		Config::getInstance().get_value_bool("subtitle-view", "show-character-per-line", show);

		renderer->property_visible() = show;
	}

	column->set_resizable(true);
}

/*
 *
 */
void SubtitleView::createColumnTranslation()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = create_treeview_column("translation");

	//translation
	{
		CellRendererTextMultiline* renderer = manage(new CellRendererTextMultiline(m_refDocument));

		column->pack_start(*renderer, true);
		column->add_attribute(renderer->property_text(), m_column.translation);
	
		append_column(*column);

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
		renderer->property_weight() = Pango::WEIGHT_ULTRALIGHT;
		bool show=true;
		Config::getInstance().get_value_bool("subtitle-view", "show-character-per-line", show);

		renderer->property_visible() = show;
	}

	column->set_resizable(true);
}

/*
 *
 */
void SubtitleView::createColumnNote()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = create_treeview_column("note");;

	CellRendererTextMultiline* renderer = manage(new CellRendererTextMultiline(m_refDocument));

	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.note);
	
	append_column(*column);
	
	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_note));

	column->set_resizable(true);
}

/*
 *
 */
void SubtitleView::createColumnEffect()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = create_treeview_column("effect");
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.effect);
	
	append_column(*column);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_effect));

	column->set_resizable(true);
}

/*
 *
 */
void SubtitleView::createColumnMarginR()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = create_treeview_column("margin-r");
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginR);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_r));

	append_column(*column);
}

/*
 *
 */
void SubtitleView::createColumnMarginL()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = create_treeview_column("margin-l");
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginL);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_l));

	append_column(*column);
}

/*
 *
 */
void SubtitleView::createColumnMarginV()
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn* column = NULL;
	Gtk::CellRendererText* renderer = NULL;
	
	column = create_treeview_column("margin-v");
	renderer = manage(new Gtk::CellRendererText);
	
	column->pack_start(*renderer, false);
	column->add_attribute(renderer->property_text(), m_column.marginV);
	
	renderer->property_editable() = true;
	renderer->property_yalign() = 0;

	renderer->signal_edited().connect(
		sigc::mem_fun(*this, &SubtitleView::on_edited_margin_v));

	append_column(*column);
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
void SubtitleView::select_and_set_cursor(const Gtk::TreeIter &iter, bool start_editing)
{
	se_debug(SE_DEBUG_VIEW);

	Gtk::TreeViewColumn *column = NULL;
	Gtk::TreeModel::Path path;
	get_cursor(path, column);

	if(column == NULL)
		column = m_columns["text"];

	get_selection()->select(iter);
	set_cursor(m_subtitleModel->get_path(iter), *column, start_editing);
}




/*
 *
 */
bool SubtitleView::on_key_press_event(GdkEventKey *event)
{
	if(event->string != NULL)
	{
		int num;
		std::istringstream ss(event->string);
		bool is_num = ss >> num != 0; 
		// Update only if it's different
		if(is_num != get_enable_search())
			set_enable_search(is_num);
	}
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
 * This is a static function. 
 * Return the humain label by the internal name of the column.
 */
Glib::ustring SubtitleView::get_column_label_by_name(const Glib::ustring &name)
{
	std::map<Glib::ustring, Glib::ustring> columns_labels;

	columns_labels["cps"] = _("CPS");
	columns_labels["duration"] = _("Duration");
	columns_labels["effect"] = _("Effect");
	columns_labels["end"] = _("End");
	columns_labels["layer"] = _("Layer");
	columns_labels["margin-l"] = _("L");
	columns_labels["margin-r"] = _("R");
	columns_labels["margin-v"] = _("V");
	columns_labels["name"] = _("Name");
	columns_labels["note"] = _("Note");
	columns_labels["number"] = _("Num");
	columns_labels["start"] = _("Start");
	columns_labels["style"] = _("Style");
	columns_labels["text"] = _("Text");
	columns_labels["translation"] = _("Translation");

	std::map<Glib::ustring, Glib::ustring>::iterator it = columns_labels.find(name);
	if(it != columns_labels.end())
		return it->second;

	return Glib::ustring("Invalid : ") + name;
}

/*
 * The position of the cursor (focused cell) has changed.
 * Update the column title (bold).
 */
void SubtitleView::on_cursor_changed()
{
	se_debug(SE_DEBUG_VIEW);

	Pango::AttrList normal;
	Pango::AttrInt att_normal = Pango::Attribute::create_attr_weight(Pango::WEIGHT_NORMAL);
	normal.insert(att_normal);
	
	Pango::AttrList active;
	Pango::AttrInt att_active = Pango::Attribute::create_attr_weight(Pango::WEIGHT_BOLD);
	active.insert(att_active);

	// get the focused column
	Gtk::TreeViewColumn *focused_column = NULL;
	Gtk::TreeModel::Path path;
	get_cursor(path, focused_column);

	// if it's the same, doesn't needs update
	if(m_currentColumn == focused_column)
		return;

	// unbold the old column
	if(m_currentColumn != NULL)
	{
		Gtk::Label *label = dynamic_cast<Gtk::Label*>(m_currentColumn->get_widget());
		label->set_attributes(normal);

		m_currentColumn = NULL;
	}
	// bold the new current column
	if(focused_column)
	{
		Gtk::Label *label = dynamic_cast<Gtk::Label*>(focused_column->get_widget());
		label->set_attributes(active);

		m_currentColumn = focused_column;
	}
}
