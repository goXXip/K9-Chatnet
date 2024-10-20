/* Declarations of IRC message structures, variables, and functions.
 *
 * (C) 2003 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id: messages.h 1 2004-03-28 21:59:56Z svn $ 
 *
 */

/*************************************************************************/
#include "modules.h"

extern Message messages[];
extern void moduleAddMsgs(void);
extern Message *find_message(const char *name);


/*************************************************************************/
