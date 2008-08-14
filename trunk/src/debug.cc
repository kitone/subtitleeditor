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
 

#include "debug.h"
#include <string>
#include <iostream>
#include <glibmm/timer.h>

/*
 *
 */
static int debug_flags = SE_NO_DEBUG;

/*
 * PROFILING
 */
//#define ENABLE_PROFILING

#ifdef ENABLE_PROFILING
static Glib::Timer timer;
static double timer_last = 0.0;
#endif//ENABLE_PROFILING

/*
 *
 */
void se_debug_init(int flags)
{
	debug_flags = flags;

#ifdef ENABLE_PROFILING
	if(debug_flags != SE_NO_DEBUG)
		timer.start();
#endif//ENABLE_PROFILING
}

/*
 *	simple teste avec le flags
 */
bool se_debug_check_flags(int flag)
{
	if(G_UNLIKELY(debug_flags & SE_DEBUG_ALL))
		return true;

	return G_UNLIKELY(debug_flags & flag);
}

/*
 *
 */
void __se_debug(
		int flag, 
		const gchar* file, 
		const gint line, 
		const gchar* fonction)
{
	if(G_UNLIKELY(debug_flags & flag) || G_UNLIKELY(debug_flags & SE_DEBUG_ALL))
	{
#ifdef ENABLE_PROFILING
		double seconds = 0.0;

		seconds = timer.elapsed();

		g_print("[%f (%f)] %s:%d (%s)\n", seconds, seconds - timer_last, file, line, fonction);
		timer_last = seconds;
#else
		g_print("%s:%d (%s)\n", file, line, fonction);
#endif//ENABLE_PROFILING

		fflush(stdout);
	}
}

/*
 *
 */
void __se_debug_message(
		int flag, 
		const gchar* file, 
		gint line, 
		const gchar* fonction, 
		const char *format, ...)
{
	if(G_UNLIKELY(debug_flags & flag) || G_UNLIKELY(debug_flags & SE_DEBUG_ALL))
	{
		va_list args;
		gchar *msg = NULL;

		g_return_if_fail(format);

		va_start(args, format);
		msg = g_strdup_vprintf(format, args);
		va_end(args);

#ifdef ENABLE_PROFILING
		double seconds = 0.0;

		seconds = timer.elapsed();

		g_print("[%f (%f)] %s:%d (%s) %s\n", seconds, seconds - timer_last, file, line, fonction, msg);
		timer_last = seconds;
#else
		g_print("%s:%d (%s) %s\n", file, line, fonction, msg);
#endif//ENABLE_PROFILING

		fflush(stdout);

		g_free(msg);
	}
}

