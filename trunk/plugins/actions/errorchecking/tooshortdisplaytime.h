#ifndef _TooShortDisplayTime_h
#define _TooShortDisplayTime_h

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

#include "errorchecking.h"
#include <i18n.h>

/*
 *
 */
class TooShortDisplayTime : public ErrorChecking
{
public:

	TooShortDisplayTime()
	:ErrorChecking(
			"too-short-display-time",
			_("Too Short Display Time"),
			_("Detects and fixes subtitles when the number of characters per second is superior to the specified value."))
	{
		m_maxCPS = 25;
	}
	
	/*
	 *
	 */
	virtual void init()
	{
		m_maxCPS = Config::getInstance().get_value_int("timing", "max-characters-per-second");
	}

	/*
	 *
	 */
	bool execute(Info &info)	
	{
		Glib::ustring text = info.currentSub.get_text();
		long duration = info.currentSub.get_duration().totalmsecs;

		double cps = utility::get_characters_per_second(text, duration);
		cps = floor( ( cps + 0.05 )*10 ) / 10;	//to make CPS errors consistent with those in the subtitle view CPS column 

		if(cps <= m_maxCPS || m_maxCPS == 0)
			return false;

		SubtitleTime value((long)(text.size() * 1000 / m_maxCPS));

		SubtitleTime new_end = info.currentSub.get_start() + value;

		if(info.tryToFix)
		{
			info.currentSub.set_duration(value);
			return true;
		}

		info.error = build_message(
				_("Subtitle display time is too short: <b>%.1f chars/s</b>"), cps);

		info.solution = build_message(
				_("<b>Automatic correction:</b> to change current subtitle end to %s."),
				new_end.str().c_str());

		return true;
	}

protected:
	int m_maxCPS;
};

#endif//_TooShortDisplayTime_h
