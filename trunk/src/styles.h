#ifndef _Styles_h
#define _Styles_h

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
 

#include "style.h"

class Document;

class Styles
{
public:
	Styles(Document &doc);
	~Styles();

	/*
	 *
	 */
	unsigned int size();

	/*
	 *
	 */
	Style get(unsigned int num);

	/*
	 */
	Style first();

	/*
	 *
	 */
	Style last();

	/*
	 *
	 */
	Style append();

	/*
	 *
	 */
	void remove(const Style &style);

protected:
	Document &m_document;
};


#endif//_Styles_h

