#ifndef _TimeUtility_h
#define _TimeUtility_h

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

#include <glibmm/ustring.h>

/*
 *
 */
enum TIMING_MODE
{
	TIME,
	FRAME
};

/*
 *
 */
enum FRAMERATE
{
	FRAMERATE_23_976,
	FRAMERATE_24,
	FRAMERATE_25,    // PAL
	FRAMERATE_29_97, // NTSC
	FRAMERATE_30
};

/*
 * Return the label of the framerate.
 */
Glib::ustring get_framerate_label(FRAMERATE framerate);

/*
 * Return the real value of the framerate.
 */
float get_framerate_value(FRAMERATE framerate);

/*
 * Return the framerate from the value.
 */
FRAMERATE get_framerate_from_value(float value);


#endif//_TimeUility_h
