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

#include "keyframes.h"
#include "utility.h"
#include <iostream>
#include <fstream>

/*
 */
KeyFrames::KeyFrames()
{
	reference();
}

/*
 */
KeyFrames::~KeyFrames()
{
}

/*
 */
void KeyFrames::reference() const
{
	++ref_count_;
}

/*
 */
void KeyFrames::unreference() const
{
	if(!(--ref_count_))
		delete this;
}

/*
 */
void KeyFrames::set_uri(const Glib::ustring &uri)
{
	m_uri = uri;
}

/*
 */
Glib::ustring KeyFrames::get_uri() const
{
	return m_uri;
}

/*
 * FIXME
 */
bool KeyFrames::open(const Glib::ustring &uri)
{
	try
	{
		Glib::ustring filename = Glib::filename_from_uri(uri);

		std::ifstream file(filename.c_str());
		if(!file)
			return false;

		std::string line;
		if(!std::getline(file, line))
		{
			file.close();
			return false;
		}

		if(line != "#subtitleeditor keyframes v1")
		{
			file.close();
			return false;
		}

		int size = 0;
		std::getline(file, line);
		if(sscanf(line.c_str(), "size: %d", &size) == 0)
		{
			file.close();
			return false;
		}

		reserve(size);

		while(!file.eof() && std::getline(file, line))
		{
			int time = utility::string_to_int(line);
			push_back(time);
		}

		file.close();
		
		set_uri(uri);

		return true;
	}
	catch(std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	catch(Glib::Error &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
	return false;
}

/*
 */
bool KeyFrames::save(const Glib::ustring &uri)
{
	return false;
}

/*
 */
Glib::RefPtr<KeyFrames> KeyFrames::create_from_file(const Glib::ustring &uri)
{
	Glib::RefPtr<KeyFrames> kf(new KeyFrames);
	if(kf->open(uri))
		return kf;
	return Glib::RefPtr<KeyFrames>(NULL);
}
