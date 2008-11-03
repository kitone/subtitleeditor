#ifndef _TooLongDisplayTime_h
#define _TooLongDisplayTime_h

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

#include "ErrorChecking.h"

/*
 *
 */
class TooLongDisplayTime : public ErrorChecking
{
public:

	TooLongDisplayTime()
	:ErrorChecking(
			"too-long-display-time",
			_("Too Long Display Time"),
			_("Detects and fixes subtitles when the number of characters per second is inferior to the specified value."))
	{
		m_minCPS = 5;
	}
	
	/*
	 *
	 */
	virtual void init()
	{
		m_minCPS = Config::getInstance().get_value_int("timing", "min-characters-per-second");
	}

	/*
	 *
	 */
	bool execute(Info &info)	
	{
		Glib::ustring text = info.currentSub.get_text();
		long duration = info.currentSub.get_duration().totalmsecs;
		
		double cps = utility::get_characters_per_second(text, duration);

		if(cps >= m_minCPS || m_minCPS == 0)
			return false;

		SubtitleTime value( (long)(text.size() * 1000 / m_minCPS) );
		
		SubtitleTime new_end = info.currentSub.get_start() + value;

		if(info.tryToFix)
		{
			info.currentSub.set_duration(value);
			return true;
		}
		
		info.error = build_message(ngettext(
				"Subtitle display time is too long: <b>%.1f char/s</b>",
				"Subtitle display time is too long: <b>%.1f chars/s</b>", cps), cps);
		
		info.solution = build_message(
				_("<b>Automatic correction:</b> to change current subtitle end to %s."),
				new_end.str().c_str());

		return true;
	}

protected:
	int m_minCPS;
};

#endif//_TooLongDisplayTime_h
