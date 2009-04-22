#ifndef _VideoPlayer_h
#define _VideoPlayer_h

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
#include "player.h"


/*
 *
 */
class VideoPlayer : public Gtk::VBox
{
public:

	/*
	 * Constructor
	 *
	 * Create the GStreamer Player, the PlayerControls (play/pause + seek)
	 */
	VideoPlayer(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

	/*
	 * Destructor
	 */
	~VideoPlayer();

	/*
	 * Load the video player config.
	 */
	void load_config();

	/*
	 * Return the gstreamer player.
	 */
	Player* player();

	/*
	 * The player state has changed. 
	 * Clear subtitle.
	 */
	void on_player_state_changed(Player::State state);

	/*
	 * The config of video player has changed.
	 */
	void on_config_video_player_changed(const Glib::ustring &key, const Glib::ustring &value);

	/*
	 * The current document has changed.
	 * Clear subtitle (sub and player text) and try to found the good subtitle.
	 */
	void on_current_document_changed(Document *doc);

	/*
	 * Check or search the good subtitle (find_subtitle).
	 */
	void on_timeout();

	/*
	 * Initialize the current subtitle and the player text to NULL.
	 */
	void clear_subtitle();

	/*
	 * Check if time is in subtitle.
	 */
	bool is_good_subtitle(const Subtitle &sub, long time);

	/*
	 * Try to found the good subtitle and init the player (text).
	 */
	bool find_subtitle();

	/*
	 * Sets the text of the player to NULL.
	 */
	void show_subtitle_null();

	/*
	 * Sets the text of the player with the current subtitle.
	 */
	void show_subtitle_text();

protected:
	sigc::connection m_connection_document_changed;
	Subtitle m_subtitle;
	Player* m_player;

	bool m_cfg_display_translated_subtitle;
};


#endif//_VideoPlayer_h

