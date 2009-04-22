#ifndef _Extension_h
#define _Extension_h

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
class Extension
{
public:
	/*
	 *
	 */
	Extension();

	/*
	 *
	 */
	virtual ~Extension();

	/*
	 *
	 */
	virtual bool is_configurable();

	/*
	 *
	 */
	virtual void create_configure_dialog();
};

/*
 *
 */
#define REGISTER_EXTENSION(classname) \
	extern "C" Extension* extension_register() \
	{ \
		return new classname; \
	}

#endif//_Extension_h
