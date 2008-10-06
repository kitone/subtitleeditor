#ifndef _debug_h
#define _debug_h

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
 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <glib.h>

enum SE_DEBUG_MESSAGE_FLAG
{
	SE_NO_DEBUG							= 0,
	SE_DEBUG_INFORMATION		= 1 << 0, //simple message ex: "subtitleeditor start, ..."
	
	SE_DEBUG_APP						= 1 << 1,
	SE_DEBUG_VIEW						= 1 << 2,
	SE_DEBUG_IO							= 1 << 3,
	SE_DEBUG_SEARCH					= 1 << 4,
	SE_DEBUG_REGEX					= 1 << 5,
	SE_DEBUG_VIDEO_PLAYER		= 1 << 6,
	SE_DEBUG_SPELL_CHECKING	= 1 << 7,
	SE_DEBUG_WAVEFORM				= 1 << 8,
	SE_DEBUG_UTILITY				= 1 << 9,
	SE_DEBUG_COMMAND				= 1 << 10,
	SE_DEBUG_PLUGINS				= 1 << 11,
	SE_DEBUG_PROFILING			= 1 << 12,

	SE_DEBUG_ALL						= 1 << 20
};



/*
 *
 */
void se_debug_init(int flags);

/*
 *	simple teste avec le flags
 */
bool se_debug_check_flags(int flags);

/*
 *
 */
void __se_debug( int flag, 
		const gchar* file, 
		gint line, 
		const gchar* fonction);

/*
 *
 */
void __se_debug_message( int flag, 
		const gchar* file, 
		gint line, 
		const gchar* fonction, 
		const char *string, ...);


#ifdef DEBUG

	/*
	 *
	 */
	#define se_debug(flag) if(se_debug_check_flags(flag)) { __se_debug(flag, __FILE__, __LINE__, __FUNCTION__); }

	/*
	 *
	 */
	#define se_debug_message(flag, ...) if(se_debug_check_flags(flag)) { __se_debug_message(flag, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); }

#else//DEBUG
	#define se_debug(flag)
	#define se_debug_message(flag, ...)
#endif//DEBUG

#endif//_debug_h
