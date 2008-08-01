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
#include "GtkUtility.h"


/*
 *
 */
class DialogMoveSubtitles : public Gtk::Dialog
{
public:
	DialogMoveSubtitles(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	:Gtk::Dialog(cobject)
	{
		refGlade->get_widget("label-start-value", m_labelStartValue);
		refGlade->get_widget_derived("spin-start-value", m_spinStartValue);
		refGlade->get_widget_derived("spin-new-start", m_spinNewStart);
	}

	/*
	 *
	 */
	void init(Document *doc, const Subtitle &subtitle)
	{
		TIMING_MODE edit_mode = doc->get_edit_timing_mode();

		m_labelStartValue->set_label((edit_mode == TIME) ? _("_Start Time:") : _("_Start Frame:"));

		m_spinStartValue->set_timing_mode(edit_mode);
		m_spinNewStart->set_timing_mode(edit_mode);

		long value = (edit_mode == TIME) ? subtitle.get_start().totalmsecs : subtitle.get_start_frame();

		m_spinStartValue->set_value(value);
		m_spinStartValue->set_range(value, value);

		m_spinNewStart->set_value(value);
		m_spinNewStart->grab_focus();
	}

	/*
	 *
	 */
	long get_diff_value()
	{
		return (long)(m_spinNewStart->get_value() - m_spinStartValue->get_value());
	}

protected:
	Gtk::Label*			m_labelStartValue;
	SpinButtonTime*	m_spinStartValue;
	SpinButtonTime*	m_spinNewStart;
};

/*
 *
 */
class MoveSubtitlesPlugin : public Plugin
{
public:

	/*
	 *
	 */
	void activate()
	{
		se_debug(SE_DEBUG_PLUGINS);

		// actions
		action_group = Gtk::ActionGroup::create("MoveSubtitlesPlugin");

		action_group->add(
				Gtk::Action::create("move-subtitles", Gtk::Stock::JUMP_TO, _("_Move Subtitles"), _("All subtitles will be also moved after the first selected subtitle")), Gtk::AccelKey("<Control>M"),
					sigc::mem_fun(*this, &MoveSubtitlesPlugin::on_move_subtitles));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui_id = ui->new_merge_id();

		ui->insert_action_group(action_group);

		//ui->add_ui(ui_id, "/menubar/menu-timings/extend-1", "move-subtitles", "move-subtitles");
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

		action_group->get_action("move-subtitles")->set_sensitive(visible);
	}

protected:

	/*
	 *
	 */
	void on_move_subtitles()
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

		// create dialog
		DialogMoveSubtitles *dialog = utility::get_widget_derived<DialogMoveSubtitles>("dialog-move-subtitles.glade", "dialog-move-subtitles");

		g_return_val_if_fail(dialog, false);

		Subtitle first_selected_subtitle = doc->subtitles().get_first_selected();

		if(first_selected_subtitle)
		{
			dialog->init(doc, first_selected_subtitle);

			if(dialog->run() == Gtk::RESPONSE_OK)
			{
				long diff = dialog->get_diff_value();

				if(diff != 0)
				{
					doc->start_command(_("Move Subtitles"));
				
					move_first_selected_subtitle_and_next(doc, diff);

					doc->emit_signal("subtitle-time-changed");
					doc->finish_command();
				}
			}
		}
		else
		{
			doc->flash_message(_("Please select at least a subtitle."));
		}

		delete dialog;

		return true;
	}
	
	/*
	 * on n'utilise seulement le premier s-t de la selection
	 * ensuite on utilise on applique les modifications au suivant
	 * sans prendre en compte la selection
	 */
	bool move_first_selected_subtitle_and_next(Document *doc, const long &diff)
	{
		se_debug(SE_DEBUG_PLUGINS);

		std::vector<Subtitle> selection = doc->subtitles().get_selection();

		if(selection.empty())
			return false;

		if(doc->get_edit_timing_mode() == TIME)
		{
			SubtitleTime time(diff);

			for(Subtitle sub = selection[0]; sub; ++sub)
			{
				sub.set_start_and_end(
						sub.get_start() + time,
						sub.get_end() + time);
			}
		}
		else
		{
			for(Subtitle sub = selection[0]; sub; ++sub)
			{
				sub.set_start_frame( sub.get_start_frame() + diff);
				sub.set_end_frame( sub.get_end_frame() + diff);
			}
		}

		return true;
	}
protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};

REGISTER_PLUGIN(MoveSubtitlesPlugin)
