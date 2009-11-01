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

#include <iostream>
#include <giomm.h>
#include <cstdio>
#include "error.h"
#include "keyframes.h"
#include "utility.h"

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
 */
bool KeyFrames::open(const Glib::ustring &uri)
{
	try
	{
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
		Glib::RefPtr<Gio::FileInputStream> fstream = file->read();
		Glib::RefPtr<Gio::DataInputStream> dstream = Gio::DataInputStream::create(fstream);

		int size = 0;
		std::string line;

		// Check the file type
		if((dstream->read_line(line) && line == "#subtitleeditor keyframes v1") == false)
			throw SubtitleError(_("Couldn't recognize format of the file."));

		// Read the keyframes number
		if((dstream->read_line(line) && sscanf(line.c_str(), "size: %d", &size) != 0) == false)
			throw SubtitleError(_("Couldn't get the keyframe size on the file."));

		// Read the keyframes data
		reserve(size);
		while(dstream->read_line(line))
		{
			push_back( utility::string_to_int(line) );
		}
		// Update the uri of the keyframe
		set_uri(uri);
		return true;
	}
	catch(const std::exception &ex)
	{
		std::cerr << Glib::ustring::compose("KeyFrames::open failed '%1' : %2", uri, ex.what()) << std::endl;
	}
	return false;
}

/*
 */
bool KeyFrames::save(const Glib::ustring &uri)
{
	try
	{
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_uri(uri);
		// If the file exists then replace it. Otherwise, create it.
		Glib::RefPtr<Gio::FileOutputStream> stream = (file->query_exists()) ? file->replace() : file->create_file();

		if(!stream)
			throw SubtitleError(Glib::ustring::compose("Gio::File::create_file returned an emptry ptr from the uri '%1'.", uri));

		stream->write("#subtitleeditor keyframes v1\n");
		stream->write(Glib::ustring::compose("size: %1\n", size()));

		for(guint i=0; i < size(); ++i)
			stream->write(Glib::ustring::compose("%1\n", (*this)[i]));

		// Close the stream to make sure that changes are write now.
		stream->close();
		stream.reset();
		// Update the uri of the keyframe
		set_uri(uri);

		return true;
	}
	catch(const std::exception &ex)
	{
		std::cerr << Glib::ustring::compose("KeyFrames::save failed '%1' : %2", uri, ex.what()) << std::endl;
	}
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
