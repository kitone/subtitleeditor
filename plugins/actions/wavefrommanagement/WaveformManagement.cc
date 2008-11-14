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
#include <utility.h>
#include <extension/Action.h>
#include <Player.h>
#include <WaveformManager.h>
#include <gui/DialogFileChooser.h>

/*
 *
 */
class WaveformManagement : public Action
{
public:

	/*
	 *
	 */
	WaveformManagement()
	{
		activate();
		update_ui();
	}

	~WaveformManagement()
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
		action_group = Gtk::ActionGroup::create("WaveformManagement");

		// Already create in MenuBar.cc
		/*
		action_group->add(
				Gtk::Action::create("menu-waveform", _("_Waveform")));
		*/

		// open & save
		action_group->add(
				Gtk::Action::create("waveform/open", Gtk::Stock::OPEN, _("_Open Waveform"), _("Open wavefrom from a file or create from a video")), Gtk::AccelKey("<Control><Alt>O"),
					sigc::mem_fun(*this, &WaveformManagement::on_open_waveform));

		action_group->add(
				Gtk::Action::create("waveform/save", Gtk::Stock::SAVE, _("_Save Waveform"), _("Save wavefrom to file")), Gtk::AccelKey("<Control><Alt>S"),
					sigc::mem_fun(*this, &WaveformManagement::on_save_waveform));

		// zoom
		action_group->add(
				Gtk::Action::create("waveform/zoom-in", Gtk::Stock::ZOOM_IN, _("Zoom _In"), _("FIXME")),
					sigc::mem_fun(*this, &WaveformManagement::on_zoom_in));

		action_group->add(
				Gtk::Action::create("waveform/zoom-out", Gtk::Stock::ZOOM_OUT, _("Zoom _Out"), _("FIXME")),
					sigc::mem_fun(*this, &WaveformManagement::on_zoom_out));

		action_group->add(
				Gtk::Action::create("waveform/zoom-selection", Gtk::Stock::ZOOM_FIT, _("Zoom _Selection"), _("FIXME")),
					sigc::mem_fun(*this, &WaveformManagement::on_zoom_selection));

		action_group->add(
				Gtk::Action::create("waveform/zoom-all", Gtk::Stock::ZOOM_100, _("Zoom _All"), _("FIXME")),
					sigc::mem_fun(*this, &WaveformManagement::on_zoom_all));

		// center
		action_group->add(
				Gtk::Action::create("waveform/center-with-selected-subtitle", _("_Center With Selected Subtitle"), _("FIXME")),
					sigc::mem_fun(*this, &WaveformManagement::on_center_with_selected_subtitle));

		// scrolling with player
		bool scroll_with_player_state = get_config().get_value_bool("waveform", "scrolling-with-player");

		action_group->add(
				Gtk::ToggleAction::create("waveform/scrolling-with-player", _("Scrolling With _Player"), _("FIXME"), scroll_with_player_state),
					sigc::mem_fun(*this, &WaveformManagement::on_scrolling_with_player));

		// scrolling with selection
		bool scroll_with_selection_state = get_config().get_value_bool("waveform", "scrolling-with-selection");

		action_group->add(
				Gtk::ToggleAction::create("waveform/scrolling-with-selection", _("Scrolling With _Selection"), _("FIXME"), scroll_with_selection_state),
					sigc::mem_fun(*this, &WaveformManagement::on_scrolling_with_selection));

		// Respect the timing
		bool respect_timing_state = get_config().get_value_bool("waveform", "respect-timing");

		action_group->add(
				Gtk::ToggleAction::create("waveform/respect-timing", _("_Respect The Timing"), _("Try to respect the timing preferences"), respect_timing_state),
					sigc::mem_fun(*this, &WaveformManagement::on_respect_timing));

		// Waveform Display
		bool waveform_display_state = get_config().get_value_bool("waveform", "display");

		action_group->add(
				Gtk::ToggleAction::create("waveform/display", _("_Wavform"), _("Show or hide the waveform in the current window"), waveform_display_state),
					sigc::mem_fun(*this, &WaveformManagement::on_waveform_display));

		// ui
		Glib::RefPtr<Gtk::UIManager> ui = get_ui_manager();

		ui->insert_action_group(action_group);

		Glib::ustring submenu = 
			"<ui>"
			"	<menubar name='menubar'>"
			"		<menu name='menu-waveform' action='menu-waveform'>"
			"			<placeholder name='waveform-management'>"
			"				<menuitem action='waveform/open'/>"
			"				<menuitem action='waveform/save'/>"
			"				<separator/>"
			"				<menuitem action='waveform/zoom-in'/>"
			"				<menuitem action='waveform/zoom-out'/>"
			"				<menuitem action='waveform/zoom-selection'/>"
			"				<menuitem action='waveform/zoom-all'/>"
			"				<separator/>"
			"				<menuitem action='waveform/center-with-selected-subtitle'/>"
			"				<separator/>"
			"				<menuitem action='waveform/scrolling-with-player'/>"
			"				<menuitem action='waveform/scrolling-with-selection'/>"
			"				<menuitem action='waveform/respect-timing'/>"
			"			</placeholder>"
			"		</menu>"
			"	</menubar>"
			"</ui>";

		ui_id = ui->add_ui_from_string(submenu);

		// Show/Hide Waveform Editor
		ui->add_ui(ui_id, "/menubar/menu-view/display-placeholder",
				"waveform/display", "waveform/display");
		
		// HACK
		WaveformManager* wm = get_waveform_manager();
		
