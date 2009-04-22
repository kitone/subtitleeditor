#ifndef _Error_h
#define _Error_h

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
#include <exception>
#include <string>

/*
 * Base class
 */
class SubtitleError : public std::exception
{
public:
	SubtitleError(const std::string &msg)
	:m_msg(msg)
	{
	}

	virtual ~SubtitleError() throw()
	{
	}

	virtual const char* what() const throw()
	{
		return m_msg.c_str();
	}

private:
	std::string m_msg;
};

/*
 *
 */
class UnrecognizeFormatError : public SubtitleError
{
public:
	UnrecognizeFormatError(const std::string &msg)
	:SubtitleError(msg)
	{
	}
};

/*
 *
 */
class IOFileError : public SubtitleError
{
public:
	IOFileError(const std::string &msg)
	:SubtitleError(msg)
	{
	}
};

/*
 *
 */
class EncodingConvertError : public SubtitleError
{
public:
	EncodingConvertError(const std::string &msg)
	:SubtitleError(msg)
	{
	}
};

#endif//_Error_h

