#ifndef _CheckErrorsUI_h
#define _CheckErrorsUI_h

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
 

/*
 * Overlapping (chevauchement) : Une erreur est détectée lorsque deux sous-titres se chevauchent.
 * Too short display time (temps trop court) : Une erreur est détectée lorsque le nombre de caractères par seconde est strictement supérieur à une valeur définie par l'utilisateur.
 * Too long display time (temps trop long) : Une erreur est détectée lorsque le nombre de caractères par seconde est strictement inférieur à une valeur définie par l'utilisateur.
 * Too long line (ligne trop longue) : Une erreur est détectée lorsque la longueur d'une ligne de sous-titre est strictement supérieur à une valeur spécifiée par l'utilisateur.
 * Transition (respect le temps entre chaque sous-titre)
*/

#include "Document.h"


void createDialogCheckErrors();


#endif//_CheckErrorsUI_h

