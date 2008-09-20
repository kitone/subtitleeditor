#ifndef _MaxCharactersPerLine_h
#define _MaxCharactersPerLine_h

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
class MaxCharactersPerLine : public ErrorChecking
{
public:

	MaxCharactersPerLine()
	:ErrorChecking(
			"max-characters-per-line",
			_("Max Characters Per Line"),
			_("An error is detected if a line is too long."))
	{
		m_maxCPL = 40;
	}

	/*
	 *
	 */
	virtual void init()
	{
		m_maxCPL = Config::getInstance().get_value_int("timing", "max-characters-per-line");
	}

	/*
	 *
	 */
	virtual bool execute(Info &info)
	{
		std::istringstream iss(info.currentSub.get_characters_per_line_text());
		std::string line;

		while(std::getline(iss, line))
		{
			int number = utility::string_to_int(line);

			if(number > m_maxCPL)
			{
				if(info.tryToFix)
				{
					// not implemented
					return false;
				}
				
				info.error = build_message(ngettext(
						"Subtitle has a too long line: <b>1 character</b>",
						"Subtitle has a too long line: <b>%i characters</b>", number), number);
				info.solution = _("<b>Automatic correction:</b> unavailable, correct the error manually.");
				
				return true;
			}
		}

		return false;
	}

protected:
	int m_maxCPL;
};

#endif//_MaxCharactersPerLine_h
