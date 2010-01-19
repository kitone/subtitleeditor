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

#include <cfg.h>
#include <debug.h>
#include "textviewcell.h"
#include "automaticspellchecker.h"

/*
 * Contructor
 * Initialize the widget with the AutomaticSpellChecker.
 */
TextViewCell::TextViewCell()
:	Glib::ObjectBase(typeid(TextViewCell)),
	Gtk::CellEditable(), 
	m_editing_canceled(false), 
	m_used_ctrl_enter_to_confirm_change(false)
{
	se_debug(SE_DEBUG_VIEW);

	m_used_ctrl_enter_to_confirm_change = Config::getInstance().get_value_bool("subtitle-view", "used-ctrl-enter-to-confirm-change");

	if(Config::getInstance().get_value_bool("subtitle-view", "property-alignment-center"))
		set_justification(Gtk::JUSTIFY_CENTER);

	set_wrap_mode(Gtk::WRAP_NONE);

	AutomaticSpellChecker::create_from_textview(this);
}

/*
 * Destructor
 */
TextViewCell::~TextViewCell()
{
	se_debug(SE_DEBUG_VIEW);
}

/*
 * Define the current text.
 */
void TextViewCell::set_text(const Glib::ustring &text)
{
	se_debug_message(SE_DEBUG_VIEW, "text=<%s>", text.c_str());
	get_buffer()->set_text(text);
}

/*
 * Return the current text.
 */
Glib::ustring TextViewCell::get_text() 
{
	se_debug(SE_DEBUG_VIEW);

	Glib::RefPtr<Gtk::TextBuffer> buffer = get_buffer();

	Gtk::TextBuffer::iterator start, end;

	buffer->get_bounds(start,end);
	return buffer->get_text(start,end);
}

/*
 * Check if the user cancel the editing with Escape.
 * Check if the user apply the editing with Enter (depend on user prefs).
 */
bool TextViewCell::on_key_press_event(GdkEventKey* event)
{
	se_debug(SE_DEBUG_VIEW);

	if(event->keyval == GDK_Escape)
	{
		m_editing_canceled = true;
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
		// We don't need to call editing_done(), there's a call in on_remove_widget()
		//editing_done();
		remove_widget();
		return true;
	}
	return Gtk::TextView::on_key_press_event(event);
}

/*
 * Before removing the widget we call editing_done 
 * if there's no canceled signal.
 */
void TextViewCell::on_remove_widget()
{
	se_debug(SE_DEBUG_VIEW);
	// We apply the editing if there's not a canceled signal. 
	if(m_editing_canceled == false)
		editing_done();
	Gtk::CellEditable::on_remove_widget();
}
