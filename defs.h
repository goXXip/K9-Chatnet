/* Set default values for any constants that should be in include files but
 *
 * (C) 2003 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * 
 * $Id: defs.h 1 2004-03-28 21:59:56Z svn $ 
 *
 */

/*************************************************************************/

#ifndef NAME_MAX
# define NAME_MAX 255
#endif

#ifndef BUFSIZ
# define BUFSIZ 256
#else
# if BUFSIZ < 256
#  define BUFSIZ 256
# endif
#endif

/* Length of an array: */
#define lenof(a)	(sizeof(a) / sizeof(*(a)))

/* Telling compilers about printf()-like functions: */
#ifdef __GNUC__
# define FORMAT(type,fmt,start) __attribute__((format(type,fmt,start)))
#else
# define FORMAT(type,fmt,start)
#endif

/*************************************************************************/
