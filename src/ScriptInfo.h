#ifndef _ScriptInfo_h
#define _ScriptInfo_h

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
 

#include <glibmm/ustring.h>

/*
 *	principalement utiliser par SSA/ASS
 */
class ScriptInfo
{
public:
	ScriptInfo();

public:
	Glib::ustring	Title;	// courte description du script
	Glib::ustring OriginalScript;	// le(s) auteur(s) du script
	Glib::ustring OriginalTranslation;
	Glib::ustring OriginalEditing;
	Glib::ustring OriginalTiming;
	Glib::ustring SynchPoint;
	Glib::ustring ScriptUpdatedBy;
	Glib::ustring UpdateDetails;
	Glib::ustring ScriptType; // version du format SSA : ex: "V4.00" (ASS="V4.00+")
	Glib::ustring Collisions;
	Glib::ustring PlayResY;
	Glib::ustring PlayResX;
	Glib::ustring PlayDepth;
	Glib::ustring Timer;	// vitesse de lecture du script en %
	Glib::ustring Dialogue;
	Glib::ustring Comment;
	Glib::ustring Picture;
	Glib::ustring Sound;
	Glib::ustring Movie;
	Glib::ustring	Command;
	Glib::ustring	WrapStyle;
};

#endif//_ScriptInfo_h

