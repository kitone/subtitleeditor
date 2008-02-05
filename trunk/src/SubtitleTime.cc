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
 
#include "SubtitleTime.h"
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "utility.h"


/**
 *
**/
SubtitleTime::SubtitleTime()
:hours(0), mins(0), secs(0), msecs(0), totalmsecs(0)
{

}

/*
 *
 */
SubtitleTime::SubtitleTime(const long &total)
:hours(0), mins(0), secs(0), msecs(0), totalmsecs(0)
{
	ldiv_t divres;
	
	divres = ldiv(total, 1000);
	long hms = divres.quot;
	msecs = divres.rem;
	
	divres = ldiv(hms, 60);
	long hm = divres.quot;
	secs = divres.rem;

	divres = ldiv(hm, 60);
	long h = divres.quot;
	mins = divres.rem;
	
	hours = (int)h;

	totalmsecs = getMSecs(*this);
}

/*
 *
 */
SubtitleTime::SubtitleTime(const Glib::ustring &str)
{
	try
	{
		hours = mins = secs = msecs = totalmsecs = 0;

		Glib::ustring::size_type pos=0, end=0;

		// hours
		end = str.find(":", pos);
		from_string(str.substr(pos, end), hours);
		pos = end+1;

		// minutes
		end = str.find(":", pos);
		from_string(str.substr(pos, end), mins);
		pos = end+1;

		// seconds
		double ms = 0.0;

		end = str.size();

		from_string(str.substr(pos, end), ms);

		// split into seconds and fraction
		secs = (int)ms;

		msecs = (int)((ms - secs)*1000+0.5);

		totalmsecs = getMSecs(*this);
	}
	catch(...)
	{
		hours = mins = secs = msecs = totalmsecs = 0;
	}
}

/*
 *
 */
SubtitleTime::SubtitleTime(const int &h, const int &m, const int &s, const int &ms)
{
	set(h,m,s,ms);
}

/*
 *
 */
void SubtitleTime::set(const int &h, const int &m, const int &s, const int &ms)
{
	hours = h;
	mins	= m;
	secs	= s;
	msecs = ms;

	totalmsecs = getMSecs(*this);
}

/*
 *
 */
void SubtitleTime::initTotalMSecs()
{
	totalmsecs = getMSecs(*this);
}

/*
 *
 */
void SubtitleTime::move(long msecs)
{
	totalmsecs+=msecs;
	*this = getTime2MSecs(totalmsecs);
}

/*
 *
 */
SubtitleTime SubtitleTime::operator-(const SubtitleTime &b) const
{
	long total = totalmsecs - b.totalmsecs;
	return getTime2MSecs(total);
}

/*
 *
 */
SubtitleTime SubtitleTime::operator+(const SubtitleTime &b) const
{
	long total = totalmsecs + b.totalmsecs;
	return getTime2MSecs(total);
}

/*
 *
 */
double SubtitleTime::operator/(const SubtitleTime &b) const
{
	double total = totalmsecs / b.totalmsecs;
	return total;
}

/*
 *
 */
SubtitleTime SubtitleTime::operator*(const double &mult) const
{
	double total = (double)(totalmsecs * mult);
	return getTime2MSecs((long int)total);
}

/*
 *
 */
bool SubtitleTime::operator>(const SubtitleTime &time) const
{
	return totalmsecs > time.totalmsecs;
}

/*
 *
 */
bool SubtitleTime::operator==(const SubtitleTime &time) const
{
	return totalmsecs == time.totalmsecs;
}

/*
 *
 */
bool SubtitleTime::operator!=(const SubtitleTime &time) const
{
	return totalmsecs != time.totalmsecs;
}

/*
 *
 */
bool SubtitleTime::operator>=(const SubtitleTime &time) const
{
	return totalmsecs >= time.totalmsecs;
}

/*
 *
 */
bool SubtitleTime::operator<(const SubtitleTime &time) const
{
	return totalmsecs < time.totalmsecs;
}

/*
 *
 */
bool SubtitleTime::operator<=(const SubtitleTime &time) const
{
	return totalmsecs <= time.totalmsecs;
}

/*
 *
 */
Glib::ustring SubtitleTime::str() const
{
	return getTime2String(*this);
}


/*
 *	return hours:mins:secs,msecs
 */
std::string getTime2String(const SubtitleTime &time)
{
	gchar *tmp = g_strdup_printf("%01i:%02i:%02i.%03i", 
								time.hours, time.mins, time.secs, time.msecs);
	std::string str(tmp);
	g_free(tmp);

	return str;
}

/*
 *
 */
long getMSecs(const SubtitleTime &time)
{
/*	
	return (( time.hours * 3600 +
						time.mins  * 60 +
						time.secs) * 1000) +
						time.msecs;
*/
	return 3600000 * time.hours + 60000 * time.mins + 1000 * time.secs + time.msecs;
}

/*
 *
 */
SubtitleTime getTime2MSecs(const long &total)
{
	ldiv_t divres;
	
	divres = ldiv(total, 1000);
	long hms = divres.quot;
	int msecs = divres.rem;
	
	divres = ldiv(hms, 60);
	long hm = divres.quot;
	int secs = divres.rem;

	divres = ldiv(hm, 60);
	long h = divres.quot;
	int mins = divres.rem;
	
	int hours = (int)h;
	
	return SubtitleTime(hours, mins, secs, msecs);
}

/*
 *	valide le format du temps
 *	h:mm:ss,ms
 */
bool SubtitleTime::validate(const Glib::ustring &str)
{
	int h,m,s,ms;

	if(sscanf(str.c_str(), "%d:%d:%d.%d", &h, &m, &s, &ms) == 4)
		return true;
	return false;
}

/*
 *	return "0:00:00.000"
 */
Glib::ustring SubtitleTime::null()
{
	return "0:00:00.000";
}

/*
 *	calcul le temps à partir d'une frame et d'un framerate
 *	ex: (450, 23.976)
 */
SubtitleTime SubtitleTime::frame_to_time(const long int& frame, const float& framerate)
{
	// secs = frame / framerate
	// msecs = secs * 1000

	return SubtitleTime((long)((frame / framerate) * 1000));
}

/*
 *
 */
long int SubtitleTime::time_to_frame(const SubtitleTime& time, const float& framerate)
{
	return (long int)round((time.totalmsecs * framerate) / 1000);
}
