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

#include <gtkmm.h>
#include <utility.h>
#include <extension/action.h>
#include <player.h>
#include <waveformmanager.h>
#include <gui/dialogfilechooser.h>

/*
 * Declared in waveformgenerator.cc
 */
Glib::RefPtr<Waveform> generate_waveform_from_file(const Glib::ustring &uri);

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
		update_ui_from_player(Player::NONE);
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
				Gtk::Action::create("waveform/open", Gtk::Stock::OPEN, _("_Open Waveform From File"), _("Open wavefrom from a file or create from a video")), Gtk::AccelKey("<Control><Alt>O"),
					sigc::mem_fun(*this, &WaveformManagement::on_open_waveform));

		action_group->add(
				Gtk::Action::create("waveform/generate-from-player-file", _("_Generate Waveform From Video"), 
					_("Generate the waveform from the current video file")), 
					sigc::mem_fun(*this, &WaveformManagement::on_generate_from_player_file));

		action_group->add(
				Gtk::Action::create("waveform/generate-dummy", _("_Generate Dummy Waveform"), 
					_("Generate an dummy waveform (sine)")), 
					sigc::mem_fun(*this, &WaveformManagement::on_generate_dummy));

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
			"				<menuitem action='waveform/generate-from-player-file'/>"
			"				<menuitem action='waveform/generate-dummy'/>"
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

		// Player state
		get_subtitleeditor_window()->get_player()->signal_state_changed().connect(
				sigc::mem_fun(*this, &WaveformManagement::update_ui_from_player));
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

	/*
	 * Update the ui state from the player state.
	 */
	void update_ui_from_player(Player::State state)
	{
		bool has_player_file = (state != Player::NONE);

		action_group->get_action("waveform/generate-from-player-file")->set_sensitive(has_player_file);
		action_group->get_action("waveform/generate-dummy")->set_sensitive(has_player_file);
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
			Glib::RefPtr<Waveform> wf = Waveform::create_from_file(uri);
			if(!wf)
				wf = generate_waveform_from_file(uri);

			if(wf)
				get_waveform_manager()->set_waveform(wf);
		}
	}

	/*
	 * Generate a waveform from the current file in the player.
	 */
	void on_generate_from_player_file()
	{
		Glib::ustring uri = get_subtitleeditor_window()->get_player()->get_uri();
		if(uri.empty() == false)
		{
			//get_waveform_manager()->generate_waveform(uri);
			Glib::RefPtr<Waveform> wf = generate_waveform_from_file(uri);
			if(wf)
				get_waveform_manager()->set_waveform(wf);
		}
	}

	/*
	 * Generate an Sine Waveform
	 */
	void on_generate_dummy()
	{
		Player* player = get_subtitleeditor_window()->get_player();
		if(player->get_state() == Player::NONE)
			return;

		// Create and initialize Waveform
		Glib::RefPtr<Waveform> wf(new Waveform);
		wf->m_video_uri = player->get_uri();		
		wf->m_n_channels = 1;
		wf->m_duration = player->get_duration();
		
		// Create Sine Waveform
		int second = SubtitleTime(0,0,1,0).totalmsecs;
		wf->m_channels[0].resize(wf->m_duration);
		
		double freq = (wf->m_duration % second) / 2;
		double amp = 0.5;
		double rate = SubtitleTime(0,1,0,0).totalmsecs;
		double rfreq = 2.0 * 3.141592653589793 * freq;

		for(unsigned int i=1; i<= wf->m_duration; ++i)
		{
			double a = amp - (amp * (i % second) * 0.001);
			wf->m_channels[0][i-1] = a * sin(rfreq * (i/rate));
		}

		get_waveform_manager()->set_waveform(wf);
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
	 * Update the video player with the new Waveform
	 * only if it's different.
	 */
	void on_waveform_changed()
	{
		Glib::RefPtr<Waveform> wf = get_waveform_manager()->get_waveform();
		if(wf && get_subtitleeditor_window()->get_player()->get_uri() != wf->m_video_uri)
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