		wm->signal_waveform_changed().connect(
				sigc::mem_fun(*this, &WaveformManagement::update_ui));

		wm->signal_waveform_changed().connect(
				sigc::mem_fun(*this, &WaveformManagement::on_waveform_changed));

		get_config().signal_changed("waveform").connect(
				sigc::mem_fun(*this, &WaveformManagement::on_config_waveform_changed));
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

		bool has_waveform = get_waveform_manager()->has_waveform();

		bool has_document = (get_current_document() != NULL);

		action_group->get_action("waveform/save")->set_sensitive(has_waveform);
		action_group->get_action("waveform/zoom-in")->set_sensitive(has_waveform);
		action_group->get_action("waveform/zoom-out")->set_sensitive(has_waveform);
		action_group->get_action("waveform/zoom-selection")->set_sensitive(has_waveform);
		action_group->get_action("waveform/zoom-all")->set_sensitive(has_waveform);

		action_group->get_action("waveform/scrolling-with-player")->set_sensitive(has_waveform);
		action_group->get_action("waveform/scrolling-with-selection")->set_sensitive(has_waveform);
		action_group->get_action("waveform/respect-timing")->set_sensitive(has_waveform);

		action_group->get_action("waveform/center-with-selected-subtitle")->set_sensitive(has_waveform && has_document);
	}


protected:

	/*
	 *
	 */
	WaveformManager* get_waveform_manager()
	{
		return get_subtitleeditor_window()->get_waveform_manager();
	}

	/*
	 * Launch the Dialog Open Waveform
	 * and try to open the Waveform.
	 * If is not a Waveform file launch the
	 * Waveform generator.
	 */
	void on_open_waveform()
	{
		se_debug(SE_DEBUG_PLUGINS);

		DialogOpenWaveform dialog;
		if(dialog.run() == Gtk::RESPONSE_OK)
		{
			dialog.hide();

			Glib::ustring uri = dialog.get_uri();
		
			WaveformManager* wm = get_waveform_manager();
			if(wm->open_waveform(uri) == false)
			{
				// try to create the Waveform from media
				wm->generate_waveform(uri);
			}
		}
	}

	/*
	 *
	 */
	void on_save_waveform()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Waveform> wf = get_waveform_manager()->get_waveform();
		if(wf)
		{
			Gtk::FileChooserDialog ui(_("Save Waveform"), Gtk::FILE_CHOOSER_ACTION_SAVE);
			ui.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			ui.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
			ui.set_default_response(Gtk::RESPONSE_OK);

			if(ui.run() == Gtk::RESPONSE_OK)
			{
				Glib::ustring uri = ui.get_uri();

				wf->save(uri);
			}
		}
	}

	/*
	 * Update the video player with the new Waveform.
	 */
	void on_waveform_changed()
	{
		Glib::RefPtr<Waveform> wf = get_waveform_manager()->get_waveform();
		if(wf)
		{
			get_subtitleeditor_window()->get_player()->open(wf->m_video_uri);
		}
	}

	/*
	 *
	 */
	void on_center_with_selected_subtitle()
	{
		se_debug(SE_DEBUG_PLUGINS);

		get_waveform_manager()->center_with_selected_subtitle();
	}

	/*
	 *
	 */
	void on_zoom_in()
	{
		se_debug(SE_DEBUG_PLUGINS);

		get_waveform_manager()->zoom_in();
	}

	/*
	 *
	 */
	void on_zoom_out()
	{
		se_debug(SE_DEBUG_PLUGINS);

		get_waveform_manager()->zoom_out();
	}

	/*
	 *
	 */
	void on_zoom_selection()
	{
		se_debug(SE_DEBUG_PLUGINS);

		get_waveform_manager()->zoom_selection();
	}

	/*
	 *
	 */
	void on_zoom_all()
	{
		se_debug(SE_DEBUG_PLUGINS);

		get_waveform_manager()->zoom_all();
	}

	/*
	 *
	 */
	void on_scrolling_with_player()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("waveform/scrolling-with-player"));
		if(action)
		{
			bool state = action->get_active();
			get_config().set_value_bool("waveform", "scrolling-with-player", state);
		}
	}

	/*
	 *
	 */
	void on_scrolling_with_selection()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("waveform/scrolling-with-selection"));
		if(action)
		{
			bool state = action->get_active();
			get_config().set_value_bool("waveform", "scrolling-with-selection", state);
		}
	}

	/*
	 *
	 */
	void on_respect_timing()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("waveform/respect-timing"));
		if(action)
		{
			bool state = action->get_active();
			get_config().set_value_bool("waveform", "respect-timing", state);
		}
	}

	/*
	 *
	 */
	void on_waveform_display()
	{
		se_debug(SE_DEBUG_PLUGINS);

		Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("waveform/display"));
		if(action)
		{
			bool state = action->get_active();
			if(get_config().get_value_bool("waveform", "display") != state)
				get_config().set_value_bool("waveform", "display", state);
		}
	}

	/*
	 *
	 */
	void on_config_waveform_changed(const Glib::ustring &key, const Glib::ustring &value)
	{
		if(key == "display")
		{
			bool state = utility::string_to_bool(value);
			
			Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(action_group->get_action("waveform/display"));
			if(action)
			{
				if(action->get_active() != state)
					action->set_active(state);
			}
		}
	}

protected:
	Gtk::UIManager::ui_merge_id ui_id;
	Glib::RefPtr<Gtk::ActionGroup> action_group;
};


REGISTER_EXTENSION(WaveformManagement)
