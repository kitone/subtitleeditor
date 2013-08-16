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

#include "comboboxvideo.h"
#include "cfg.h"
#include "i18n.h"

/*
 * Constructor
 */
ComboBoxVideo::ComboBoxVideo(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /*builder*/)
:Gtk::ComboBoxText(cobject)
{
	// separator function
	Gtk::ComboBoxText::set_row_separator_func(
		sigc::mem_fun(*this, &ComboBoxVideo::on_row_separator_func));
}

/*
 * Search all videos in the folder.
 */
bool ComboBoxVideo::set_current_folder(const Glib::ustring &path)
{
	clear();

	Glib::Dir dir(path);
	std::vector<std::string> files(dir.begin(), dir.end());

	if(files.empty())
	{
		set_sensitive(false);
		return false;
	}

	Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(
			"^(.*)\\.((avi)|(wma)|(mkv)|(mpg)|(mpeg)|(ogg)|(mov)|(mp4)|(xvid))$");
		
	for(unsigned int i=0; i<files.size(); ++i)
	{
		if(re->match(files[i]))
			append_text(files[i]);
	}

	if(get_model()->children().size() == 0)
	{
		set_sensitive(false);
		return false;
	}
	
	set_sensitive(true);

	prepend_text("<separator>");
	prepend_text(_("None"));

	return true;
}

/*
 * Try to select the good video from the subtitle.
 * Only if the option "automatically-open-video" is enabled.
 */
bool ComboBoxVideo::auto_select_video(const Glib::ustring &subtitle)
{
	if(get_model()->children().empty())
		return false;

	if(Config::getInstance().get_value_bool("video-player", "automatically-open-video") == false)
	{
		set_active(0);
		return false;
	}

	Glib::RefPtr<Glib::Regex> re = Glib::Regex::create(
			"^(.*)\\.((avi)|(wma)|(mkv)|(mpg)|(mpeg)|(ogg)|(mov)|(mp4)|(xvid))$");

	Gtk::TreeIter it = get_model()->children().begin();

	for(; it; ++it)
	{
		Glib::ustring video = (*it)[m_text_columns.m_column];

		std::vector<Glib::ustring> group = re->split(video);

		if(group.size() == 1)
			continue;

		if(subtitle.find(group[1]) != Glib::ustring::npos)
		{
			set_active_text(video);
			return true;
		}
	}

	// not found, active "None"
	set_active(0);

	return false;
}

/*
 * Return the video selected or a empty string.
 */
Glib::ustring ComboBoxVideo::get_value() const
{
	if(is_sensitive() == false || get_active_row_number() == 0)
		return Glib::ustring();

	return get_active_text();
}

/*
 * Used to define the separator.
 * label = "<separator>"
 */
bool ComboBoxVideo::on_row_separator_func(const Glib::RefPtr<Gtk::TreeModel> &/*model*/, const Gtk::TreeModel::iterator &it)
{
	Glib::ustring text = (*it)[m_text_columns.m_column];
	if(text == "<separator>")
		return true;
	return false;
}

