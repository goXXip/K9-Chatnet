/* Routines for time-delayed actions.
 *
 * (C) 2003 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id: timeout.c 5 2004-03-29 01:29:50Z dane $ 
 *
 */

#include "services.h"
#include "timeout.h"

static Timeout *timeouts = NULL;

/*************************************************************************/

#ifdef DEBUG_COMMANDS

/* Send the timeout list to the given user. */

void send_timeout_list(User * u)
{
    Timeout *to, *last;

    notice(s_OperServ, u->nick, "Now: %ld", time(NULL));
    for (to = timeouts, last = NULL; to; last = to, to = to->next) {
        notice(s_OperServ, u->nick, "%p: %ld: %p (%p)",
               to, to->timeout, to->code, to->data);
        if (to->prev != last)
            notice(s_OperServ, u->nick,
                   "    to->prev incorrect!  expected=%p seen=%p",
                   last, to->prev);
    }
}

#endif                          /* DEBUG_COMMANDS */

/*************************************************************************/

/* Check the timeout list for any pending actions. */

void check_timeouts(void)
{
    Timeout *to, *to2;
    time_t t = time(NULL);

    if (debug >= 2)
        alog("debug: Checking timeouts at %ld", t);

    to = timeouts;
    while (to) {
        if (t < to->timeout) {
            to = to->next;
            continue;
        }
        if (debug >= 4) {
            alog("debug: Running timeout %p (code=%p repeat=%d)",
                 to, to->code, to->repeat);
        }
        to->code(to);
        if (to->repeat) {
            to = to->next;
            continue;
        }
        to2 = to->next;
        if (to->next)
            to->next->prev = to->prev;
        if (to->prev)
            to->prev->next = to->next;
        else
            timeouts = to->next;
        free(to);
        to = to2;
    }
    if (debug >= 2)
        alog("debug: Finished timeout list");
}

/*************************************************************************/

/* Add a timeout to the list to be triggered in `delay' seconds.  If
 * `repeat' is nonzero, do not delete the timeout after it is triggered.
 * This must maintain the property that timeouts added from within a
 * timeout routine do not get checked during that run of the timeout list.
 */

Timeout *add_timeout(int delay, void (*code) (Timeout *), int repeat)
{
    Timeout *t = scalloc(sizeof(Timeout), 1);
    t->settime = time(NULL);
    t->timeout = t->settime + delay;
    t->code = code;
    t->repeat = repeat;
    t->next = timeouts;
    t->prev = NULL;
    if (timeouts)
        timeouts->prev = t;
    timeouts = t;
    return t;
}

/*************************************************************************/

/* Remove a timeout from the list (if it's there). */

void del_timeout(Timeout * t)
{
    Timeout *ptr;

    for (ptr = timeouts; ptr; ptr = ptr->next) {
        if (ptr == t)
            break;
    }
    if (!ptr)
        return;
    if (t->prev)
        t->prev->next = t->next;
    else
        timeouts = t->next;
    if (t->next)
        t->next->prev = t->prev;
    free(t);
}

/*************************************************************************/
