/***************************************************************************************/
/* Anope Module : cs_chanlist.c : v1.0.5                                               */  
/* Trystan Scott Lee                                                                   */
/* trystan@nomadirc.net                                                                */ 
/*                                                                                     */
/* Anope (c) 2000-2006 Anope.org                                                       */
/*                                                                                     */ 
/* This program is free software; you can redistribute it and/or modify it under the   */
/* terms of the GNU General Public License as published by the Free Software           */
/* Foundation; either version 1, or (at your option) any later version.                */
/*                                                                                     */
/*  This program is distributed in the hope that it will be useful, but WITHOUT ANY    */
/*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A    */
/*  PARTICULAR PURPOSE.  See the GNU General Public License for more details.          */
/*                                                                                     */
/***************************************************************************************/
/***************************************************************************************/
/* Adds /CHANSERV LIMIT command                                                        */
/***************************************************************************************/
/***************************************************************************************/
/* Module was original done and paid for by Mark <mark@chasingsol.com>                 */
/*                                                                                     */
/* At this time of release there is a known issue that may cause Anope to lock up      */
/* or at least it has for Mark, I make no promises that the same won't happen if       */
/* you run this module on your network.                                                */
/***************************************************************************************/

services.conf config options

ChanLimitDB "chanlimit.db"   <--- name of the channel limit db file
ChanLimitDefaultVar 5        <--- default variance
ChanLimitDefaultTimer "10s"  <--- default timer
ChanLimitDefaultMinVar 3     <--- lowest anyone can take the variance
ChanLimitDefaultMinTimer "5s" <-- short the timer can be set to
ChanLimitNoTimers             <-- disable using callback functions and set mode right then

You need both the *.c and *.h file, place both in the src/modules folder and build as 
normal.

Please let me know if you run into any issues with it

======================================================================================
Known issues
 - if you join services to the channel being monitored you count will be off as 
   services do not count towards user level in channel


======================================================================================

Change Log

1.0.5
- Fixed defines not showing what function was being called
- new option ChanLimitNoTimers to not use the core Anope timers and just go straight to
  setting the mode.
- removed debug: from in front of the config file alog() messages
- show the module name and extension in the debug messages.
- on module load it will set the modes on channels

1.0.4
- Added debug to everything so we can find the crash bug
- CreateChanLimit() will only alloc if it can't find the channel first, to prevent
  memory from being eatten up.
- Removed stray "continue" in handle_join()

1.0.3
- prevent linking to channels without a #
- prevent linking to channels that are to long
- handles botserv bot being ASSIGN and UNASSIGN

1.0.2
 - the ON command takes into account botserv bots
 - alog() for SET MAX now shows what channel
 - can no longer use SET VARIANCE to raise the limit if the limit is already at max
 - fixed display not showing LINKONMAX
 - do_save() no longer executes twice
 - linkonmax is called LINK now
 - added cs limit #chan view 
 - linking is only setable on ircds with +L
 - Removed redunant check of cl->use_limit in do_event_join()

1.0.1 
 - New config options to allow the user to define the min values for variance and timer
 - random timer names to avoid issues with anope core
 - backups the database on EVENT_DB_BACKUP
 - changing the variance cause it to update the mode in channel
 - permissions check for if the user can set
 - permissions check to make sure the channel is not suspended
 - all commands are logged to the logchan
 - the set command is handled differently then before
 - feature for the channel max, so that it won't go any higher
 - feature for setting a redirect on reaching the channel max
 - timers check the last time the channel had a executed timer.. so it doesn't flood, 
   it does try to do 1 callback again but till the one executes it won't retry unless 
   the last call back is greater then the channels timer.. this should prevent a massive 
   join and flooding with modes as it tries to keep up
 - better error handling if it can not bind to events so it can be debugged better

1.0
 - first release