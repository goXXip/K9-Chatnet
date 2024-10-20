/* Routines for sending stuff to the network.
 *
 * (C) 2003 Anope Team
 * Contact us at info@anope.org
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id: send.c 5 2004-03-29 01:29:50Z dane $ 
 *
 */

#include "services.h"

/*************************************************************************/

/* Send a command to the server.  The two forms here are like
 * printf()/vprintf() and friends. */

void send_cmd(const char *source, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsend_cmd(source, fmt, args);
    va_end(args);
}

void vsend_cmd(const char *source, const char *fmt, va_list args)
{
    char buf[BUFSIZE];

    vsnprintf(buf, sizeof(buf), fmt, args);
    if (source) {
        sockprintf(servsock, ":%s %s\r\n", source, buf);
        if (debug)
            alog("debug: Sent: :%s %s", source, buf);
    } else {
        sockprintf(servsock, "%s\r\n", buf);
        if (debug)
            alog("debug: Sent: %s", buf);
    }
}

/*************************************************************************/

/* Send out a WALLOPS (a GLOBOPS on ircd.dal). */

void wallops(const char *source, const char *fmt, ...)
{
    va_list args;
    char buf[BUFSIZE];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);
#ifdef IRC_HYBRID
    send_cmd(source ? source : ServerName, "WALLOPS :%s", buf);
#else
    send_cmd(source ? source : ServerName, "GLOBOPS :%s", buf);
#endif
}

/*************************************************************************/

/* Send a NOTICE from the given source to the given nick. */
void notice(const char *source, const char *dest, const char *fmt, ...)
{
    va_list args;
    char buf[BUFSIZE];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);
    send_cmd(source, "%s %s :%s", (UsePrivmsg ? "PRIVMSG" : "NOTICE"),
             dest, buf);
}

/*************************************************************************/

void notice_user(const char *source, User * u, const char *fmt, ...)
{
    va_list args;
    char buf[BUFSIZE];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);
    send_cmd(source, "%s %s :%s",
             (UsePrivmsg && (!u->na || (u->na->nc->flags & NI_MSG)) ?
              "PRIVMSG" : "NOTICE"), u->nick, buf);
}

/*************************************************************************/

/* Send a NULL-terminated array of text as NOTICEs. */
void notice_list(const char *source, const char *dest, const char **text)
{
    while (*text) {
        /* Have to kludge around an ircII bug here: if a notice includes
         * no text, it is ignored, so we replace blank lines by lines
         * with a single space.
         */
        if (**text)
            notice(source, dest, *text);
        else
            notice(source, dest, " ");
        text++;
    }
}

/*************************************************************************/

/* Send a message in the user's selected language to the user using NOTICE. */
void notice_lang(const char *source, User * dest, int message, ...)
{
    va_list args;
    char buf[4096];             /* because messages can be really big */
    char *s, *t;
    const char *fmt;
    if (!dest)
        return;
    va_start(args, message);
    fmt = getstring(dest->na, message);
    if (!fmt)
        return;
    memset(buf, 0, 4096);
    vsnprintf(buf, sizeof(buf), fmt, args);
    s = buf;
    while (*s) {
        t = s;
        s += strcspn(s, "\n");
        if (*s)
            *s++ = 0;
        send_cmd(source, "%s %s :%s", (UsePrivmsg
                                       && (!dest->na || (dest->na->nc->
                                                         flags &
                                                         NI_MSG)) ?
                                       "PRIVMSG" : "NOTICE"),
                 dest->nick, *t ? t : " ");
    }
    //flaggazzo=1;
}

/*************************************************************************/

/* Like notice_lang(), but replace %S by the source.  This is an ugly hack
 * to simplify letting help messages display the name of the pseudoclient
 * that's sending them.
 */
void notice_help(const char *source, User * dest, int message, ...)
{
    va_list args;
    char buf[4096], buf2[4096], outbuf[BUFSIZE];
    char *s, *t;
    const char *fmt;

    if (!dest)
        return;
    va_start(args, message);
    fmt = getstring(dest->na, message);
    if (!fmt)
        return;
    /* Some sprintf()'s eat %S or turn it into just S, so change all %S's
     * into \1\1... we assume this doesn't occur anywhere else in the
     * string. */
    strscpy(buf2, fmt, sizeof(buf2));
    strnrepl(buf2, sizeof(buf2), "%S", "\1\1");
    vsnprintf(buf, sizeof(buf), buf2, args);
    s = buf;
    while (*s) {
        t = s;
        s += strcspn(s, "\n");
        if (*s)
            *s++ = 0;
        strscpy(outbuf, t, sizeof(outbuf));
        strnrepl(outbuf, sizeof(outbuf), "\1\1", source);
        send_cmd(source, "%s %s :%s",
                 (UsePrivmsg
                  && (!dest->na
                      || (dest->na->nc->
                          flags & NI_MSG)) ? "PRIVMSG" : "NOTICE"),
                 dest->nick, *outbuf ? outbuf : " ");
    }
}

/*************************************************************************/

/* Send a PRIVMSG from the given source to the given nick. */
void privmsg(const char *source, const char *dest, const char *fmt, ...)
{
    va_list args;
    char buf[BUFSIZE];

    va_start(args, fmt);

    vsnprintf(buf, sizeof(buf), fmt, args);
    send_cmd(source, "PRIVMSG %s :%s", dest, buf);
}

/*************************************************************************/
