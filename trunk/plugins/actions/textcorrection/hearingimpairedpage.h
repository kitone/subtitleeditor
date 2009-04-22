#ifndef _HearingImpairedPage_h
#define _HearingImpairedPage_h

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

#include "patternspage.h"

class HearingImpairedPage : public PatternsPage
{
public:
	HearingImpairedPage()
	:PatternsPage(
			"hearing-impaired",
			_("Select Hearing Impaired Patterns"),
			_("Remove hearing impaired texts"), 
			_("Remove explanatory texts meant for the hearing impaired"))
	{
	}
};

#endif//_HearingImpairedPage_h
