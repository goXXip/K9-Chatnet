/* ChanServ functions.
 *
 * (C) 2005 CNservices Team
 * Contact us at coders@smit-h.net
 *
 * Please read COPYING and README for furhter details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church. 
 * 
 * $Id: chanserv.c 5 2004-03-29 01:29:50Z dane $ 
 *
 */

 /* ChanServ restart needs access to this if were gonna avoid sending ourself a signal */
 extern int do_restart_services(void);



/*************************************************************************/

#include "services.h"
#include "pseudo.h"


/*
 * Disable the modules on OpenBSD (for now)
 * there is work in progress for this.
 */
#ifdef __OpenBSD__
#ifdef USE_MODULES
#undef USE_MODULES
#endif                          /* USE_MODULES */
#endif                          /* __OpenBSD__ */

extern Module *mod_current_module;
extern char *lastcommand;
extern int mod_current_op;
extern User *mod_current_user;
extern ModuleHash *MODULE_HASH[MAX_CMD_HASH];


/*************************************************************************/
/* *INDENT-OFF* */

ChannelInfo *chanlists[256];


static int def_levels[][2] = {
    { CA_AUTOOP,                   400 },
    { CA_JOIN,                     450 },
    { CA_PART,                     450 },
    { CA_DIEDIE,                   800 },
    { CA_RESTART,                  800 },
    { CA_DESC,                     800 },
    { CA_URL,                      800 },
    { CA_NOEXPIRE,                 800 },
    { CA_FORBID,                   800 },
    { CA_UPDATE,                   800 },
    { CA_SCHAN,                    630 },
    { CA_UNSCHAN,                  630 },
    { CA_REGISTER,                 630 },
    { CA_GETPASS,                  610 },
    { CA_LASTCMDS,                 610 },
    { CA_LIST,                     610 },
    { CA_EMAIL,                    610 },
    { CA_FOUNDER,                  610 },
    { CA_STATUS,                   610 },
    { CA_REMCHAN,                  610 },
    { CA_GENPASS,                  600 },
    { CA_SETPASS,                  500 },
    { CA_SUCCESSOR,                500 },
    { CA_RESTART,                  800 },
    { CA_ACT,                      500 },
    { CA_AUTH,                     500 },
    { CA_SND,                      500 },
    { CA_SAY,                      500 },
    { CA_ONJOIN,                   450 },
    { CA_GUARDM,                   450 },
    { CA_GUARDT,                   450 },
    { CA_AUTOVOICE,                 2 },
    { CA_AUTODEOP,                  -2 },
    { CA_NOJOIN,                    -2 },
    { CA_INVITE,                     100 },
    { CA_AKICK,                     75 },
    { CA_SET,     		ACCESS_INVALID },
    { CA_CLEAR,   		ACCESS_INVALID },
    { CA_UNBAN,                      75 },
    { CA_COMMENT,                      75 },
    { CA_OPDEOP,                     100 },
    { CA_ACCESS_LIST,                1 },
    { CA_ACCESS_CHANGE,             400 },
    { CA_MEMO,                      10 },
    { CA_ASSIGN,  		ACCESS_INVALID },
    { CA_BADWORDS,                  10 },
    { CA_NOKICK,                     1 },
    { CA_FANTASIA,                   3 },
    { CA_GREET,                      5 },
    { CA_VOICEME,                    100 },
    { CA_VOICE,                      100 },
    { CA_GETKEY,                     5 },
    { CA_AUTOHALFOP,                 4 },
    { CA_AUTOPROTECT,               2 },
    { CA_OPDEOPME,                   100 },
    { CA_HALFOPME,                   4 },
    { CA_HALFOP,                     5 },
    { CA_PROTECTME,                 2 },
    { CA_PROTECT,  		ACCESS_INVALID },
    { CA_KICKME,               		 50 },
    { CA_KICK,                       50 },
    { CA_SIGNKICK, 		ACCESS_INVALID },
    { CA_BANME,                      75 },
    { CA_BAN,                        75 },
    { CA_TOPIC,         200 },
    { CA_INFO,          ACCESS_INVALID },
    { -1 }
};

typedef struct {
    int what;
    char *name;
    int desc;
} LevelInfo;
static LevelInfo levelinfo[] = {
    { CA_AUTODEOP,      "AUTODEOP",   	CHAN_LEVEL_AUTODEOP },
#ifdef HAS_HALFOP
	{ CA_AUTOHALFOP,    "AUTOHALFOP",   CHAN_LEVEL_AUTOHALFOP },
#endif
    { CA_AUTOOP,        "AUTOOP",     	CHAN_LEVEL_AUTOOP },
#ifdef IRC_UNREAL
	{ CA_AUTOPROTECT,   "AUTOPROTECT",  CHAN_LEVEL_AUTOPROTECT },
#endif
#ifdef IRC_VIAGRA
        { CA_AUTOPROTECT,   "AUTOPROTECT",  CHAN_LEVEL_AUTOPROTECT },
#endif
#ifdef IRC_ULTIMATE3
	{ CA_AUTOPROTECT,   "AUTOADMIN",  CHAN_LEVEL_AUTOPROTECT },
#endif
    { CA_AUTOVOICE,     "AUTOVOICE",  	CHAN_LEVEL_AUTOVOICE },
    { CA_NOJOIN,        "NOJOIN",     	CHAN_LEVEL_NOJOIN },
    { CA_SIGNKICK,      "SIGNKICK",     CHAN_LEVEL_SIGNKICK },

    { CA_ACCESS_LIST,   "ACC-LIST",   	CHAN_LEVEL_ACCESS_LIST },
    { CA_ACCESS_CHANGE, "ACC-CHANGE", 	CHAN_LEVEL_ACCESS_CHANGE },
    { CA_AKICK,         "AKICK",      	CHAN_LEVEL_AKICK },
    { CA_SET,           "SET",        	CHAN_LEVEL_SET },
    { CA_ACT,           "ACT",        	CHAN_LEVEL_ACT },
    { CA_RESTART,       "RESTART",      CHAN_LEVEL_ACT },
    { CA_SUCCESSOR,     "SUCCESSOR",    CHAN_LEVEL_ACT },
    { CA_SETPASS,       "SETPASS",      CHAN_LEVEL_ACT },

    { CA_BAN,           "BAN",          CHAN_LEVEL_BAN },
	{ CA_BANME,         "BANME",        CHAN_LEVEL_BANME },
    { CA_CLEAR,         "CLEAR",      	CHAN_LEVEL_CLEAR },
    { CA_GETKEY,        "GETKEY",     	CHAN_LEVEL_GETKEY },
#ifdef HAS_HALFOP
	{ CA_HALFOP,        "HALFOP",       CHAN_LEVEL_HALFOP },
    { CA_HALFOPME,      "HALFOPME",     CHAN_LEVEL_HALFOPME },
#endif
    { CA_INFO,          "INFO",         CHAN_LEVEL_INFO },
	{ CA_KICK,          "KICK",         CHAN_LEVEL_KICK },
//	{ CA_COMMENT,          "COMMENT",         CHAN_LEVEL_COMMENT },
	{ CA_KICKME,        "KICKME",       CHAN_LEVEL_KICKME },
    { CA_INVITE,        "INVITE",     	CHAN_LEVEL_INVITE },
    { CA_OPDEOP,        "OPDEOP",     	CHAN_LEVEL_OPDEOP },
    { CA_OPDEOPME,      "OPDEOPME",     CHAN_LEVEL_OPDEOPME },
#ifdef IRC_UNREAL
	{ CA_PROTECT,       "PROTECT",      CHAN_LEVEL_PROTECT },
    { CA_PROTECTME,     "PROTECTME",    CHAN_LEVEL_PROTECTME },
#endif
#ifdef IRC_VIAGRA
        { CA_PROTECT,       "PROTECT",      CHAN_LEVEL_PROTECT },
    { CA_PROTECTME,     "PROTECTME",    CHAN_LEVEL_PROTECTME },
#endif
#ifdef IRC_ULTIMATE3
  { CA_PROTECT,       "ADMIN",      CHAN_LEVEL_PROTECT },
    { CA_PROTECTME,     "ADMINME",    CHAN_LEVEL_PROTECTME },
#endif
    { CA_TOPIC,         "TOPIC",        CHAN_LEVEL_TOPIC },
    { CA_UNBAN,         "UNBAN",      	CHAN_LEVEL_UNBAN },
    { CA_VOICE,         "VOICE",      	CHAN_LEVEL_VOICE },
    { CA_VOICEME,       "VOICEME",      CHAN_LEVEL_VOICEME },

    { CA_MEMO,          "MEMO",       	CHAN_LEVEL_MEMO },

    { CA_ASSIGN,        "ASSIGN",     	CHAN_LEVEL_ASSIGN },
    { CA_BADWORDS,      "BADWORDS",   	CHAN_LEVEL_BADWORDS },
    { CA_FANTASIA,      "FANTASIA",   	CHAN_LEVEL_FANTASIA },
    { CA_GREET,			"GREET",	  	CHAN_LEVEL_GREET },
    { CA_NOKICK,        "NOKICK",     	CHAN_LEVEL_NOKICK },
    { CA_SAY,			"SAY",		  	CHAN_LEVEL_SAY },
	{ CA_SCHAN,         "SCHAN",        CHAN_LEVEL_TOPIC },

    { -1 }
};
static int levelinfo_maxwidth = 0;

CSModeUtil csmodeutils[] = {
	{ "DEOP",   	"!deop",		"-o",   CI_OPNOTICE,	CA_OPDEOP, CA_OPDEOPME },
	{ "OP",			"!op",			"+o",	CI_OPNOTICE,	CA_OPDEOP, CA_OPDEOPME },
	{ "DEVOICE",	"!devoice", 	"-v",   0          ,	CA_VOICE,  CA_VOICEME  },
	{ "VOICE",		"!voice",   	"+v",   0          ,	CA_VOICE,  CA_VOICEME  },
#ifdef HAS_HALFOP
    { "DEHALFOP",	"!dehalfop",	"-h",	0          ,	CA_HALFOP, CA_HALFOPME },
	{ "HALFOP",		"!halfop",		"+h",	0          ,	CA_HALFOP, CA_HALFOPME },
#endif
#ifdef IRC_UNREAL
	{ "DEPROTECT",	"!deprotect",	"-a",	0          ,	CA_PROTECT, CA_PROTECTME },
	{ "PROTECT",	"!protect",		"+a",	0          ,	CA_PROTECT, CA_PROTECTME },
#endif
#ifdef IRC_VIAGRA
	{ "DEPROTECT",	"!deprotect",	"-a",	0          ,	CA_PROTECT, CA_PROTECTME },
	{ "PROTECT",	"!protect",		"+a",	0          ,	CA_PROTECT, CA_PROTECTME },
#endif
#ifdef IRC_ULTIMATE3
  	{ "DEPROTECT",	"!deadmin",	"-a",	0          ,	CA_PROTECT, CA_PROTECTME },
	{ "PROTECT",	"!admin",		"+a",	0          ,	CA_PROTECT, CA_PROTECTME },
#endif

	{ NULL }
};

int xop_msgs[4][14] = {
	{	CHAN_AOP_SYNTAX,
		CHAN_AOP_DISABLED,
		CHAN_AOP_NICKS_ONLY,
		CHAN_AOP_ADDED,
		CHAN_AOP_MOVED,
		CHAN_AOP_NO_SUCH_ENTRY,
		CHAN_AOP_NOT_FOUND,
		CHAN_AOP_NO_MATCH,
		CHAN_AOP_DELETED,
		CHAN_AOP_DELETED_ONE,
		CHAN_AOP_DELETED_SEVERAL,
		CHAN_AOP_LIST_EMPTY,
		CHAN_AOP_LIST_HEADER,
		CHAN_AOP_CLEAR
	},
	{	CHAN_SOP_SYNTAX,
		CHAN_SOP_DISABLED,
		CHAN_SOP_NICKS_ONLY,
		CHAN_SOP_ADDED,
		CHAN_SOP_MOVED,
		CHAN_SOP_NO_SUCH_ENTRY,
		CHAN_SOP_NOT_FOUND,
		CHAN_SOP_NO_MATCH,
		CHAN_SOP_DELETED,
		CHAN_SOP_DELETED_ONE,
		CHAN_SOP_DELETED_SEVERAL,
		CHAN_SOP_LIST_EMPTY,
		CHAN_SOP_LIST_HEADER,
		CHAN_SOP_CLEAR
	},
	{	CHAN_VOP_SYNTAX,
		CHAN_VOP_DISABLED,
		CHAN_VOP_NICKS_ONLY,
		CHAN_VOP_ADDED,
		CHAN_VOP_MOVED,
		CHAN_VOP_NO_SUCH_ENTRY,
		CHAN_VOP_NOT_FOUND,
		CHAN_VOP_NO_MATCH,
		CHAN_VOP_DELETED,
		CHAN_VOP_DELETED_ONE,
		CHAN_VOP_DELETED_SEVERAL,
		CHAN_VOP_LIST_EMPTY,
		CHAN_VOP_LIST_HEADER,
		CHAN_VOP_CLEAR
	},
	{	CHAN_HOP_SYNTAX,
		CHAN_HOP_DISABLED,
		CHAN_HOP_NICKS_ONLY,
		CHAN_HOP_ADDED,
		CHAN_HOP_MOVED,
		CHAN_HOP_NO_SUCH_ENTRY,
		CHAN_HOP_NOT_FOUND,
		CHAN_HOP_NO_MATCH,
		CHAN_HOP_DELETED,
		CHAN_HOP_DELETED_ONE,
		CHAN_HOP_DELETED_SEVERAL,
		CHAN_HOP_LIST_EMPTY,
		CHAN_HOP_LIST_HEADER,
		CHAN_HOP_CLEAR
	}
};

/* *INDENT-ON* */
/*************************************************************************/

static void alpha_insert_chan(ChannelInfo * ci);
static ChannelInfo *makechan(const char *chan);
static int delchan(ChannelInfo * ci);
static void reset_levels(ChannelInfo * ci);
static int is_real_founder(User * user, ChannelInfo * ci);
static int is_identified(User * user, ChannelInfo * ci);
static int do_banlist(User * u);
static int do_help(User * u);
static int do_version(User * u);
static int do_say(User * u);
static int do_mode(User * u);
static int do_suspends(User * u);
static int do_chanlimit(User * u);
static int do_register(User * u);
static int do_identify(User * u);
static int do_drop(User * u);
static int do_restart(User * u);
static int do_reload(User * u);
static int do_shutdown(User * u);
static int do_update(User * u);
static int do_autoop(User * u);
static int do_command(User * u);
static int do_comment(User * u);
static int do_csjoin(User * u);
static int do_lastcmds(User * u);
static int do_cspart(User * u);
static int do_act(User * u);
static int do_snd(User * u);
static int do_access(User * u);
static int do_adduser(User * u);
static int do_moduser(User * u);
static int do_remuser(User * u);
//static int do_akick(User * u);
static int do_info(User * u);
static int do_list(User * u);
static int do_invite(User * u);
static int do_util(User * u, CSModeUtil * util);
static int do_op(User * u);
static int do_deop(User * u);
static int do_voice(User * u);
static int do_devoice(User * u);
static int  save_history(char *buf,User *u);
#ifdef HAS_HALFOP
static int do_halfop(User * u);
static int do_dehalfop(User * u);
#endif
#ifdef IRC_UNREAL
static int do_protect(User * u);
static int do_deprotect(User * u);
static int do_owner(User * u);
static int do_deowner(User * u);
#endif
#ifdef IRC_VIAGRA
static int do_protect(User * u);
static int do_deprotect(User * u);
static int do_owner(User * u);
static int do_deowner(User * u);
#endif
#ifdef IRC_ULTIMATE3
static int do_protect(User * u);
static int do_deprotect(User * u);
#endif
static int do_cs_kick(User * u);
static int do_ban(User * u);
static int do_cs_topic(User * u);
static int do_unban(User * u);
static int do_getpass(User * u);
static int do_sendpass(User * u);
static int do_forbid(User * u);
static int do_suspend(User * u);
static int do_unsuspend(User * u);
static int do_status(User * u);
static int do_set_founder(User * u);
static int do_set_successor(User * u);
static int do_set_password(User * u);
static int do_set_desc(User * u);
static int do_set_url(User * u);
static int do_set_email(User * u);
static int do_set_entrymsg(User * u);
static int do_set_topiclock(User * u);
static int do_set_mlock(User * u);
static int do_set_noexpire(User * u);

#ifdef USE_MODULES
int do_modload(User * u); 
int do_modunload(User * u);
int do_modlist(User * u);
int do_modinfo(User * u);
static int showModuleCmdLoaded(CommandHash * cmdList, char *mod_name, User * u);
static int showModuleMsgLoaded(MessageHash * msgList, char *mod_name,User * u);
#endif

void moduleAddChanServCmds(void);
/*************************************************************************/
/* *INDENT-OFF* */
void moduleAddChanServCmds(void) {
    Command *c;
    c = createCommand("COMMANDS",  do_command, NULL, CHAN_HELP_COMMANDS,-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("COMMENT",  do_comment, NULL, CHAN_HELP_COMMENT,-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("BANLIST",   do_banlist, NULL, CHAN_HELP_BANLIST, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("HELP",     do_help,     NULL,  -1,                       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("SAY",				do_say,		 NULL,  CHAN_HELP_SAY,				 -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("MODE", do_mode, NULL, CHAN_HELP_MODE , 
-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("AUTOOP",     do_autoop,     NULL,  CHAN_HELP_AUTOOPS,                       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("SND",     do_snd,     NULL,  CHAN_HELP_SND,                       
-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("JOIN",     do_csjoin,     NULL, CHAN_HELP_JOIN,                         -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("PART",     do_cspart,     NULL, CHAN_HELP_PART,                         -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("LASTCMDS",     do_lastcmds,     NULL,  CHAN_HELP_LASTCMDS,                       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("VERSION",       do_version,       NULL,  VERSION_NO_HELP,                   
-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("REGISTER", do_register, is_services_admin,  
CHAN_HELP_REGISTER,       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("ADDCHAN", do_register, NULL, CHAN_HELP_ADDCHAN, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("RESTART",    do_restart,    NULL,OPER_HELP_RESTART, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("REHASH",    do_reload,    
is_services_admin,VERSION_NO_HELP, -1,-1,-1,-1); 
addCoreCommand(CHANSERV,c);
    c = createCommand("DIEDIE",   do_shutdown,   is_services_admin,OPER_HELP_SHUTDOWN, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("UPDATE",     do_update,     is_services_admin,OPER_HELP_UPDATE, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("AUTH", do_identify, NULL,  CHAN_HELP_IDENTIFY,       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("REMCHAN",     do_drop,     NULL,  CHAN_HELP_DROP, -1,-1,-1,-1 ); addCoreCommand(CHANSERV,c);
    c = createCommand("FOUNDER",    do_set_founder,  NULL,  CHAN_HELP_SET_FOUNDER,    -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("SUCCESSOR",  do_set_successor,  NULL,  CHAN_HELP_SET_SUCCESSOR,  -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("SETPASS",   do_set_password,  NULL,  CHAN_HELP_SET_PASSWORD,  -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DESC",       do_set_desc,  NULL,       -1,-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("URL",        do_set_url,  NULL,  CHAN_HELP_SET_URL,       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("EMAIL",      do_set_email,  NULL,  CHAN_HELP_SET_EMAIL,      -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("ONJOIN",   do_set_entrymsg,  NULL,  CHAN_HELP_SET_ENTRYMSG,   -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("GUARDT",  do_set_topiclock,  NULL,  CHAN_HELP_SET_TOPICLOCK,  -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("GUARDM",      do_set_mlock,  NULL,  CHAN_HELP_SET_MLOCK,      -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("ACT",                            do_act,          NULL,  BOT_HELP_ACT, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("NOEXPIRE",   do_set_noexpire,  NULL, VERSION_NO_HELP, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#ifdef HAS_HALFOP
    c = createCommand("HOP",      do_hop,      NULL,  CHAN_HELP_HOP,            -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#endif
    c = createCommand("ADDUSER",  do_adduser,  NULL,  CHAN_HELP_ADDUSER, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
//    c = createCommand("AUTOLIMIT",     do_fixme,     NULL, CHAN_HELP_CHANLIMIT, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("MODUSER",  do_moduser,  NULL,  CHAN_HELP_MODUSER, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("REMUSER",  do_remuser,  NULL,  CHAN_HELP_REMUSER, -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("ACCESS",   do_access,   NULL,  CHAN_HELP_ACCESS,         -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
   // c = createCommand("AKICK",    do_akick,    NULL,  CHAN_HELP_AKICK,          -1,-1,-1,-1); 
addCoreCommand(CHANSERV,c);
    c = createCommand("INFO",     do_info,     NULL,  CHAN_HELP_INFO,-1, CHAN_SERVADMIN_HELP_INFO, CHAN_SERVADMIN_HELP_INFO,CHAN_SERVADMIN_HELP_INFO); addCoreCommand(CHANSERV,c);
    c = createCommand("LIST",     do_list,     NULL,  -1,CHAN_HELP_LIST, CHAN_SERVADMIN_HELP_LIST,CHAN_SERVADMIN_HELP_LIST, CHAN_SERVADMIN_HELP_LIST); addCoreCommand(CHANSERV,c);
    c = createCommand("OP",       do_op,       NULL,  CHAN_HELP_OP,             -1,-1,-1,-1); 
addCoreCommand(CHANSERV,c);
    c = createCommand("DEOP",     do_deop,     NULL,  CHAN_HELP_DEOP,           -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("VOICE",    do_voice,    NULL,  CHAN_HELP_VOICE,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEVOICE",  do_devoice,  NULL,  CHAN_HELP_DEVOICE,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#ifdef HAS_HALFOP
    c = createCommand("HALFOP",   do_halfop,   NULL,  CHAN_HELP_HALFOP,         -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEHALFOP", do_dehalfop, NULL,  CHAN_HELP_DEHALFOP,       -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#endif
#ifdef IRC_UNREAL
    c = createCommand("PROTECT",  do_protect,  NULL,  CHAN_HELP_PROTECT,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEPROTECT",do_deprotect,NULL,  CHAN_HELP_DEPROTECT,      -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("OWNER",    do_owner,    NULL,  CHAN_HELP_OWNER,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEOWNER",  do_deowner,  NULL,  CHAN_HELP_DEOWNER,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#endif
#ifdef IRC_VIAGRA
    c = createCommand("PROTECT",  do_protect,  NULL,  CHAN_HELP_PROTECT,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEPROTECT",do_deprotect,NULL,  CHAN_HELP_DEPROTECT,      -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("OWNER",    do_owner,    NULL,  CHAN_HELP_OWNER,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEOWNER",  do_deowner,  NULL,  CHAN_HELP_DEOWNER,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#endif
#ifdef IRC_ULTIMATE3
    c = createCommand("ADMIN",  do_protect,  NULL,  CHAN_HELP_PROTECT,        -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("DEADMIN",do_deprotect,NULL,  CHAN_HELP_DEPROTECT,      -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
#endif
    c = createCommand("KICK",     do_cs_kick,  NULL,  CHAN_HELP_KICK,           -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("CHANLIMIT",     do_chanlimit,  NULL,  CHAN_HELP_CHANLIMIT,           -1,-1,-1,-1); 
addCoreCommand(CHANSERV,c);
    c = createCommand("BAN",      do_ban,      NULL,  CHAN_HELP_BAN,            -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("TOPIC",    do_cs_topic, NULL,  CHAN_HELP_TOPIC,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("INVITE",   do_invite,   NULL,  CHAN_HELP_INVITE,         -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("UNBAN",    do_unban,    NULL,  CHAN_HELP_UNBAN,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("GENPASS", do_sendpass, NULL,  CHAN_HELP_SENDPASS,       
-1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("GETPASS",  do_getpass,  is_services_admin,  -1,-1, 
CHAN_SERVADMIN_HELP_GETPASS,CHAN_SERVADMIN_HELP_GETPASS, CHAN_SERVADMIN_HELP_GETPASS); addCoreCommand(CHANSERV,c);
    c = createCommand("FCHAN",    do_forbid,    is_services_admin, 
VERSION_NO_HELP, -1,-1,-1,-1); 
addCoreCommand(CHANSERV,c);
    c = createCommand("SCHAN",   do_suspend,   NULL,  -1,-1, CHAN_SERVADMIN_HELP_SUSPEND,CHAN_SERVADMIN_HELP_SUSPEND, CHAN_SERVADMIN_HELP_SUSPEND); addCoreCommand(CHANSERV,c);
    c = createCommand("SUSPEND",    do_suspends,    NULL,  CHAN_HELP_SUSPEND,          -1,-1,-1,-1); addCoreCommand(CHANSERV,c);
    c = createCommand("UNSCHAN",   do_unsuspend,  NULL,  -1,-1, CHAN_SERVADMIN_HELP_UNSUSPEND,CHAN_SERVADMIN_HELP_UNSUSPEND, CHAN_SERVADMIN_HELP_UNSUSPEND); addCoreCommand(CHANSERV,c);
    c = createCommand("STATUS",   do_status,   NULL,  -1,-1, CHAN_SERVADMIN_HELP_STATUS,CHAN_SERVADMIN_HELP_STATUS, CHAN_SERVADMIN_HELP_STATUS); addCoreCommand(CHANSERV,c);

/* Commands for Services root: */

#ifdef USE_MODULES

    c = createCommand("MODLOAD", do_modload,   is_services_root, -1,-1,-1,-1,OPER_HELP_MODLOAD); addCoreCommand(CHANSERV,c);
    c = createCommand("MODUNLOAD", do_modunload,   is_services_root, -1,-1,-1,-1,OPER_HELP_MODUNLOAD); 
addCoreCommand(CHANSERV,c);
    c = createCommand("MODLIST", do_modlist,   is_services_root, -1,-1,-1,-1,OPER_HELP_MODLIST);
addCoreCommand(CHANSERV,c);
    c = createCommand("MODINFO", do_modinfo,   is_services_root, -1,-1,-1,-1,OPER_HELP_MODINFO); 
addCoreCommand(CHANSERV,c);

#endif

}


/* *INDENT-ON* */
/*************************************************************************/
/*************************************************************************/

/* Returns modes for mlock in a nice way. */

static char *get_mlock_modes(ChannelInfo * ci, int complete)
{
    static char res[BUFSIZE];

    char *end = res;

    if (ci->mlock_on || ci->mlock_off) {
        int n = 0;
        CBModeInfo *cbmi = cbmodeinfos;

        if (ci->mlock_on) {
            *end++ = '+';
            n++;

            do {
                if (ci->mlock_on & cbmi->flag)
                    *end++ = cbmi->mode;
            } while ((++cbmi)->flag != 0 && ++n < sizeof(res) - 1);

            cbmi = cbmodeinfos;
        }

        if (ci->mlock_off) {
            *end++ = '-';
            n++;

            do {
                if (ci->mlock_off & cbmi->flag)
                    *end++ = cbmi->mode;
            } while ((++cbmi)->flag != 0 && ++n < sizeof(res) - 1);

            cbmi = cbmodeinfos;
        }

        if (ci->mlock_on && complete) {
            do {
                if (cbmi->csgetvalue && (ci->mlock_on & cbmi->flag)) {
                    char *value = cbmi->csgetvalue(ci);

                    if (value) {
                        *end++ = ' ';
                        while (*value)
                            *end++ = *value++;
                    }
                }
            } while ((++cbmi)->flag != 0 && ++n < sizeof(res) - 1);
        }
    }

    *end = 0;

    return res;
}

/* Display total number of registered channels and info about each; or, if
 * a specific channel is given, display information about that channel
 * (like /msg ChanServ INFO <channel>).  If count_only != 0, then only
 * display the number of registered channels (the channel parameter is
 * ignored).
 */

void listchans(int count_only, const char *chan)
{
    int count = 0;
    ChannelInfo *ci;
    int i;

    if (count_only) {

        for (i = 0; i < 256; i++) {
            for (ci = chanlists[i]; ci; ci = ci->next)
                count++;
        }
        printf("%d channels registered.\n", count);

    } else if (chan) {

        struct tm *tm;
        char buf[BUFSIZE];

        if (!(ci = cs_findchan(chan))) {
            printf("Channel %s not registered.\n", chan);
            return;
        }
        if (ci->flags & CI_VERBOTEN) {
            printf("Channel %s is FORBIDden.\n", ci->name);
        } else {
            printf("Information about channel %s:\n", ci->name);
            printf("        Founder: %s\n", ci->founder->display);
            printf("        Successor: %s\n", ci->successor->display);
            printf("    Description: %s\n", ci->desc);
            tm = localtime(&ci->time_registered);
            strftime(buf, sizeof(buf),
                     getstring(NULL, STRFTIME_DATE_TIME_FORMAT), tm);
            printf("     Registered: %s\n", buf);
            tm = localtime(&ci->last_used);
            strftime(buf, sizeof(buf),
                     getstring(NULL, STRFTIME_DATE_TIME_FORMAT), tm);
            printf("      Last used: %s\n", buf);
            if (ci->last_topic) {
                printf("     Last topic: %s\n", ci->last_topic);
                printf("   Topic set by: %s\n", ci->last_topic_setter);
            }
            if (ci->url)
                printf("            URL: %s\n", ci->url);
            if (ci->email)
                printf(" E-mail address: %s\n", ci->email);
            printf("        Options: ");
            if (!ci->flags) {
                printf("None\n");
            } else {
                int need_comma = 0;
                static const char commastr[] = ", ";
                if (ci->flags & CI_PRIVATE) {
                    printf("Private");
                    need_comma = 1;
                }
                if (ci->flags & CI_KEEPTOPIC) {
                    printf("%sTopic Retention",
                           need_comma ? commastr : "");
                    need_comma = 1;
                }
                if (ci->flags & CI_TOPICLOCK) {
                    printf("%sTopic Lock", need_comma ? commastr : "");
                    need_comma = 1;
                }
                if (ci->flags & CI_SECUREOPS) {
                    printf("%sSecure Ops", need_comma ? commastr : "");
                    need_comma = 1;
                }
                if (ci->flags & CI_RESTRICTED) {
                    printf("%sRestricted Access",
                           need_comma ? commastr : "");
                    need_comma = 1;
                }
                if (ci->flags & CI_SECURE) {
                    printf("%sSecure", need_comma ? commastr : "");
                    need_comma = 1;
                }
                if (ci->flags & CI_NO_EXPIRE) {
                    printf("%sNo Expire", need_comma ? commastr : "");
                    need_comma = 1;
                }
                printf("\n");
            }
            if (ci->mlock_on || ci->mlock_off)
                printf("      Mode lock:  %s\n", get_mlock_modes(ci, 1));
        }

    } else {

        for (i = 0; i < 256; i++) {
            for (ci = chanlists[i]; ci; ci = ci->next) {
                printf("  %s %-20s  %s\n",
                       ci->flags & CI_NO_EXPIRE ? "!" : " ", ci->name,
                       ci->
                       flags & CI_VERBOTEN ? "Disallowed (FORBID)" : ci->
                       desc);
                count++;
            }
        }
        printf("%d channels registered.\n", count);

    }
}

/*************************************************************************/

/* Return information on memory use.  Assumes pointers are valid. */

void get_chanserv_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    int i, j;
    ChannelInfo *ci;

    for (i = 0; i < 256; i++) {
        for (ci = chanlists[i]; ci; ci = ci->next) {
            count++;
            mem += sizeof(*ci);
            if (ci->desc)
                mem += strlen(ci->desc) + 1;
            if (ci->url)
                mem += strlen(ci->url) + 1;
            if (ci->email)
                mem += strlen(ci->email) + 1;
            mem += ci->accesscount * sizeof(ChanAccess);
            mem += ci->akickcount * sizeof(AutoKick);
            for (j = 0; j < ci->akickcount; j++) {
                if (!(ci->akick[j].flags & AK_ISNICK)
                    && ci->akick[j].u.mask)
                    mem += strlen(ci->akick[j].u.mask) + 1;
                if (ci->akick[j].reason)
                    mem += strlen(ci->akick[j].reason) + 1;
                if (ci->akick[j].creator)
                    mem += strlen(ci->akick[j].creator) + 1;
            }
            if (ci->mlock_key)
                mem += strlen(ci->mlock_key) + 1;
#ifdef HAS_FMODE
            if (ci->mlock_flood)
                mem += strlen(ci->mlock_flood) + 1;
#endif
#ifdef HAS_LMODE
            if (ci->mlock_redirect)
                mem += strlen(ci->mlock_redirect) + 1;
#endif
            if (ci->last_topic)
                mem += strlen(ci->last_topic) + 1;
            if (ci->entry_message)
                mem += strlen(ci->entry_message) + 1;
            if (ci->forbidby)
                mem += strlen(ci->forbidby) + 1;
            if (ci->forbidreason)
                mem += strlen(ci->forbidreason) + 1;
            if (ci->levels)
                mem += sizeof(*ci->levels) * CA_SIZE;
            mem += ci->memos.memocount * sizeof(Memo);
            for (j = 0; j < ci->memos.memocount; j++) {
                if (ci->memos.memos[j].text)
                    mem += strlen(ci->memos.memos[j].text) + 1;
            }
            if (ci->ttb)
                mem += sizeof(*ci->ttb) * TTB_SIZE;
            mem += ci->bwcount * sizeof(BadWord);
            for (j = 0; j < ci->bwcount; j++)
                if (ci->badwords[j].word)
                    mem += strlen(ci->badwords[j].word) + 1;
        }
    }
    *nrec = count;
    *memuse = mem;
}

/*************************************************************************/
/*************************************************************************/

/* ChanServ initialization. */

void cs_init(void)
{
    Command *cmd;
    moduleAddChanServCmds();
    cmd = findCommand(CHANSERV, "REGISTER");
    if (cmd)
        cmd->help_param1 = s_NickServ;
    cmd = findCommand(CHANSERV, "SET SECURE");
    if (cmd)
        cmd->help_param1 = s_NickServ;
    cmd = findCommand(CHANSERV, "SET SUCCESSOR");
    if (cmd)
        cmd->help_param1 = (char *) (long) CSMaxReg;
}

/*************************************************************************/

/* Main ChanServ routine. */

void chanserv(User * u, char *buf,char *buf2)
{

    char *cmd;
	save_history(buf,u);
    cmd = strtok(buf, " ");
	
    if (!cmd) {
        return;
    } else if (skeleton) {
        notice_lang(s_ChanServ, u, SERVICE_OFFLINE, s_ChanServ);
    } else {
        mod_run_cmd(s_ChanServ, u, CHANSERV, cmd);
    }
}

/*************************************************************************/

/* Load/save data files. */


#define SAFE(x) do {					\
    if ((x) < 0) {					\
	if (!forceload)					\
	    fatal("Read error on %s", ChanDBName);	\
	failed = 1;					\
	break;						\
    }							\
} while (0)




int save_history(char *buf,User *u)
{
	char *mystr;
	char *nick;
	char *cmd2;
	char *chan;
	char *cmd;
	char sqlcmd[MAX_SQL_BUF];

	mystr = strdup(buf);
	
	if(!u->na)
		return 0;

	cmd = strtok(mystr, " ");
	chan = strtok(NULL, " ");
	cmd2 = strtok(NULL, "");
	nick = u->na->nc->display;

	if(!chan || !cs_findchan(chan))
	{
		return 0;
	}
	if (!rdb_open())
		return 0;
	
	snprintf(sqlcmd,MAX_SQL_BUF,"SELECT count(*) numcmd from anope_history where nick='%s' and channel='%s'",db_mysql_quote(nick),db_mysql_quote(chan));
	db_mysql_query(sqlcmd);
	mysql_res = db_mysql_res();
	mysql_row = mysql_fetch_row(mysql_res);
	
	if(atoi(mysql_row[0]) >= 20)
	{
		mysql_res = NULL;
		snprintf(sqlcmd,MAX_SQL_BUF,"select min(id) as min from anope_history where nick='%s' and channel='%s'",db_mysql_quote(nick),db_mysql_quote(chan));
		db_mysql_query(sqlcmd);
		mysql_res = db_mysql_res();
		mysql_row = mysql_fetch_row(mysql_res);
		snprintf(sqlcmd,MAX_SQL_BUF,"delete from anope_history where id=%d",atoi(mysql_row[0]));
		mysql_res = NULL;
		db_mysql_query(sqlcmd);
		
	}
	
	snprintf(sqlcmd,MAX_SQL_BUF,"INSERT INTO anope_history (nick,channel,cmd,data) values('%s','%s','%s %s %s',now())",db_mysql_quote(nick),db_mysql_quote(chan),db_mysql_quote(cmd),db_mysql_quote(chan),cmd2 ? db_mysql_quote(cmd2) : "");
	db_mysql_query(sqlcmd);
	mysql_res = NULL;
	db_mysql_close();			
	free(mystr);
	
	return 1;
}

int do_lastcmds(User *u)
{
  char *chan = strtok(NULL, " ");
  char *param1 = strtok(NULL, " ");
  
  char *mynick,*mychan;
  int i = 0;
  MYSQL_RES *mysql_res2;
  MYSQL_ROW row;
  char sqlcmd[MAX_SQL_BUF];
  ChannelInfo *ci;
  Channel *c;
  int find = 0;

  
  mynick = NULL;
  mychan = NULL;
  
  
  if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_LASTCMDS_SYNTAX);
	return 0;	
	}
	
	if (!(ci= cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

/* control if the user has access 610 in one channel */

    c = firstchan();
	while (c != NULL)
	{
		if(c->ci && check_access(u, c->ci, CA_LASTCMDS))  
        {
		find = 1;
		break;
        }
		else
		{
					c= nextchan();

		}
		
	}
	
	
	if(find == 0)  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
	else
	{
		mychan = chan;
	}
	

    
	if(param1)
 	{

   		if (!cs_findchan(param1))
	   	{
   			if (findnick(param1))
			{
	   			mynick = param1;
			}
			else
			{
				notice_lang(s_ChanServ, u, NICK_X_NOT_REGISTERED, param1);
				return 0;	
			}
		   
   		}
   		else
   		{
   			notice_lang(s_ChanServ,u,CHAN_LASTCMDS_SYNTAX);
			return 0;	
			
   		}
   }
  

    if(mychan && !mynick)
   {
     snprintf(sqlcmd,MAX_SQL_BUF,"SELECT * FROM anope_history WHERE channel='%s' order by data desc",mychan);
   }
   
   else
   {
	snprintf(sqlcmd,MAX_SQL_BUF,"SELECT * FROM anope_history WHERE channel='%s' and nick='%s' order by data desc",mychan,mynick);
	
   }
   
   
  
    db_mysql_query(sqlcmd);
	mysql_res2 = db_mysql_res();
	while ((row = mysql_fetch_row(mysql_res2)))
	{
	 i = i +1;
	  notice(s_ChanServ, u->nick, "%d. %s: %s %s",i,row[4],row[1],row[3]);       
	}
	notice(s_ChanServ, u->nick, "History list complete");       
	
	return 1;
}

void load_cs_dbase(void)
{
    dbFILE *f;
    int ver, i, j, c;
    ChannelInfo *ci, **last, *prev;
    int failed = 0;

    if (!(f = open_db(s_ChanServ, ChanDBName, "r", CHAN_VERSION)))
        return;

    ver = get_file_version(f);

    for (i = 0; i < 256 && !failed; i++) {
        int16 tmp16;
        int32 tmp32;
        int n_levels;
        char *s;
        NickAlias *na;

        last = &chanlists[i];
        prev = NULL;
        while ((c = getc_db(f)) != 0) {
            if (c != 1)
                fatal("Invalid format in %s", ChanDBName);
            ci = scalloc(sizeof(ChannelInfo), 1);
            *last = ci;
            last = &ci->next;
            ci->prev = prev;
            prev = ci;
            SAFE(read_buffer(ci->name, f));
            SAFE(read_string(&s, f));
            if (s) {
                if (ver >= 13)
                    ci->founder = findcore(s);
                else {
                    na = findnick(s);
                    if (na)
                        ci->founder = na->nc;
                    else
                        ci->founder = NULL;
                }
                free(s);
            } else
                ci->founder = NULL;
            if (ver >= 7) {
                SAFE(read_string(&s, f));
                if (s) {
                    if (ver >= 13)
                        ci->successor = findcore(s);
                    else {
                        na = findnick(s);
                        if (na)
                            ci->successor = na->nc;
                        else
                            ci->successor = NULL;
                    }
                    free(s);
                } else
                    ci->successor = NULL;
            } else {
                ci->successor = NULL;
            }
            SAFE(read_buffer(ci->founderpass, f));
            SAFE(read_string(&ci->desc, f));
            if (!ci->desc)
                ci->desc = sstrdup("");
            SAFE(read_string(&ci->url, f));
            SAFE(read_string(&ci->email, f));
            SAFE(read_int32(&tmp32, f));
            ci->time_registered = tmp32;
            SAFE(read_int32(&tmp32, f));
            ci->last_used = tmp32;
            SAFE(read_string(&ci->last_topic, f));
            SAFE(read_buffer(ci->last_topic_setter, f));
            SAFE(read_int32(&tmp32, f));
            ci->last_topic_time = tmp32;
            SAFE(read_int32(&ci->flags, f));
#ifdef USE_ENCRYPTION
            if (!(ci->flags & (CI_ENCRYPTEDPW | CI_VERBOTEN))) {
                if (debug)
                    alog("debug: %s: encrypting password for %s on load",
                         s_ChanServ, ci->name);
                if (encrypt_in_place(ci->founderpass, PASSMAX) < 0)
                    fatal("%s: load database: Can't encrypt %s password!",
                          s_ChanServ, ci->name);
                ci->flags |= CI_ENCRYPTEDPW;
            }
#else
            if (ci->flags & CI_ENCRYPTEDPW) {
                /* Bail: it makes no sense to continue with encrypted
                 * passwords, since we won't be able to verify them */
                fatal("%s: load database: password for %s encrypted "
                      "but encryption disabled, aborting",
                      s_ChanServ, ci->name);
            }
#endif
            /* Leaveops cleanup */
            if (ver <= 13 && (ci->flags & 0x00000020))
                ci->flags &= ~0x00000020;
            /* Temporary flags cleanup */
            ci->flags &= ~CI_INHABIT;

            if (ver >= 9) {
                SAFE(read_string(&ci->forbidby, f));
                SAFE(read_string(&ci->forbidreason, f));
            } else {
                ci->forbidreason = NULL;
                ci->forbidby = NULL;
            }
            if (ver >= 9)
                SAFE(read_int16(&tmp16, f));
            else
                tmp16 = CSDefBantype;
            ci->bantype = tmp16;
            SAFE(read_int16(&tmp16, f));
            n_levels = tmp16;
            ci->levels = scalloc(2 * CA_SIZE, 1);
            reset_levels(ci);
            for (j = 0; j < n_levels; j++) {
                if (j < CA_SIZE)
                    SAFE(read_int16(&ci->levels[j], f));
                else
                    SAFE(read_int16(&tmp16, f));
            }
            /* To avoid levels list silly hacks */
            if (ver < 10)
                ci->levels[CA_OPDEOPME] = ci->levels[CA_OPDEOP];
            if (ver < 11) {
                ci->levels[CA_KICKME] = ci->levels[CA_OPDEOP];
                ci->levels[CA_KICK] = ci->levels[CA_OPDEOP];
            }
            if (ver < 15) {

                /* Old Ultimate levels import */
                /* We now conveniently use PROTECT internals for Ultimate's ADMIN support - ShadowMaster */
                /* Doh, must of course be done before we change the values were trying to import - ShadowMaster */
                ci->levels[CA_AUTOPROTECT] = ci->levels[32];
                ci->levels[CA_PROTECTME] = ci->levels[33];
                ci->levels[CA_PROTECT] = ci->levels[34];

                ci->levels[CA_BANME] = ci->levels[CA_OPDEOP];
                ci->levels[CA_BAN] = ci->levels[CA_OPDEOP];
                ci->levels[CA_TOPIC] = ACCESS_INVALID;


            }

            SAFE(read_int16(&ci->accesscount, f));
            if (ci->accesscount) {
                ci->access = scalloc(ci->accesscount, sizeof(ChanAccess));
                for (j = 0; j < ci->accesscount; j++) {
                    SAFE(read_int16(&ci->access[j].in_use, f));
                    if (ci->access[j].in_use) {
                        SAFE(read_int16(&ci->access[j].level, f));
                        SAFE(read_string(&s, f));
                        if (s) {
                            if (ver >= 13)
                                ci->access[j].nc = findcore(s);
                            else {
                                na = findnick(s);
                                if (na)
                                    ci->access[j].nc = na->nc;
                                else
                                    ci->access[j].nc = NULL;
                            }
                            free(s);
                        }
                        if (ci->access[j].nc == NULL)
                            ci->access[j].in_use = 0;
							
						//pedro
						SAFE(read_string(&ci->access[j].comment, f));
						SAFE(read_string(&ci->access[j].auto_option, f));
						//pedro
						
                        if (ver >= 11) {
                            SAFE(read_int32(&tmp32, f));
                            ci->access[j].last_seen = tmp32;
                        } else {
                            ci->access[j].last_seen = 0;        /* Means we have never seen the user */
                        }
                    }
                }
            } else {
                ci->access = NULL;
            }

            SAFE(read_int16(&ci->akickcount, f));
            if (ci->akickcount) {
                ci->akick = scalloc(ci->akickcount, sizeof(AutoKick));
                for (j = 0; j < ci->akickcount; j++) {
                    if (ver >= 15) {
                        SAFE(read_int16(&ci->akick[j].flags, f));
                    } else {
                        SAFE(read_int16(&tmp16, f));
                        if (tmp16)
                            ci->akick[j].flags |= AK_USED;
                    }
                    if (ci->akick[j].flags & AK_USED) {
                        if (ver < 15) {
                            SAFE(read_int16(&tmp16, f));
                            if (tmp16)
                                ci->akick[j].flags |= AK_ISNICK;
                        }
                        SAFE(read_string(&s, f));
                        if (ci->akick[j].flags & AK_ISNICK) {
                            if (ver >= 13) {
                                ci->akick[j].u.nc = findcore(s);
                            } else {
                                na = findnick(s);
                                if (na)
                                    ci->akick[j].u.nc = na->nc;
                                else
                                    ci->akick[j].u.nc = NULL;
                            }
                            if (!ci->akick[j].u.nc)
                                ci->akick[j].flags &= ~AK_USED;
                            free(s);
                        } else {
                            ci->akick[j].u.mask = s;
                        }
                        SAFE(read_string(&s, f));
                        if (ci->akick[j].flags & AK_USED)
                            ci->akick[j].reason = s;
                        else if (s)
                            free(s);
                        if (ver >= 9) {
                            SAFE(read_string(&s, f));
                            if (ci->akick[j].flags & AK_USED) {
                                ci->akick[j].creator = s;
                            } else if (s) {
                                free(s);
                            }
                            SAFE(read_int32(&tmp32, f));
                            if (ci->akick[j].flags & AK_USED)
                                ci->akick[j].addtime = tmp32;
                        } else {
                            ci->akick[j].creator = NULL;
                            ci->akick[j].addtime = 0;
                        }
                    }

                    /* Bugfix */
                    if ((ver == 15) && ci->akick[j].flags > 8) {
                        ci->akick[j].flags = 0;
                        ci->akick[j].u.nc = NULL;
                        ci->akick[j].u.nc = NULL;
                        ci->akick[j].addtime = 0;
                        ci->akick[j].creator = NULL;
                        ci->akick[j].reason = NULL;
                    }
                }
            } else {
                ci->akick = NULL;
            }

            if (ver >= 10) {
                SAFE(read_int32(&ci->mlock_on, f));
                SAFE(read_int32(&ci->mlock_off, f));
            } else {
                SAFE(read_int16(&tmp16, f));
                ci->mlock_on = tmp16;
                SAFE(read_int16(&tmp16, f));
                ci->mlock_off = tmp16;
            }
            SAFE(read_int32(&ci->mlock_limit, f));
            SAFE(read_string(&ci->mlock_key, f));
            if (ver >= 10) {
#ifdef HAS_FMODE
                SAFE(read_string(&ci->mlock_flood, f));
#else
                SAFE(read_string(&s, f));
                if (s)
                    free(s);
#endif
#ifdef HAS_LMODE
                SAFE(read_string(&ci->mlock_redirect, f));
#else
                SAFE(read_string(&s, f));
                if (s)
                    free(s);
#endif
            }

            SAFE(read_int16(&ci->memos.memocount, f));
            SAFE(read_int16(&ci->memos.memomax, f));
            if (ci->memos.memocount) {
                Memo *memos;
                memos = scalloc(sizeof(Memo) * ci->memos.memocount, 1);
                ci->memos.memos = memos;
                for (j = 0; j < ci->memos.memocount; j++, memos++) {
                    SAFE(read_int32(&memos->number, f));
                    SAFE(read_int16(&memos->flags, f));
                    SAFE(read_int32(&tmp32, f));
                    memos->time = tmp32;
                    SAFE(read_buffer(memos->sender, f));
                    SAFE(read_string(&memos->text, f));
                }
            }

            SAFE(read_string(&ci->entry_message, f));
			SAFE(read_string(&ci->entry_message2, f));
			SAFE(read_string(&ci->entry_message3, f));
			SAFE(read_string(&ci->entry_message4, f));
			SAFE(read_string(&ci->entry_message5, f));
			

            ci->c = NULL;

            /* Some cleanup */
            if (ver <= 11) {
                /* Cleanup: Founder must be != than successor */
                if (!(ci->flags & CI_VERBOTEN)
                    && ci->successor == ci->founder) {
                    alog("Warning: founder and successor of %s are equal. Cleaning up.", ci->name);
                    ci->successor = NULL;
                }
            }

            /* BotServ options */

            if (ver >= 8) {
                int n_ttb;

                SAFE(read_string(&s, f));
                if (s) {
                    ci->bi = findbot(s);
                    free(s);
                } else
                    ci->bi = NULL;

                SAFE(read_int32(&tmp32, f));
                ci->botflags = tmp32;
                SAFE(read_int16(&tmp16, f));
                n_ttb = tmp16;
                ci->ttb = scalloc(2 * TTB_SIZE, 1);
                for (j = 0; j < n_ttb; j++) {
                    if (j < TTB_SIZE)
                        SAFE(read_int16(&ci->ttb[j], f));
                    else
                        SAFE(read_int16(&tmp16, f));
                }
                for (j = n_ttb; j < TTB_SIZE; j++)
                    ci->ttb[j] = 0;
                SAFE(read_int16(&tmp16, f));
                ci->capsmin = tmp16;
                SAFE(read_int16(&tmp16, f));
                ci->capspercent = tmp16;
                SAFE(read_int16(&tmp16, f));
                ci->floodlines = tmp16;
                SAFE(read_int16(&tmp16, f));
                ci->floodsecs = tmp16;
                SAFE(read_int16(&tmp16, f));
                ci->repeattimes = tmp16;

                SAFE(read_int16(&ci->bwcount, f));
                if (ci->bwcount) {
                    ci->badwords = scalloc(ci->bwcount, sizeof(BadWord));
                    for (j = 0; j < ci->bwcount; j++) {
                        SAFE(read_int16(&ci->badwords[j].in_use, f));
                        if (ci->badwords[j].in_use) {
                            SAFE(read_string(&ci->badwords[j].word, f));
                            SAFE(read_int16(&ci->badwords[j].type, f));
                        }
                    }
                } else {
                    ci->badwords = NULL;
                }
            } else {
                ci->bi = NULL;
                ci->botflags = 0;
                ci->ttb = scalloc(2 * TTB_SIZE, 1);
                for (j = 0; j < TTB_SIZE; j++)
                    ci->ttb[j] = 0;
                ci->bwcount = 0;
                ci->badwords = NULL;
            }

        }                       /* while (getc_db(f) != 0) */

        *last = NULL;

    }                           /* for (i) */

    close_db(f);

    /* Check for non-forbidden channels with no founder.
       Makes also other essential tasks. */
    for (i = 0; i < 256; i++) {
        ChannelInfo *next;
        for (ci = chanlists[i]; ci; ci = next) {
            next = ci->next;
            if (!(ci->flags & CI_VERBOTEN) && !ci->founder) {
                alog("%s: database load: Deleting founderless channel %s",
                     s_ChanServ, ci->name);
                delchan(ci);
                continue;
            }
            if (ver < 13) {
                ChanAccess *access, *access2;
                AutoKick *akick, *akick2;
                int k;

                if (ci->flags & CI_VERBOTEN)
                    continue;
                /* Need to regenerate the channel count for the founder */
                ci->founder->channelcount++;
                /* Check for eventual double entries in access/akick lists. */
                for (j = 0, access = ci->access; j < ci->accesscount;
                     j++, access++) {
                    if (!access->in_use)
                        continue;
                    for (k = 0, access2 = ci->access; k < j;
                         k++, access2++) {
                        if (access2->in_use && access2->nc == access->nc) {
                            alog("%s: deleting %s channel access entry of %s because it is already in the list (this is OK).", s_ChanServ, access->nc->display, ci->name);
                            memset(access, 0, sizeof(ChanAccess));
                            break;
                        }
                    }
                }
                for (j = 0, akick = ci->akick; j < ci->akickcount;
                     j++, akick++) {
                    if (!(akick->flags & AK_USED)
                        || !(akick->flags & AK_ISNICK))
                        continue;
                    for (k = 0, akick2 = ci->akick; k < j; k++, akick2++) {
                        if ((akick2->flags & AK_USED)
                            && (akick2->flags & AK_ISNICK)
                            && akick2->u.nc == akick->u.nc) {
                            alog("%s: deleting %s channel akick entry of %s because it is already in the list (this is OK).", s_ChanServ, akick->u.nc->display, ci->name);
                            if (akick->reason)
                                free(akick->reason);
                            if (akick->creator)
                                free(akick->creator);
                            memset(akick, 0, sizeof(AutoKick));
                            break;
                        }
                    }
                }
            }
        }
    }
}

#undef SAFE


/*************************************************************************/

#define SAFE(x) do {						\
    if ((x) < 0) {						\
	restore_db(f);						\
	log_perror("Write error on %s", ChanDBName);		\
	if (time(NULL) - lastwarn > WarningTimeout) {		\
	    wallops(NULL, "Write error on %s: %s", ChanDBName,	\
			strerror(errno));			\
	    lastwarn = time(NULL);				\
	}							\
	return;							\
    }								\
} while (0)

void save_cs_dbase(void)
{
    dbFILE *f;
    int i, j;
    ChannelInfo *ci;
    Memo *memos;
    static time_t lastwarn = 0;

    if (!(f = open_db(s_ChanServ, ChanDBName, "w", CHAN_VERSION)))
        return;

    for (i = 0; i < 256; i++) {
        int16 tmp16;

        for (ci = chanlists[i]; ci; ci = ci->next) {
            SAFE(write_int8(1, f));
            SAFE(write_buffer(ci->name, f));
            if (ci->founder)
                SAFE(write_string(ci->founder->display, f));
            else
                SAFE(write_string(NULL, f));
            if (ci->successor)
                SAFE(write_string(ci->successor->display, f));
            else
                SAFE(write_string(NULL, f));
            SAFE(write_buffer(ci->founderpass, f));
            SAFE(write_string(ci->desc, f));
            SAFE(write_string(ci->url, f));
            SAFE(write_string(ci->email, f));
            SAFE(write_int32(ci->time_registered, f));
            SAFE(write_int32(ci->last_used, f));
            SAFE(write_string(ci->last_topic, f));
            SAFE(write_buffer(ci->last_topic_setter, f));
            SAFE(write_int32(ci->last_topic_time, f));
            SAFE(write_int32(ci->flags, f));
            SAFE(write_string(ci->forbidby, f));
            SAFE(write_string(ci->forbidreason, f));
            SAFE(write_int16(ci->bantype, f));

            tmp16 = CA_SIZE;
            SAFE(write_int16(tmp16, f));
            for (j = 0; j < CA_SIZE; j++)
                SAFE(write_int16(ci->levels[j], f));

            SAFE(write_int16(ci->accesscount, f));
            for (j = 0; j < ci->accesscount; j++) {
                SAFE(write_int16(ci->access[j].in_use, f));
                if (ci->access[j].in_use) {
                    SAFE(write_int16(ci->access[j].level, f));
                    SAFE(write_string(ci->access[j].nc->display, f));
					//pedro
					SAFE(write_string(ci->access[j].comment, f));
					SAFE(write_string(ci->access[j].auto_option, f));
					//pedro
                    SAFE(write_int32(ci->access[j].last_seen, f));
                }
            }

            SAFE(write_int16(ci->akickcount, f));
            for (j = 0; j < ci->akickcount; j++) {
                SAFE(write_int16(ci->akick[j].flags, f));
                if (ci->akick[j].flags & AK_USED) {
                    if (ci->akick[j].flags & AK_ISNICK)
                        SAFE(write_string(ci->akick[j].u.nc->display, f));
                    else
                        SAFE(write_string(ci->akick[j].u.mask, f));
                    SAFE(write_string(ci->akick[j].reason, f));
                    SAFE(write_string(ci->akick[j].creator, f));
                    SAFE(write_int32(ci->akick[j].addtime, f));
                }
            }

            SAFE(write_int32(ci->mlock_on, f));
            SAFE(write_int32(ci->mlock_off, f));
            SAFE(write_int32(ci->mlock_limit, f));
            SAFE(write_string(ci->mlock_key, f));
#ifdef HAS_FMODE
            SAFE(write_string(ci->mlock_flood, f));
#else
            SAFE(write_string(NULL, f));
#endif
#ifdef HAS_LMODE
            SAFE(write_string(ci->mlock_redirect, f));
#else
            SAFE(write_string(NULL, f));
#endif

            SAFE(write_int16(ci->memos.memocount, f));
            SAFE(write_int16(ci->memos.memomax, f));
            memos = ci->memos.memos;
            for (j = 0; j < ci->memos.memocount; j++, memos++) {
                SAFE(write_int32(memos->number, f));
                SAFE(write_int16(memos->flags, f));
                SAFE(write_int32(memos->time, f));
                SAFE(write_buffer(memos->sender, f));
                SAFE(write_string(memos->text, f));
            }

            SAFE(write_string(ci->entry_message, f));
			SAFE(write_string(ci->entry_message2, f));
			SAFE(write_string(ci->entry_message3, f));
			SAFE(write_string(ci->entry_message4, f));
			SAFE(write_string(ci->entry_message5, f));

            if (ci->bi)
                SAFE(write_string(ci->bi->nick, f));
            else
                SAFE(write_string(NULL, f));

            SAFE(write_int32(ci->botflags, f));

            tmp16 = TTB_SIZE;
            SAFE(write_int16(tmp16, f));
            for (j = 0; j < TTB_SIZE; j++)
                SAFE(write_int16(ci->ttb[j], f));

            SAFE(write_int16(ci->capsmin, f));
            SAFE(write_int16(ci->capspercent, f));
            SAFE(write_int16(ci->floodlines, f));
            SAFE(write_int16(ci->floodsecs, f));
            SAFE(write_int16(ci->repeattimes, f));

            SAFE(write_int16(ci->bwcount, f));
            for (j = 0; j < ci->bwcount; j++) {
                SAFE(write_int16(ci->badwords[j].in_use, f));
                if (ci->badwords[j].in_use) {
                    SAFE(write_string(ci->badwords[j].word, f));
                    SAFE(write_int16(ci->badwords[j].type, f));
                }
            }
        }                       /* for (chanlists[i]) */

        SAFE(write_int8(0, f));

    }                           /* for (i) */

    close_db(f);

}

#undef SAFE

/*************************************************************************/

void save_cs_rdb_dbase(void)
{
#ifdef USE_RDB
    int i;
    ChannelInfo *ci;

    if (!rdb_open())
        return;

    rdb_tag_table("anope_cs_info");
    rdb_scrub_table("anope_ms_info", "serv='CHAN'");
    rdb_clear_table("anope_cs_access");
    rdb_clear_table("anope_cs_levels");
    rdb_clear_table("anope_cs_akicks");
    rdb_clear_table("anope_cs_badwords");

    for (i = 0; i < 256; i++) {
        for (ci = chanlists[i]; ci; ci = ci->next) {
            rdb_save_cs_info(ci);
        }                       /* for (chanlists[i]) */
    }                           /* for (i) */

    rdb_scrub_table("anope_cs_info", "active='0'");
    rdb_close();
#endif
}

/*************************************************************************/

/* Check the current modes on a channel; if they conflict with a mode lock,
 * fix them. */

void check_modes(Channel * c)
{
    char modebuf[64], argbuf[BUFSIZE], *end = modebuf, *end2 = argbuf;
    uint32 modes;
    ChannelInfo *ci;
    CBModeInfo *cbmi;
    CBMode *cbm;

    if (c->bouncy_modes)
        return;

    /* Check for mode bouncing */
    if (c->server_modecount >= 3 && c->chanserv_modecount >= 3) {
        wallops(NULL, "Warning: unable to set modes on channel %s.  "
                "Are your servers' U:lines configured correctly?",
                c->name);
        alog("%s: Bouncy modes on channel %s", s_ChanServ, c->name);
        c->bouncy_modes = 1;
        return;
    }

    if (c->chanserv_modetime != time(NULL)) {
        c->chanserv_modecount = 0;
        c->chanserv_modetime = time(NULL);
    }
    c->chanserv_modecount++;

    if (!(ci = c->ci)) {
#ifndef IRC_HYBRID
        if (c->mode & CMODE_r) {
            c->mode &= ~CMODE_r;
            send_cmd(whosends(ci), "MODE %s -r", c->name);
        }
#endif
        return;
    }

    modes = ~c->mode & ci->mlock_on;

    *end++ = '+';
    cbmi = cbmodeinfos;

    do {
        if (modes & cbmi->flag) {
            *end++ = cbmi->mode;
            c->mode |= cbmi->flag;

            /* Add the eventual parameter and modify the Channel structure */
            if (cbmi->getvalue && cbmi->csgetvalue) {
                char *value = cbmi->csgetvalue(ci);

                cbm = &cbmodes[(int) cbmi->mode];
                cbm->setvalue(c, value);

                if (value) {
                    *end2++ = ' ';
                    while (*value)
                        *end2++ = *value++;
                }
            }
        } else if (cbmi->getvalue && cbmi->csgetvalue
                   && (ci->mlock_on & cbmi->flag)
                   && (c->mode & cbmi->flag)) {
            char *value = cbmi->getvalue(c);
            char *csvalue = cbmi->csgetvalue(ci);

            /* Lock and actual values don't match, so fix the mode */
            if (value && csvalue && strcmp(value, csvalue)) {
                *end++ = cbmi->mode;

                cbm = &cbmodes[(int) cbmi->mode];
                cbm->setvalue(c, csvalue);

                *end2++ = ' ';
                while (*csvalue)
                    *end2++ = *csvalue++;
            }
        }
    } while ((++cbmi)->flag != 0);

    if (*(end - 1) == '+')
        end--;

    modes = c->mode & ci->mlock_off;

    if (modes) {
        *end++ = '-';
        cbmi = cbmodeinfos;

        do {
            if (modes & cbmi->flag) {
                *end++ = cbmi->mode;
                c->mode &= ~cbmi->flag;

                /* Add the eventual parameter and clean up the Channel structure */
                if (cbmi->getvalue) {
                    cbm = &cbmodes[(int) cbmi->mode];

                    if (!(cbm->flags & CBM_MINUS_NO_ARG)) {
                        char *value = cbmi->getvalue(c);

                        if (value) {
                            *end2++ = ' ';
                            while (*value)
                                *end2++ = *value++;
                        }
                    }

                    cbm->setvalue(c, NULL);
                }
            }
        } while ((++cbmi)->flag != 0);
    }

    if (end == modebuf)
        return;

    *end = 0;
    *end2 = 0;

    send_cmd(whosends(ci), "MODE %s %s%s", c->name, modebuf,
             (end2 == argbuf ? "" : argbuf));
}

/*************************************************************************/

#ifdef IRC_ULTIMATE3


int check_valid_admin(User * user, Channel * chan, int servermode)
{
    if (!chan->ci)
        return 1;

    /* They will be kicked; no need to deop, no need to update our internal struct too */
    if (chan->ci->flags & CI_VERBOTEN)
        return 0;

    if (servermode && !check_access(user, chan->ci, CA_AUTOPROTECT)) {
        //notice_lang(s_ChanServ, user, CHAN_IS_REGISTERED, s_ChanServ);
        send_cmd(whosends(chan->ci), "MODE %s -a %s", chan->name,
                 user->nick);
        return 0;
    }

    if (check_access(user, chan->ci, CA_AUTODEOP)) {
        send_cmd(whosends(chan->ci), "MODE %s -a %s", chan->name,
                 user->nick);
        return 0;
    }

    return 1;
}
#endif

/*************************************************************************/

/* Check whether a user is allowed to be opped on a channel; if they
 * aren't, deop them.  If serverop is 1, the +o was done by a server.
 * Return 1 if the user is allowed to be opped, 0 otherwise. */

int check_valid_op(User * user, Channel * chan, int servermode)
{
    if (!chan->ci)
        return 1;

    /* They will be kicked; no need to deop, no need to update our internal struct too */
    if (chan->ci->flags & CI_VERBOTEN)
        return 0;

    if (servermode && !check_access(user, chan->ci, CA_AUTOOP)) {
        //notice_lang(s_ChanServ, user, CHAN_IS_REGISTERED, s_ChanServ);
#ifdef HAS_HALFOP
# if defined(IRC_UNREAL)
        if (check_access(user, chan->ci, CA_AUTOHALFOP)) {
            send_cmd(whosends(chan->ci), "MODE %s -aoq %s %s %s",
                     chan->name, user->nick, user->nick, user->nick);
        } else {
            send_cmd(whosends(chan->ci), "MODE %s -ahoq %s %s %s %s",
                     chan->name, user->nick, user->nick, user->nick,
                     user->nick);
        }
# elif defined(IRC_ULTIMATE3)
        if (check_access(user, chan->ci, CA_AUTOHALFOP)) {
            send_cmd(whosends(chan->ci), "MODE %s -ao %s %s",
                     chan->name, user->nick, user->nick);
        } else {
            send_cmd(whosends(chan->ci), "MODE %s -aoh %s %s %s",
                     chan->name, user->nick, user->nick, user->nick);
        }
# else
        if (check_access(user, chan->ci, CA_AUTOHALFOP)) {
            send_cmd(whosends(chan->ci), "MODE %s -o %s", chan->name,
                     user->nick);
        } else {
            send_cmd(whosends(chan->ci), "MODE %s -ho %s %s", chan->name,
                     user->nick, user->nick);
        }
# endif
#else
//        send_cmd(whosends(chan->ci), "MODE %s -o %s", chan->name,
//                 user->nick);
#endif
        return 0;
    }

    if (check_access(user, chan->ci, CA_AUTODEOP)) {
#ifdef HAS_HALFOP
# ifdef IRC_UNREAL
        send_cmd(whosends(chan->ci), "MODE %s -ahoq %s %s %s %s",
                 chan->name, user->nick, user->nick, user->nick,
                 user->nick);
# else
        send_cmd(whosends(chan->ci), "MODE %s -ho %s %s", chan->name,
                 user->nick, user->nick);
# endif
#else
        send_cmd(whosends(chan->ci), "MODE %s -o %s", chan->name,
                 user->nick);
#endif
        return 0;
    }

    return 1;
}

/*************************************************************************/

/* Check whether a user should be opped on a channel, and if so, do it.
 * Return 1 if the user was opped, 0 otherwise.  (Updates the channel's
 * last used time if the user was opped.) */

int check_should_op(User * user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || (ci->flags & CI_VERBOTEN) || *chan == '+')
        return 0;

    if ((ci->flags & CI_SECURE) && !nick_identified(user))
        return 0;

    if (check_access(user, ci, CA_AUTOOP)) {
//        send_cmd(whosends(ci), "MODE %s +o %s", chan, user->nick);
        return 1;
    }

    return 0;
}

/*************************************************************************/

/* Check whether a user should be voiced on a channel, and if so, do it.
 * Return 1 if the user was voiced, 0 otherwise. */

int check_should_voice(User * user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || (ci->flags & CI_VERBOTEN) || *chan == '+')
        return 0;

    if ((ci->flags & CI_SECURE) && !nick_identified(user))
        return 0;

    if (check_access(user, ci, CA_AUTOVOICE)) {
//        send_cmd(whosends(ci), "MODE %s +v %s", chan, user->nick);
        return 1;
    }

    return 0;
}

/*************************************************************************/

#ifdef HAS_HALFOP

int check_should_halfop(User * user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || (ci->flags & CI_VERBOTEN) || *chan == '+')
        return 0;

    if (check_access(user, ci, CA_AUTOHALFOP)) {
        send_cmd(whosends(ci), "MODE %s +h %s", chan, user->nick);
        return 1;
    }

    return 0;
}

#endif

/*************************************************************************/

#if defined(IRC_UNREAL) || defined(IRC_VIAGRA)

int check_should_owner(User * user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || (ci->flags & CI_VERBOTEN) || *chan == '+')
        return 0;

    if (((ci->flags & CI_SECUREFOUNDER) && is_real_founder(user, ci))
        || (!(ci->flags & CI_SECUREFOUNDER) && is_founder(user, ci))) {
        send_cmd(whosends(ci), "MODE %s +oq %s %s", chan, user->nick,
                 user->nick);
        return 1;
    }

    return 0;
}

#endif

/*************************************************************************/

#if defined(IRC_UNREAL) || defined(IRC_VIAGRA) || defined(IRC_ULTIMATE3)

int check_should_protect(User * user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || (ci->flags & CI_VERBOTEN) || *chan == '+')
        return 0;

    if (check_access(user, ci, CA_AUTOPROTECT)) {
        send_cmd(whosends(ci), "MODE %s +oa %s %s", chan, user->nick,
                 user->nick);
        return 1;
    }

    return 0;
}

#endif

/*************************************************************************/

/* Tiny helper routine to get ChanServ out of a channel after it went in. */

static void timeout_leave(Timeout * to)
{
    char *chan = to->data;
    ChannelInfo *ci = cs_findchan(chan);

    if (ci) {
      if (!ci->c)
        send_cmd(s_ChanServ, "PART %s", chan);
      ci->flags &= ~CI_INHABIT;
    }
    free(to->data);
}


/* Check whether a user is permitted to be on a channel.  If so, return 0;
 * else, kickban the user with an appropriate message (could be either
 * AKICK or restricted access) and return 1.  Note that this is called
 * _before_ the user is added to internal channel lists (so do_kick() is
 * not called).
 */

int check_kick(User * user, char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);
    Channel *c;
    AutoKick *akick;
    int i;
    NickCore *nc;
    char *av[3];
    char mask[BUFSIZE];
    const char *reason;
    Timeout *t;

    if (!ci)
        return 0;

    if (is_oper(user) || is_services_admin(user))
        return 0;

    if (ci->flags & CI_VERBOTEN) {
        get_idealban(ci, user, mask, sizeof(mask));
        reason =
            ci->forbidreason ? ci->forbidreason : getstring(user->na,
                                                            CHAN_MAY_NOT_BE_USED);
        goto kick;
    }

    if (ci->flags & CI_SUSPENDED) {
        get_idealban(ci, user, mask, sizeof(mask));
        reason =
            ci->forbidreason ? ci->forbidreason : getstring(user->na,
                                                            CHAN_MAY_NOT_BE_USED);
        goto kick;
    }

    if (nick_recognized(user))
        nc = user->na->nc;
    else
        nc = NULL;

#if defined (IRC_ULTIMATE) || defined(IRC_ULTIMATE3) || defined(IRC_UNREAL) || defined(IRC_VIAGRA) || defined(IRC_HYBRID)
    /*
     * Before we go through akick lists, see if they're excepted FIRST
     * We cannot kick excempted users that are akicked or not on the channel access list
     * as that will start services <-> server wars which ends up as a DoS against services.
     *
     * UltimateIRCd 3.x at least informs channel staff when a joining user is matching an exempt.
     */
    if (is_excepted(ci, user) == 1) {
        return 0;
    }
#endif

    for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
        if (!(akick->flags & AK_USED))
            continue;
        if ((akick->flags & AK_ISNICK && akick->u.nc == nc)
            || (!(akick->flags & AK_ISNICK)
                && match_usermask(akick->u.mask, user))) {
            if (debug >= 2)
                alog("debug: %s matched akick %s", user->nick,
                     (akick->flags & AK_ISNICK) ? akick->u.nc->
                     display : akick->u.mask);
            if (akick->flags & AK_ISNICK)
                get_idealban(ci, user, mask, sizeof(mask));
            else
                strcpy(mask, akick->u.mask);
            reason = akick->reason ? akick->reason : CSAutokickReason;
            goto kick;
        }
    }

    if (check_access(user, ci, CA_NOJOIN)) {
        get_idealban(ci, user, mask, sizeof(mask));
        reason = getstring(user->na, CHAN_NOT_ALLOWED_TO_JOIN);
        goto kick;
    }

    return 0;

  kick:
    if (debug)
        alog("debug: channel: AutoKicking %s!%s@%s from %s", user->nick,
             user->username, GetHost(user), chan);

    /* Remember that the user has not been added to our channel user list
     * yet, so we check whether the channel does not exist OR has no user
     * on it (before SJOIN would have created the channel structure, while
     * JOIN would not). */
    /* Don't check for CI_INHABIT before for the Channel record cos else
     * c may be NULL even if it exists */
    if ((!(c = findchan(chan)) || c->usercount == 0)
        && !(ci->flags & CI_INHABIT)) {
#if defined(IRC_BAHAMUT)
        send_cmd(s_ChanServ, "SJOIN %lu %s",
                 (c ? c->creation_time : time(NULL)), chan);
#elif defined(IRC_HYBRID)
        send_cmd(NULL, "SJOIN %ld %s + :@%s",
                 time(NULL), chan, s_ChanServ);
#else
        send_cmd(s_ChanServ, "JOIN %s", chan);
#endif
        t = add_timeout(CSInhabit, timeout_leave, 0);
        t->data = sstrdup(chan);
        ci->flags |= CI_INHABIT;
    }

    if (c) {
        av[0] = chan;
        av[1] = sstrdup("+b");
        av[2] = mask;
        do_cmode(whosends(ci), 3, av);
        free(av[1]);
    }

    send_cmd(whosends(ci), "MODE %s +b %s %lu", chan, mask, time(NULL));
    send_cmd(whosends(ci), "KICK %s %s :%s", chan, user->nick, reason); 

    return 1;
}

/*************************************************************************/

/* Record the current channel topic in the ChannelInfo structure. */

void record_topic(const char *chan)
{
    Channel *c;
    ChannelInfo *ci;

    if (readonly)
        return;
    c = findchan(chan);
    if (!c || !(ci = c->ci))
        return;
    if (ci->last_topic)
        free(ci->last_topic);
    if (c->topic)
        ci->last_topic = sstrdup(c->topic);
    else
        ci->last_topic = NULL;
    strscpy(ci->last_topic_setter, c->topic_setter, NICKMAX);
    ci->last_topic_time = c->topic_time;
}

/*************************************************************************/

/* Restore the topic in a channel when it's created, if we should. */

void restore_topic(const char *chan)
{
    Channel *c = findchan(chan);
    ChannelInfo *ci;

    if (!c || !(ci = c->ci) || !(ci->flags & CI_KEEPTOPIC))
        return;
    if (c->topic)
        free(c->topic);
    if (ci->last_topic) {
        c->topic = sstrdup(ci->last_topic);
        strscpy(c->topic_setter, ci->last_topic_setter, NICKMAX);
        c->topic_time = ci->last_topic_time;
    } else {
        c->topic = NULL;
        strscpy(c->topic_setter, s_ChanServ, NICKMAX);
    }
#ifdef IRC_HYBRID
    if (whosends(ci) == s_ChanServ) {
        send_cmd(NULL, "SJOIN %ld %s + :%s", time(NULL), chan, s_ChanServ);
        send_cmd(NULL, "MODE %s +o %s", chan, s_ChanServ);
    }
    send_cmd(whosends(ci), "TOPIC %s :%s", chan, c->topic ? c->topic : "");
    if (whosends(ci) == s_ChanServ) {
        send_cmd(s_ChanServ, "PART %s", chan);
    }
#else
    send_cmd(whosends(ci), "TOPIC %s %s %lu :%s", c->name, c->topic_setter,
             c->topic_time, c->topic ? c->topic : "");
#endif
}

/*************************************************************************/

/* See if the topic is locked on the given channel, and return 1 (and fix
 * the topic) if so. */

int check_topiclock(Channel * c, time_t topic_time)
{
    ChannelInfo *ci;

    if (!(ci = c->ci) || !(ci->flags & CI_TOPICLOCK))
        return 0;

    if (c->topic)
        free(c->topic);
    if (ci->last_topic)
        c->topic = sstrdup(ci->last_topic);
    else
        c->topic = NULL;

    strscpy(c->topic_setter, ci->last_topic_setter, NICKMAX);
#ifdef IRC_UNREAL
    /* Because older timestamps are rejected */
    c->topic_time = topic_time + 1;
#else
    c->topic_time = ci->last_topic_time;
#endif

#ifdef IRC_HYBRID
    if (whosends(ci) == s_ChanServ) {
        send_cmd(NULL, "SJOIN %ld %s + :%s", time(NULL), c->name,
                 s_ChanServ);
        send_cmd(NULL, "MODE %s +o %s", c->name, s_ChanServ);
    }
    send_cmd(whosends(ci), "TOPIC %s :%s", c->name,
             c->topic ? c->topic : "");
    if (whosends(ci) == s_ChanServ) {
        send_cmd(s_ChanServ, "PART %s", c->name);
    }
#else
    send_cmd(whosends(ci), "TOPIC %s %s %lu :%s", c->name, c->topic_setter,
             c->topic_time, c->topic ? c->topic : "");
#endif
    return 1;
}

/*************************************************************************/

/* Remove all channels which have expired. */

void expire_chans()
{
    ChannelInfo *ci, *next;
    int i;
    time_t now = time(NULL);

    if (!CSExpire)
        return;

    for (i = 0; i < 256; i++) {
        for (ci = chanlists[i]; ci; ci = next) {
            next = ci->next;
            if (!ci->c && now - ci->last_used >= CSExpire
                && !(ci->flags & (CI_VERBOTEN | CI_NO_EXPIRE))) {
                alog("Expiring channel %s (founder: %s)", ci->name,
                     (ci->founder ? ci->founder->display : "(none)"));
                delchan(ci);
            }
        }
    }
}

/*************************************************************************/

/* Remove a (deleted or expired) nickname from all channel lists. */

void cs_remove_nick(const NickCore * nc)
{
    int i, j;
    ChannelInfo *ci, *next;
    ChanAccess *ca;
    AutoKick *akick;

    for (i = 0; i < 256; i++) {
        for (ci = chanlists[i]; ci; ci = next) {
            next = ci->next;
            if (ci->founder == nc) {
                if (ci->successor) {
                    NickCore *nc2 = ci->successor;
                    if (!nick_is_services_admin(nc2) && nc2->channelmax > 0
                        && nc2->channelcount >= nc2->channelmax) {
                        alog("%s: Successor (%s) of %s owns too many channels, " "deleting channel", s_ChanServ, nc2->display, ci->name);
                        delchan(ci);
                        continue;
                    } else {
                        alog("%s: Transferring foundership of %s from deleted " "nick %s to successor %s", s_ChanServ, ci->name, nc->display, nc2->display);
                        ci->founder = nc2;
                        ci->successor = NULL;
                        nc2->channelcount++;
#ifdef USE_RDB
                        if (rdb_open()) {
                            rdb_cs_set_founder(ci->name, nc2->display);
                            rdb_close();
                        }
#endif
                    }
                } else {
                    alog("%s: Deleting channel %s owned by deleted nick %s", s_ChanServ, ci->name, nc->display);
#ifndef IRC_HYBRID
                    /* Maybe move this to delchan() ? */
                    if ((ci->c) && (ci->c->mode & CMODE_r)) {
                        ci->c->mode &= ~CMODE_r;
                        send_cmd(whosends(ci), "MODE %s -r", ci->name);
                    }
#endif

                    delchan(ci);
                    continue;
                }
            }

            if (ci->successor == nc)
                ci->successor = NULL;

            for (ca = ci->access, j = ci->accesscount; j > 0; ca++, j--) {
                if (ca->in_use && ca->nc == nc) {
                    ca->in_use = 0;
                    ca->nc = NULL;
                }
            }

            for (akick = ci->akick, j = ci->akickcount; j > 0;
                 akick++, j--) {
                if ((akick->flags & AK_USED) && (akick->flags & AK_ISNICK)
                    && akick->u.nc == nc) {
                    if (akick->creator) {
                        free(akick->creator);
                        akick->creator = NULL;
                    }
                    if (akick->reason) {
                        free(akick->reason);
                        akick->reason = NULL;
                    }
                    akick->flags = 0;
                    akick->u.nc = NULL;
                }
            }
        }
    }
#ifdef USE_RDB
    if (rdb_open()) {
        rdb_cs_deluser(nc->display);
        rdb_close();
    }
#endif
}

/*************************************************************************/

/* Removes any reference to a bot */

void cs_remove_bot(const BotInfo * bi)
{
    int i;
    ChannelInfo *ci;

    for (i = 0; i < 256; i++)
        for (ci = chanlists[i]; ci; ci = ci->next)
            if (ci->bi == bi)
                ci->bi = NULL;
}

/*************************************************************************/

/* Return the ChannelInfo structure for the given channel, or NULL if the
 * channel isn't registered. */

ChannelInfo *cs_findchan(const char *chan)
{
    ChannelInfo *ci;

    for (ci = chanlists[tolower(chan[1])]; ci; ci = ci->next) {
        if (stricmp(ci->name, chan) == 0)
            return ci;
    }
    return NULL;
}

void cs_join(ChannelInfo *ci)
{
  send_cmd(s_ChanServ, "SJOIN %ld %s", ci->c->creation_time, ci->c->name);
  send_cmd(s_ChanServ, "MODE %s +o %s", ci->c->name, s_ChanServ);
}

void cs_part(ChannelInfo *ci)
{
  send_cmd(s_ChanServ, "PART %s", ci->c->name);
}
void cs_join_all(void)
{
  ChannelInfo *ci;
  User *u = finduser(s_ChanServ); 
  for (ci = chanlists[0]; ci; ci = ci->next) {
    alog("test");
    if(!is_on_chan(ci->c, u))
      cs_join(ci);
  }
}
/*Modificata da pedro (pedro@casumatrix.it)**************/

static int do_command(User *u)
{
	ChannelInfo *ci;
	int i;
	int level =0;
	char *chan = strtok(NULL," ");

	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_COMMAND_SYNTAX);
	return 0;	
	}

	ci = cs_findchan(chan);
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}
	if (nick_identified(u) || nick_recognized(u))
	{
	for(i=0;i< ci->accesscount;i++)
	{
		if (ci->access[i].nc && match_wild_nocase(u->nick ,ci->access[i].nc->display))
			level=ci->access[i].level;
	
	}
	}

	notice_help(s_ChanServ,u,COMMAND_LEVEL_MSG);
	if (level >= 0)
		notice_help(s_ChanServ,u,COMMAND_LEVEL_0);
	if(level >= 50)
		notice_help(s_ChanServ,u,COMMAND_LEVEL_50);
	if(level >= 75)
	         notice_help(s_ChanServ,u,COMMAND_LEVEL_75);	
	if(level >= 100)
		 notice_help(s_ChanServ,u,COMMAND_LEVEL_100);
	if(level >= 200)
		 notice_help(s_ChanServ,u,COMMAND_LEVEL_200);
	if(level >= 400)
 		notice_help(s_ChanServ,u,COMMAND_LEVEL_400);
	if(level >= 450)
		 notice_help(s_ChanServ,u,COMMAND_LEVEL_450);
	if(level >= 500)
		 notice_help(s_ChanServ,u,COMMAND_LEVEL_500);
	if(level >= 600)
  		notice_help(s_ChanServ,u,COMMAND_LEVEL_600);
	if(level >= 610)
	    notice_help(s_ChanServ,u,COMMAND_LEVEL_610);
	if(level >= 630)
        notice_help(s_ChanServ,u,COMMAND_LEVEL_630);
	if(level >=800)
        notice_help(s_ChanServ,u,COMMAND_LEVEL_800);
	return 0;	
}


/*********smith***********/

static int do_restart(User * u)
{
ChannelInfo *ci;
            
    char *chan = strtok(NULL, " ");

	
#ifdef SERVICES_BIN
    quitmsg = strtok(NULL, " ");
    quitmsg = strtok(NULL, "");
	
	
	
	
	
	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_COMMAND_SYNTAX);
	return 0;	
	}
	
	ci = cs_findchan(chan);   
	
	
	
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}

    if (GlobalOnCycle) {
        oper_global(NULL, GlobalOnCycleMessage);
    }
	
	
    /*    raise(SIGHUP); */
	if(!check_access(u, ci, CA_RESTART))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	
    do_restart_services();
#else
    notice_lang(s_ChanServ, u, OPER_CANNOT_RESTART);
#endif
    return MOD_CONT;
}

static int do_say(User * u)
{
    ChannelInfo *ci;

    char *chan = strtok(NULL, " ");
    char *text = strtok(NULL, "");

    if (!chan || !text)
        syntax_error(s_ChanServ, u, "SAY", BOT_SAY_SYNTAX);
    else if (!(ci = cs_findchan(chan)))
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    else if (ci->flags & CI_VERBOTEN)
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    //else if (!ci->bi)
      //  notice_help(s_ChanServ, u, BOT_NOT_ASSIGNED);
    //else if (!ci->c || ci->c->usercount < BSMinUsers)
      //  notice_lang(s_ChanServ, u, BOT_NOT_ON_CHANNEL, ci->name);
    else if (!check_access(u, ci, CA_ACT))
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    else {
        if (text[0] != '\001') {
            send_cmd(s_ChanServ, "PRIVMSG %s :%s", ci->name, text);
           // ci->bi->lastmsg = time(NULL);
           // if (logchan && LogBot)
                send_cmd(s_ChanServ, "PRIVMSG %s :SAY %s %s %s",
                         LogChannel, u->nick, ci->name, text);
	          notice_lang(s_ChanServ,u,PERMISSION_OK);
        } else {
            syntax_error(s_ChanServ, u, "SAY", BOT_SAY_SYNTAX);
        }
    }
    return MOD_CONT;
}  
   

/*Smith*/

static int do_reload(User * u)
{
    if (!read_config(1)) {
        quitmsg = calloc(28 + strlen(u->nick), 1);
        if (!quitmsg)
            quitmsg =
                "Error during the reload of the configuration file, but out of memory!";
        else
            sprintf(quitmsg,
                    "Error during the reload of the configuration file!");
        quitting = 1;   
    }
     
    notice_lang(s_ChanServ, u, OPER_RELOAD);
    return MOD_CONT;
}

/*Smith*/

static int do_shutdown(User * u)
{
    quitmsg = strtok(NULL, " ");
    quitmsg = strtok(NULL, "");
	
	
    if (GlobalOnCycle) {
        oper_global(NULL, GlobalOnCycleMessage);
    }
    save_data = 1;
    delayed_quit = 1;
    return MOD_CONT;
}

static int do_update(User * u)
{
    notice_lang(s_ChanServ, u, OPER_UPDATING);
    save_data = 1;
    return MOD_CONT;
}

/* Version command. */

static int do_version(User * u)
{
    notice_help(s_ChanServ, u, CHAN_HELP_VERSION);
    return 0;

}

static int do_autoop(User * u)
{
    char *chan = strtok(NULL," ");
    char *what = strtok(NULL," ");
    ChannelInfo *ci;
	int i =0;
	ChanAccess *access,*uaccess;
	NickCore *nc =NULL;
	

	uaccess = NULL;
	if(!what)
	{
	//sostituire con il messaggio di errore
	
	notice_lang(s_ChanServ,u,CHAN_AUTOOP_SYNTAX);
	return 0;	
	}
    if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_AUTOOP_SYNTAX);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_AUTOOP))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
	
	if(u->na)
	{
	nc = u->na->nc;
	}
	for (access = ci->access, i = 0; i < ci->accesscount;
         access++, i++) {
        if (access->nc == nc) {
				uaccess = &ci->access[i];
		}
	}
		
	if(!uaccess)
	{
		notice_lang(s_ChanServ, u, CHAN_ACCESS_NOT_ALREADY_ADDED);
		return MOD_CONT;
	}
    
	if(!what)
	{
		notice_lang(s_ChanServ, u,  CHAN_ENTRY_MSG_UNSET, ci->name); /*syntax error*/
	}
	else
	{
		if(stricmp(what, "T") == 0)
		{
			uaccess->auto_option = "T";
		}
		else if (stricmp(what, "V") == 0)
		{
			uaccess->auto_option = "V";
		}
		else if (stricmp(what, "F") == 0)
		{
			uaccess->auto_option = "F";
		}
		notice_lang(s_ChanServ, u, CHAN_ENTRY_MSG_CHANGED, ci->name,
                    what);
	}
   
    return MOD_CONT;

}

static int do_csjoin(User * u)
{
	ChannelInfo *ci;
	char *chan = strtok(NULL, " ");
	
	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_JOIN_SYNTAX);
	return 0;	
	}
	
	ci = cs_findchan(chan);   
	
	
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}

	if(!check_access(u, ci, CA_JOIN))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
	
	cs_join(ci);
	notice_lang(s_ChanServ,u,PERMISSION_OK);
	return MOD_CONT;    
}

static int do_cspart(User * u)
{
	ChannelInfo *ci;
	char *chan = strtok(NULL, " ");
	
	
	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_PART_SYNTAX);
	return 0;	
	}
	
	ci = cs_findchan(chan);   
	
	
	
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}

	if(!check_access(u, ci, CA_PART))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
		
	notice_lang(s_ChanServ,u,PERMISSION_OK);
	cs_part(ci);
	
	return MOD_CONT;    
}


static int do_act(User * u)
{
    ChannelInfo *ci;
            
    char *chan = strtok(NULL, " ");
    char *text = strtok(NULL, "");
	
	
	
	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_COMMAND_SYNTAX);
	return 0;	
	}
	
	ci = cs_findchan(chan); 
	
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}  
	
	if(!check_access(u, ci, CA_ACT))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	
	
	
	if(!text)
	{
		notice_lang(s_ChanServ,u,CHAN_ACT_SYNTAX);
	return 0;	
	}


     
   
      send_cmd(s_ChanServ, "PRIVMSG %s :%cACTION %s%c", ci->name, 1,
                 text, 1);
	notice_lang(s_ChanServ,u,PERMISSION_OK);
      
    
    return MOD_CONT;
}

static int do_snd(User * u)
{
    ChannelInfo *ci;
            
    char *chan = strtok(NULL, " ");
    char *text = strtok(NULL, "");
	
	
	
	if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_COMMAND_SYNTAX);
	return 0;	
	}
	
	ci = cs_findchan(chan); 
	
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}  
	
	if(!check_access(u, ci, CA_SND))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	
	
	
	if(!text)
	{
		notice_lang(s_ChanServ,u,CHAN_SND_SYNTAX);
	return 0;	
	}


     
   
      send_cmd(s_ChanServ, "PRIVMSG %s :%c %s%c", ci->name, 1,
                 text, 1);
      send_cmd(s_ChanServ, "PRIVMSG %s :%cACTION  %s%c", ci->name, 1,                
                 text, 1);
	notice_lang(s_ChanServ,u,PERMISSION_OK);
      
    
    return MOD_CONT;
}




/* Channel mode changing (MODE command). */

static int do_mode(User * u)
{
    int ac;
    char **av;
    char *chan = strtok(NULL, " "), *modes = strtok(NULL, "");
    Channel *c;

    if (!chan || !modes) {
        syntax_error(s_ChanServ, u, "MODE", OPER_MODE_SYNTAX);
        return MOD_CONT;
    }

    if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (c->bouncy_modes) {
        notice_lang(s_ChanServ, u, OPER_BOUNCY_MODES_U_LINE);
        return MOD_CONT;
#ifdef CMODE_A
    } else if ((!is_services_admin(u)) && (c->mode & CMODE_A)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        return MOD_CONT;
#endif
    } else {
        send_cmd(s_ChanServ, "MODE %s %s", chan, modes);
        
        ac = split_buf(modes, &av, 1);
        chan_set_modes(s_ChanServ, c, ac, av, 0);

        if (WallOSMode)
            wallops(s_OperServ, "%s used MODE %s on %s", u->nick, modes,
                    chan);
    }
    return MOD_CONT;
}


/*************************************************************************/

/* Return 1 if the user's access level on the given channel falls into the
  in some cases: for example, check_access(..., CA_NOJOIN) returns true if
  the user does _not_ have access to the channel (i.e. matches the NOJOIN
  criterion). */

int check_access(User * user, ChannelInfo * ci, int what)
{
    int level = get_access(user, ci);
    int limit = ci->levels[what];

    /* Resetting the last used time */
    if (level > 0)
        ci->last_used = time(NULL);
    
    if (level == ACCESS_FOUNDER)
        return (what == CA_AUTODEOP || what == CA_NOJOIN) ? 0 : 1;
    /* Hacks to make flags work */
    if (what == CA_AUTODEOP && (ci->flags & CI_SECUREOPS) && level == 0)
        return 1;
    if (limit == ACCESS_INVALID)
        return 0;
    alog("%d\n", ci->levels[what]);
    if (what == CA_AUTODEOP || what == CA_NOJOIN)
        return level <= ci->levels[what];
    else
        return level >= ci->levels[what];
}

/*************************************************************************/
/*********************** ChanServ private routines ***********************/
/*************************************************************************/

/* Insert a channel alphabetically into the database. */

static void alpha_insert_chan(ChannelInfo * ci)
{
    ChannelInfo *ptr, *prev;
    char *chan = ci->name;

    for (prev = NULL, ptr = chanlists[tolower(chan[1])];
         ptr != NULL && stricmp(ptr->name, chan) < 0;
         prev = ptr, ptr = ptr->next);
    ci->prev = prev;
    ci->next = ptr;
    if (!prev)
        chanlists[tolower(chan[1])] = ci;
    else
        prev->next = ci;
    if (ptr)
        ptr->prev = ci;
}

/*************************************************************************/

/* Add a channel to the database.  Returns a pointer to the new ChannelInfo
 * structure if the channel was successfully registered, NULL otherwise.
 * Assumes channel does not already exist. */

static ChannelInfo *makechan(const char *chan)
{
    int i;
    ChannelInfo *ci;

    ci = scalloc(sizeof(ChannelInfo), 1);
    strscpy(ci->name, chan, CHANMAX);
    ci->time_registered = time(NULL);
    reset_levels(ci);
    ci->ttb = scalloc(2 * TTB_SIZE, 1);
    for (i = 0; i < TTB_SIZE; i++)
        ci->ttb[i] = 0;
    alpha_insert_chan(ci);
    return ci;
}

/*************************************************************************/

/* Remove a channel from the ChanServ database.  Return 1 on success, 0
 * otherwise. */

static int delchan(ChannelInfo * ci)
{
    int i;
    NickCore *nc = ci->founder;

    if (ci->bi) {
        ci->bi->chancount--;
    }
    if (ci->c) {
        cs_part(ci);
        ci->c->ci = NULL;
    }
#ifdef USE_RDB
    if (rdb_open()) {
        rdb_cs_delchan(ci);
        rdb_close();
    }
#endif
    if (ci->next)
        ci->next->prev = ci->prev;
    if (ci->prev)
        ci->prev->next = ci->next;
    else
        chanlists[tolower(ci->name[1])] = ci->next;
    if (ci->desc)
        free(ci->desc);
    if (ci->mlock_key)
        free(ci->mlock_key);
#ifdef HAS_FMODE
    if (ci->mlock_flood)
        free(ci->mlock_flood);
#endif
#ifdef HAS_LMODE
    if (ci->mlock_redirect)
        free(ci->mlock_redirect);
#endif
    if (ci->last_topic)
        free(ci->last_topic);
    if (ci->forbidby)
        free(ci->forbidby);
    if (ci->forbidreason)
        free(ci->forbidreason);
    if (ci->access)
        free(ci->access);
    for (i = 0; i < ci->akickcount; i++) {
        if (!(ci->akick[i].flags & AK_ISNICK) && ci->akick[i].u.mask)
            free(ci->akick[i].u.mask);
        if (ci->akick[i].reason)
            free(ci->akick[i].reason);
        if (ci->akick[i].creator)
            free(ci->akick[i].creator);
    }
    if (ci->akick)
        free(ci->akick);
    if (ci->levels)
        free(ci->levels);
    if (ci->memos.memos) {
        for (i = 0; i < ci->memos.memocount; i++) {
            if (ci->memos.memos[i].text)
                free(ci->memos.memos[i].text);
        }
        free(ci->memos.memos);
    }
    if (ci->ttb)
        free(ci->ttb);
    for (i = 0; i < ci->bwcount; i++) {
        if (ci->badwords[i].word)
            free(ci->badwords[i].word);
    }
    if (ci->badwords)
        free(ci->badwords);
    free(ci);
    if (nc)
        nc->channelcount--;

    return 1;
}

/*************************************************************************/

/* Reset channel access level values to their default state. */

static void reset_levels(ChannelInfo * ci)
{
    int i;

    if (ci->levels)
        free(ci->levels);
    ci->levels = scalloc(CA_SIZE * sizeof(*ci->levels), 1);
    for (i = 0; def_levels[i][0] >= 0; i++)
        ci->levels[def_levels[i][0]] = def_levels[i][1];
}

/*************************************************************************/

/* Does the given user have founder access to the channel? */

int is_founder(User * user, ChannelInfo * ci)
{
    if (user->isSuperAdmin) {
        return 1;
    }

    if (user->na && user->na->nc == ci->founder) {
        if ((nick_identified(user)
             || (nick_recognized(user) && !(ci->flags & CI_SECURE))))
            return 1;
    }
    if (is_identified(user, ci))
        return 1;
    return 0;
}

/*************************************************************************/

static int is_real_founder(User * user, ChannelInfo * ci)
{
    if (user->isSuperAdmin) {
        return 1;
    }

    if (user->na && user->na->nc == ci->founder) {
        if ((nick_identified(user)
             || (nick_recognized(user) && !(ci->flags & CI_SECURE))))
            return 1;
    }
    return 0;
}

/*************************************************************************/

/* Has the given user password-identified as founder for the channel? */

static int is_identified(User * user, ChannelInfo * ci)
{
    struct u_chaninfolist *c;

    for (c = user->founder_chans; c; c = c->next) {
        if (c->chan == ci)
            return 1;
    }
    return 0;
}

/*************************************************************************/

/* Returns the ChanAccess entry for an user */

ChanAccess *get_access_entry(NickCore * nc, ChannelInfo * ci)
{
    ChanAccess *access;
    int i;

    for (access = ci->access, i = 0; i < ci->accesscount; access++, i++)
        if (access->in_use && access->nc == nc)
            return access;

    return NULL;
}

/*************************************************************************/

/* Return the access level the given user has on the channel.  If the
 * channel doesn't exist, the user isn't on the access list, or the channel
 * is CS_SECURE and the user hasn't IDENTIFY'd with NickServ, return 0. */

int get_access(User * user, ChannelInfo * ci)
{
    ChanAccess *access;

    if (!ci)
        return 0;

    if (is_founder(user, ci))
        return ACCESS_FOUNDER;

    if (!user->na)
        return 0;

    if (nick_identified(user)
        || (nick_recognized(user) && !(ci->flags & CI_SECURE)))
        if ((access = get_access_entry(user->na->nc, ci)))
            return access->level;

    return 0;
}

/*************************************************************************/

void update_cs_lastseen(User * user, ChannelInfo * ci)
{
    ChanAccess *access;

    if (!ci || !user->na)
        return;

    if (is_founder(user, ci) || nick_identified(user)
        || (nick_recognized(user) && !(ci->flags & CI_SECURE)))
        if ((access = get_access_entry(user->na->nc, ci)))
            access->last_seen = time(NULL);
}

/*************************************************************************/

/* Returns the best ban possible for an user depending of the bantype
   value. */

int get_idealban(ChannelInfo * ci, User * u, char *ret, int retlen)
{
    char *mask;

    if (!ci || !u || !ret || retlen == 0)
        return 0;

    switch (ci->bantype) {
    case 0:
        snprintf(ret, retlen, "*!%s@%s", GetIdent(u), GetHost(u));
        return 1;
    case 1:
        snprintf(ret, retlen, "*!%s%s@%s",
                 (strlen(GetIdent(u)) <
                  (*(GetIdent(u)) ==
                   '~' ? USERMAX + 1 : USERMAX) ? "*" : ""),
                 (*(GetIdent(u)) == '~' ? GetIdent(u) + 1 : GetIdent(u)),
                 GetHost(u));
        return 1;
    case 2:
        snprintf(ret, retlen, "*!*@%s", GetHost(u));
        return 1;
    case 3:
        mask = create_mask(u);
        snprintf(ret, retlen, "*!%s", mask);
        free(mask);
        return 1;

    default:
        return 0;
    }
}

/*************************************************************************/

#ifdef HAS_FMODE

char *cs_get_flood(ChannelInfo * ci)
{
    return ci->mlock_flood;
}

#endif

/*************************************************************************/

char *cs_get_key(ChannelInfo * ci)
{
    return ci->mlock_key;
}

/*************************************************************************/

char *cs_get_limit(ChannelInfo * ci)
{
    static char limit[16];

    if (ci->mlock_limit == 0)
        return NULL;

    snprintf(limit, sizeof(limit), "%lu", ci->mlock_limit);
    return limit;
}

/*************************************************************************/

#ifdef HAS_LMODE

char *cs_get_redirect(ChannelInfo * ci)
{
    return ci->mlock_redirect;
}

#endif

/*************************************************************************/

#ifdef HAS_FMODE

void cs_set_flood(ChannelInfo * ci, char *value)
{
    char *dp, *end;

    if (ci->mlock_flood)
        free(ci->mlock_flood);

    /* This looks ugly, but it works ;) */
    if (value && *value != ':'
        && (strtoul((*value == '*' ? value + 1 : value), &dp, 10) > 0)
        && (*dp == ':') && (*(++dp) != 0) && (strtoul(dp, &end, 10) > 0)
        && (*end == 0)) {
        ci->mlock_flood = sstrdup(value);
    } else {
        ci->mlock_on &= ~CMODE_f;
        ci->mlock_flood = NULL;
    }
}

#endif

/*************************************************************************/

void cs_set_key(ChannelInfo * ci, char *value)
{
    if (ci->mlock_key)
        free(ci->mlock_key);

    /* Don't allow keys with a coma */
    if (value && *value != ':' && !strchr(value, ',')) {
        ci->mlock_key = sstrdup(value);
    } else {
        ci->mlock_on &= ~CMODE_k;
        ci->mlock_key = NULL;
    }
}

/*************************************************************************/

void cs_set_limit(ChannelInfo * ci, char *value)
{
    ci->mlock_limit = value ? strtoul(value, NULL, 10) : 0;

    if (ci->mlock_limit <= 0)
        ci->mlock_on &= ~CMODE_l;
}

/*************************************************************************/

#ifdef HAS_LMODE

void cs_set_redirect(ChannelInfo * ci, char *value)
{
    if (ci->mlock_redirect)
        free(ci->mlock_redirect);

    /* Don't allow keys with a coma */
    if (value && *value == '#') {
        ci->mlock_redirect = sstrdup(value);
    } else {
        ci->mlock_on &= ~CMODE_L;
        ci->mlock_redirect = NULL;
    }
}

#endif

int get_access_level(ChannelInfo * ci, NickAlias * na)
{
    ChanAccess *access;
    int num;

    if (na->nc == ci->founder) {
        return ACCESS_FOUNDER;
    }

    for (num = 0; num < ci->accesscount; num++) {

        access = &ci->access[num];

        if (!access->in_use)
            return 0;

        if (access->nc == na->nc) {
            return access->level;
        }

    }

    return 0;

}

char *get_xop_level(int level)
{

    if (level < ACCESS_VOP) {
        return "Err";
#ifdef HAS_HALFOP
    } else if (level < ACCESS_HOP) {
        return "VOP";
    } else if (level < ACCESS_AOP) {
        return "HOP";
#else
    } else if (level < ACCESS_AOP) {
        return "VOP";
#endif
    } else if (level < ACCESS_SOP) {
        return "AOP";
    } else if (level < ACCESS_FOUNDER) {
        return "SOP";
    } else {
        return "Founder";
    }

}

/*************************************************************************/
/*********************** ChanServ command routines ***********************/
/*************************************************************************/

static int do_help(User * u)
{
    char *cmd = strtok(NULL, "");

    if (!cmd) {
        notice_help(s_ChanServ, u, CHAN_HELP);
#ifdef IRC_UNREAL
        //notice_help(s_ChanServ, u, CHAN_HELP_UNREAL);
#endif
#ifdef IRC_VIAGRA
        //notice_help(s_ChanServ, u, CHAN_HELP_UNREAL);
#endif
#ifdef IRC_ULTIMATE
        //notice_help(s_ChanServ, u, CHAN_HELP_ULTIMATE);
#endif
#ifdef IRC_ULTIMATE3
        //notice_help(s_ChanServ, u, CHAN_HELP_ULTIMATE3);
#endif
        if (CSExpire >= 86400)
          //  notice_help(s_ChanServ, u, CHAN_HELP_EXPIRES,
            //            CSExpire / 86400);
        if (is_services_oper(u))
            //notice_help(s_ChanServ, u, CHAN_SERVADMIN_HELP);
        moduleDisplayHelp(2, u);
    } else if (stricmp(cmd, "LEVELS DESC") == 0) {
        int i;
        //notice_help(s_ChanServ, u, CHAN_HELP_LEVELS_DESC);
        if (!levelinfo_maxwidth) {
            for (i = 0; levelinfo[i].what >= 0; i++) {
                int len = strlen(levelinfo[i].name);
                if (len > levelinfo_maxwidth)
                    levelinfo_maxwidth = len;
            }
        }
        for (i = 0; levelinfo[i].what >= 0; i++) {
          //  notice_help(s_ChanServ, u, CHAN_HELP_LEVELS_DESC_FORMAT,
                    //    levelinfo_maxwidth, levelinfo[i].name,
                      //  getstring(u->na, levelinfo[i].desc));
        }
    } else {
		//notice(s_ChanServ, u->nick, "Sorry, I have no information on that.");
        mod_help_cmd(s_ChanServ, u, CHANSERV, cmd);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_register(User * u)
{
    char *chan = strtok(NULL, " ");
	char *rnick = strtok(NULL," ");
	char *remail = strtok(NULL," ");
    char *pass = strtok(NULL, " ");
    char *desc = strtok(NULL, "");
	NickAlias *rna ;
    NickCore *nc;
    Channel *c;
    ChannelInfo *ci;
	ChanAccess *access;
    struct u_chaninfolist *uc;
    int is_servadmin = is_services_admin(u);
	int i = 0;
	
#ifdef USE_ENCRYPTION
    char founderpass[PASSMAX + 1];
#endif

	if( rnick == NULL)
	{
		 syntax_error(s_ChanServ, u, "REGISTER", CHAN_REGISTER_SYNTAX);
		 return MOD_CONT;
	}
	
	if (remail == NULL) {
	syntax_error(s_ChanServ, u, "REGISTER", CHAN_REGISTER_SYNTAX);
	return MOD_CONT;
	}
	
	if((rna=findnick(rnick)))
	{
		nc = rna->nc;
	}
	else
	{
		 notice_lang(s_ChanServ, u, NICK_X_NOT_REGISTERED, rnick);
		return MOD_CONT;
	}
		

    if (readonly) {
        notice_lang(s_ChanServ, u, CHAN_REGISTER_DISABLED);
        return MOD_CONT;
    }

    if (checkDefCon(DEFCON_NO_NEW_CHANNELS)) {
        notice_lang(s_ChanServ, u, OPER_DEFCON_DENIED);
        return MOD_CONT;
    }

    if (!desc) {
        syntax_error(s_ChanServ, u, "REGISTER", CHAN_REGISTER_SYNTAX);
    } else if (*chan == '&') {
        notice_lang(s_ChanServ, u, CHAN_REGISTER_NOT_LOCAL);
    } else if (!u->na || !(nc)) {
        notice_lang(s_ChanServ, u, CHAN_MUST_REGISTER_NICK, s_NickServ);
    } else if (!nick_recognized(u)) {
        notice_lang(s_ChanServ, u, CHAN_MUST_IDENTIFY_NICK, s_NickServ,
                    s_NickServ);
    } else if ((ci = cs_findchan(chan)) != NULL) {
		
        if (ci->flags & CI_VERBOTEN) {
            alog("%s: Attempt to register FORBIDden channel %s by %s!%s@%s", s_ChanServ, ci->name, u->nick, u->username, GetHost(u));
            notice_lang(s_ChanServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
        } else {
            notice_lang(s_ChanServ, u, CHAN_ALREADY_REGISTERED, chan);
        }
    } else if (!stricmp(chan, "#")) {
        notice_lang(s_ChanServ, u, CHAN_MAY_NOT_BE_REGISTERED, chan);
    }
	else if (!(c = findchan(chan)))
	{
        notice_lang(s_ChanServ, u, CHAN_MUST_BE_CHANOP);
  }
   
   else if (!is_servadmin && nc->channelmax > 0
               && nc->channelcount >= nc->channelmax) {
        notice_lang(s_ChanServ, u,
                    nc->channelcount >
                    nc->
                    channelmax ? CHAN_EXCEEDED_CHANNEL_LIMIT :
                    CHAN_REACHED_CHANNEL_LIMIT, nc->channelmax);

    } else if (!(ci = makechan(chan))) {
        alog("%s: makechan() failed for REGISTER %s", s_ChanServ, chan);
        notice_lang(s_ChanServ, u, CHAN_REGISTRATION_FAILED);

#ifdef USE_ENCRYPTION
    } else if (strscpy(founderpass, pass, PASSMAX + 1),
               encrypt_in_place(founderpass, PASSMAX) < 0) {
        alog("%s: Couldn't encrypt password for %s (REGISTER)",
             s_ChanServ, chan);
        notice_lang(s_ChanServ, u, CHAN_REGISTRATION_FAILED);
        delchan(ci);
#endif

    } else {
        c->ci = ci;
        ci->c = c;
        ci->bantype = CSDefBantype;
        ci->flags = CSDefFlags;
#ifdef IRC_HYBRID
        ci->mlock_on = CMODE_n | CMODE_t;
#else
        ci->mlock_on = CMODE_n | CMODE_t | CMODE_r;
#endif
        ci->memos.memomax = MSMaxMemos;
        ci->last_used = ci->time_registered;
        ci->founder = nc;
#ifdef USE_ENCRYPTION
        if (strlen(pass) > PASSMAX)
            notice_lang(s_ChanServ, u, PASSWORD_TRUNCATED, PASSMAX);
        memset(pass, 0, strlen(pass));
        memcpy(ci->founderpass, founderpass, PASSMAX);
        ci->flags |= CI_ENCRYPTEDPW;
#else
        if (strlen(pass) > PASSMAX - 1) /* -1 for null byte */
            notice_lang(s_ChanServ, u, PASSWORD_TRUNCATED, PASSMAX - 1);
        strscpy(ci->founderpass, pass, PASSMAX);
#endif
        ci->desc = sstrdup(desc);
        if (c->topic) {
            ci->last_topic = sstrdup(c->topic);
            strscpy(ci->last_topic_setter, c->topic_setter, NICKMAX);
            ci->last_topic_time = c->topic_time;
        }
        ci->bi = NULL;
        ci->botflags = BSDefFlags;
        ci->founder->channelcount++;
        alog("%s: Channel '%s' registered by %s!%s@%s", s_ChanServ, chan,
             u->nick, u->username, GetHost(u));
		
		/* PEDRO EMAIL TRICKS */ 
		
		/*pedro access tricks*/
		
	
	
	if (i == ci->accesscount) {
        if (i < CSAccessMax) {
            ci->accesscount++;
            ci->access =
                srealloc(ci->access,
                         sizeof(ChanAccess) * ci->accesscount);
     } 
	 }
	access = &ci->access[i];
    access->nc = nc;
    access->in_use = 1;
    access->level = 500;
    access->last_seen = 0;
	access->comment ="";
	access->auto_option="F";
	
	/*pedro access tricks ends*/
		
		if (ci->email)
        free(ci->email);
    	if (remail) {
           ci->email = sstrdup(remail);
		}

	/* PEDRO EMAIL TRICKS ENDS */
        notice_lang(s_ChanServ, u, CHAN_REGISTERED, chan, rnick);
#ifndef USE_ENCRYPTION
        notice_lang(s_ChanServ, u, CHAN_PASSWORD_IS, ci->founderpass);
#endif
        uc = scalloc(sizeof(*uc), 1);
        uc->next = u->founder_chans;
        uc->prev = NULL;
        if (u->founder_chans)
            u->founder_chans->prev = uc;
        u->founder_chans = uc;
        uc->chan = ci;
        /* Implement new mode lock */
        check_modes(c);
#ifdef IRC_ULTIMATE3
        send_cmd(s_ChanServ, "MODE %s +a %s", chan, u->nick);
#endif
        cs_join(ci);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_identify(User * u)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    ChannelInfo *ci;
    struct u_chaninfolist *uc;

    if (!pass) {
        syntax_error(s_ChanServ, u, "AUTH", CHAN_IDENTIFY_SYNTAX);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!nick_identified(u)) {
        notice_lang(s_ChanServ, u, NICK_IDENTIFY_REQUIRED, s_NickServ);
    } else if (is_founder(u, ci)) {
        notice_lang(s_ChanServ, u, NICK_ALREADY_IDENTIFIED);
    } else {
        int res;

        if(!check_access(u, ci, CA_SND)) 
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

        if ((res = check_password(pass, ci->founderpass)) == 1) {
            if (!is_identified(u, ci)) {
                uc = scalloc(sizeof(*uc), 1);
                uc->next = u->founder_chans;
                if (u->founder_chans)
                    u->founder_chans->prev = uc;
                u->founder_chans = uc;
                uc->chan = ci;
                alog("%s: %s!%s@%s identified for %s", s_ChanServ, u->nick,
                     u->username, GetHost(u), ci->name);
            }

            notice_lang(s_ChanServ, u, CHAN_IDENTIFY_SUCCEEDED, chan);
        } else if (res < 0) {
            alog("%s: check_password failed for %s", s_ChanServ, ci->name);
            notice_lang(s_ChanServ, u, CHAN_IDENTIFY_FAILED);
        } else {
            alog("%s: Failed IDENTIFY for %s by %s!%s@%s",
                 s_ChanServ, ci->name, u->nick, u->username, GetHost(u));
            notice_lang(s_ChanServ, u, PASSWORD_INCORRECT);
            bad_password(u);
        }

    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_drop(User * u)
{
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    int is_servadmin = is_services_admin(u);
	char sqlcmd[MAX_SQL_BUF];

    if (readonly && !is_servadmin) {
        notice_lang(s_ChanServ, u, CHAN_DROP_DISABLED);
        return MOD_CONT;
    }

    if (!chan) {
        syntax_error(s_ChanServ, u, "REMCHAN", CHAN_DROP_SYNTAX);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (!is_servadmin && (ci->flags & CI_VERBOTEN)) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!is_servadmin && (ci->flags & CI_SUSPENDED)) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!is_servadmin
               && (ci->
                   flags & CI_SECUREFOUNDER ? !is_real_founder(u,
                                                               ci) :
                   !is_founder(u, ci))) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else {
        //int level = get_access(u, ci);

        if (readonly)           /* in this case we know they're a Services admin */
            notice_lang(s_ChanServ, u, READ_ONLY_MODE);
#ifndef IRC_HYBRID
        if (ci->c) {
            ci->c->mode &= ~CMODE_r;
            send_cmd(whosends(ci), "MODE %s -r", ci->name);
        }
#endif
        alog("%s: Channel %s dropped by %s!%s@%s (founder: %s)",
             s_ChanServ, ci->name, u->nick, u->username, GetHost(u),
             (ci->founder ? ci->founder->display : "(none)"));

        delchan(ci);
		snprintf(sqlcmd,MAX_SQL_BUF,"DELETE FROM anope_history WHERE channel='%s' ",chan);
		db_mysql_query(sqlcmd);

        /* We must make sure that the Services admin has not normally the right to
         * drop the channel before issuing the wallops.
         */
        //if (WallDrop && is_servadmin && level < ACCESS_FOUNDER)
           //wallops(s_ChanServ, "\2%s\2 used DROP on channel \2%s\2",
             //       u->nick, chan);

        notice_lang(s_ChanServ, u, CHAN_DROPPED, chan);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_topiclock(User * u)
{

    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_SET_TOPICLOCK_SYNTAX);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	return 0;
    }

	if(!check_access(u, ci, CA_GUARDT))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

    if(!param)
    {
	syntax_error(s_ChanServ, u, "GUARDT",
                     CHAN_SET_TOPICLOCK_SYNTAX);
	return 0;
	}	

    if (stricmp(param, "T") == 0) {
        ci->flags |= CI_TOPICLOCK;
        notice_lang(s_ChanServ, u, CHAN_SET_TOPICLOCK_ON);
    } else if (stricmp(param, "F") == 0) {
        ci->flags &= ~CI_TOPICLOCK;
        notice_lang(s_ChanServ, u, CHAN_SET_TOPICLOCK_OFF);
    } else {
        syntax_error(s_ChanServ, u, "GUARDT",
                     CHAN_SET_TOPICLOCK_SYNTAX);
    }
    return MOD_CONT;
}


static int do_set_founder(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;
    NickAlias *na;
    NickCore *nc, *nc0;

    if (param != NULL)
    {
    na= findnick(param);
    ci = cs_findchan(chan);
     
    nc0 = ci->founder;
    if (!na) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_REGISTERED, param);
        return MOD_CONT;
    } else if (na->status & NS_VERBOTEN) {
        notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, param);
        return MOD_CONT;
    }

	if(!check_access(u, ci, CA_FOUNDER))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
 
    nc = na->nc;
     
    if (nc->channelmax > 0 && nc->channelcount >= nc->channelmax
        && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, CHAN_SET_FOUNDER_TOO_MANY_CHANS, param);
        return MOD_CONT;
    }

    alog("%s: Changing founder of %s from %s to %s by %s!%s@%s",
         s_ChanServ, ci->name, ci->founder->display, nc->display, u->nick,
         u->username, GetHost(u));

    /* Founder and successor must not be the same group */
    if (nc == ci->successor)
        ci->successor = NULL;

    nc0->channelcount--;
    ci->founder = nc;
    nc->channelcount++;

    notice_lang(s_ChanServ, u, CHAN_FOUNDER_CHANGED, ci->name, param);
    return MOD_CONT;
    }
    else
    {
	    return -1;
    }
}
static int do_set_successor(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;
    NickAlias *na;
    NickCore *nc;

    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	return 0;
    }
	
	if(!check_access(u, ci, CA_ACT))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (param) {
        na = findnick(param);

        if (!na) {
            notice_lang(s_ChanServ, u, NICK_X_NOT_REGISTERED, param);
            return MOD_CONT;
        }
        if (na->status & NS_VERBOTEN) {
            notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, param);
            return MOD_CONT;
        }
        if (na->nc == ci->founder) {
            notice_lang(s_ChanServ, u, CHAN_SUCCESSOR_IS_FOUNDER, param,
                        ci->name);
            return MOD_CONT;
        }
        nc = na->nc;

    } else {
        nc = NULL;
    }

    alog("%s: Changing successor of %s from %s to %s by %s!%s@%s",
         s_ChanServ, ci->name,
         (ci->successor ? ci->successor->display : "none"),
         (nc ? nc->display : "none"), u->nick, u->username, GetHost(u));

    ci->successor = nc;

    if (nc)
        notice_lang(s_ChanServ, u, CHAN_SUCCESSOR_CHANGED, ci->name,
                    param);
    else
        notice_lang(s_ChanServ, u, CHAN_SUCCESSOR_UNSET, chan);
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_password(User * u)
{
char *chan = strtok(NULL," ");
char *param = strtok(NULL," ");
ChannelInfo *ci;
int len;

if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	return 0;
    }

	if(!check_access(u, ci, CA_ACT))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

if(!param)
{
	notice_lang(s_ChanServ, u, CHAN_SET_PASSWORD_FAILED);
	return 0;
}
#ifdef USE_ENCRYPTION
    
    len = strlen(param);
   

    if (len > PASSMAX) {
        len = PASSMAX;
        param[len] = 0;
        notice_lang(s_ChanServ, u, PASSWORD_TRUNCATED, PASSMAX);
    }

    if (encrypt(param, len, ci->founderpass, PASSMAX) < 0) {
        memset(param, 0, strlen(param));
        alog("%s: Failed to encrypt password for %s (set)", s_ChanServ,
             ci->name);
        notice_lang(s_ChanServ, u, CHAN_SET_PASSWORD_FAILED);
        return MOD_CONT;
    }

    memset(param, 0, strlen(param));
    notice_lang(s_ChanServ, u, CHAN_PASSWORD_CHANGED, ci->name);

#else
	                           /* !USE_ENCRYPTION */
    
    if (strlen(param) > PASSMAX - 1)    /* -1 for null byte */
        notice_lang(s_ChanServ, u, PASSWORD_TRUNCATED, PASSMAX - 1);
    strscpy(ci->founderpass, param, PASSMAX);
    notice_lang(s_ChanServ, u, CHAN_PASSWORD_CHANGED_TO, ci->name,
                ci->founderpass);
#endif                          /* USE_ENCRYPTION */

    if (get_access(u, ci) < ACCESS_FOUNDER) {
        alog("%s: %s!%s@%s set password as Services admin for %s",
             s_ChanServ, u->nick, u->username, GetHost(u), ci->name);
        if (WallSetpass)
            wallops(s_ChanServ,
                    "\2%s\2 set password as Services admin for channel \2%s\2",
                    u->nick, ci->name);
    } else {
        alog("%s: %s!%s@%s changed password of %s (founder: %s)",
             s_ChanServ, u->nick, u->username, GetHost(u),
             ci->name, ci->founder->display);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_desc(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;

    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	return 0;
    }

	if(!check_access(u, ci, CA_DESC))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

    	

    if (ci->desc)
        free(ci->desc);
    if(param)
	    ci->desc = sstrdup(param);
    else
	    ci->desc = "";	
    notice_lang(s_ChanServ, u, CHAN_DESC_CHANGED, ci->name, param);
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_url(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_URL))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

    if (ci->url)
        free(ci->url);
    if (param) {
        ci->url = sstrdup(param);
        notice_lang(s_ChanServ, u, CHAN_URL_CHANGED, ci->name, param);
    } else {
        ci->url = NULL;
        notice_lang(s_ChanServ, u, CHAN_URL_UNSET, ci->name);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_email(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_EMAIL))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

    if (ci->email)
        free(ci->email);
    if (param) {
        ci->email = sstrdup(param);
        notice_lang(s_ChanServ, u, CHAN_EMAIL_CHANGED, ci->name, param);
    } else {
        ci->email = NULL;
        notice_lang(s_ChanServ, u, CHAN_EMAIL_UNSET, ci->name);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_set_entrymsg(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    char *what = strtok(NULL,"\n");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_ONJOIN_SYNTAX);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_ONJOIN))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
    
    if(!param)
    {
	notice_lang(s_ChanServ,u,CHAN_ONJOIN_SYNTAX);	
	return 0;
    }
    else
    {
//    if (ci->entry_message)
	//        free(ci->entry_message);

	if (stricmp(param, "add") == 0) 
	{
        if (!what){
		notice_lang(s_ChanServ,u,CHAN_ONJOIN_SYNTAX);	
		return 0;
	}
	if ( ci->entry_message == NULL )
		ci->entry_message = sstrdup(what);
	else if ( ci->entry_message2 == NULL )
		ci->entry_message2 = sstrdup(what);
	else if ( ci->entry_message3 == NULL )
		ci->entry_message3 = sstrdup(what);
	else if ( ci->entry_message4 == NULL )
		ci->entry_message4 = sstrdup(what);
	else 
		ci->entry_message5 = sstrdup(what);
	
		
        notice_lang(s_ChanServ, u, CHAN_ENTRY_MSG_CHANGED, ci->name,
                    param);
	}
	else if(stricmp(param, "clear") == 0) 
	{
		if(what)
		{
			switch(atoi(what))
			{
				case 1:
					ci->entry_message = NULL;
					break;
				case 2:
					ci->entry_message2 = NULL;
					break;
				case 3:
					ci->entry_message3 = NULL;
					break;
				case 4:
					ci->entry_message4 = NULL;
					break;
				case 5:
					ci->entry_message5 = NULL;
					break;

			}
				
		}
		else
		{
        	ci->entry_message = NULL;
			ci->entry_message2 = NULL;
			ci->entry_message3 = NULL;
			ci->entry_message4 = NULL;
			ci->entry_message5 = NULL;
		}
        notice_lang(s_ChanServ, u, CHAN_ENTRY_MSG_UNSET, ci->name);
	}
	else
	{
	/*METTERE LA STRINGA CON LA SINTASSI*/
		notice_lang(s_ChanServ,u,CHAN_ONJOIN_SYNTAX);
	}

    }

    return MOD_CONT;
}

/**********************************************************************

Funzione COMMENT created by Pedro

**********************************************************************/

static int do_comment(User * u)
{
    char *chan = strtok(NULL," ");
    char *what = strtok(NULL,"\n");
	char *param = NULL;
    ChannelInfo *ci;
	int i =0;
	ChanAccess *access,*uaccess;
	NickCore *nc =NULL;
	

	uaccess = NULL;
    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_COMMENT))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
	
	if(u->na)
	{
	nc = u->na->nc;
	}
	for (access = ci->access, i = 0; i < ci->accesscount;
         access++, i++) {
        if (access->nc == nc) {
				uaccess = &ci->access[i];
		}
	}
		
	if(!uaccess)
	{
		notice_lang(s_ChanServ, u, CHAN_ACCESS_NOT_ALREADY_ADDED);
		return MOD_CONT;
	}
	
    
	if(!what)
	{
		uaccess->comment = "";
		notice_lang(s_ChanServ, u,  CHAN_ENTRY_MSG_UNSET, ci->name);
	}
	else
	{
		if(strlen(what) > 256)
		{
			 notice(s_ChanServ, u->nick, "Channel comments must be no longer than 256 characters!");
			 return MOD_CONT;
		}
		uaccess->comment = sstrdup(what);
		notice_lang(s_ChanServ, u, CHAN_ENTRY_MSG_CHANGED, ci->name,
                    param);
	}
   
    return MOD_CONT;
}



/*************************************************************************/

static int do_set_mlock(User * u)
{
    int add = -1;               /* 1 if adding, 0 if deleting, -1 if neither */
    unsigned char mode;
    CBMode *cbm;
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,CHAN_GUARDM_SYNTAX);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
       return 0;
    }

	if(!check_access(u, ci, CA_GUARDM))
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
    if(!param)
    {
	/* stringa giusta */
	notice_lang(s_ChanServ,u,CHAN_GUARDM_SYNTAX);
	return 0;
    }

    if (checkDefCon(DEFCON_NO_MLOCK_CHANGE)) {
        notice_lang(s_ChanServ, u, OPER_DEFCON_DENIED);
        return MOD_CONT;
    }

    /* Reinitialize everything */
#ifdef IRC_HYBRID
    ci->mlock_on = 0;
#else
    ci->mlock_on = CMODE_r;
#endif
    ci->mlock_off = ci->mlock_limit = 0;
    ci->mlock_key = NULL;
#ifdef HAS_FMODE
    ci->mlock_flood = NULL;
#endif
#ifdef HAS_LMODE
    ci->mlock_redirect = NULL;
#endif

    while ((mode = *param++)) {
        switch (mode) {
        case '+':
            add = 1;
            continue;
        case '-':
            add = 0;
            continue;
        default:
            if (add < 0)
                continue;
        }

        if ((int) mode < 128 && (cbm = &cbmodes[(int) mode])->flag != 0) {
            if ((cbm->flags & CBM_NO_MLOCK)
                || ((cbm->flags & CBM_NO_USER_MLOCK) && !is_oper(u))) {
                notice_lang(s_ChanServ, u, CHAN_SET_MLOCK_IMPOSSIBLE_CHAR,
                            mode);
            } else if (add) {
                ci->mlock_on |= cbm->flag;
                ci->mlock_off &= ~cbm->flag;
                if (cbm->cssetvalue)
                    cbm->cssetvalue(ci, strtok(NULL, " "));
            } else {
                ci->mlock_off |= cbm->flag;
                if (ci->mlock_on & cbm->flag) {
                    ci->mlock_on &= ~cbm->flag;
                    if (cbm->cssetvalue)
                        cbm->cssetvalue(ci, NULL);
                }
            }
        } else {
            notice_lang(s_ChanServ, u, CHAN_SET_MLOCK_UNKNOWN_CHAR, mode);
        }
    }                           /* while (*param) */

#ifdef HAS_LMODE
    /* We can't mlock +L if +l is not mlocked as well. */
    if ((ci->mlock_on & CMODE_L) && !(ci->mlock_on & CMODE_l)) {
        ci->mlock_on &= ~CMODE_L;
        free(ci->mlock_redirect);
        notice_lang(s_ChanServ, u, CHAN_SET_MLOCK_L_REQUIRED);
    }
#endif

#if defined(IRC_ULTIMATE) || defined(IRC_UNREAL) || defined(IRC_ULTIMATE3)
    /* We can't mlock +K if +i is not mlocked as well. */
    if ((ci->mlock_on & CMODE_K) && !(ci->mlock_on & CMODE_i)) {
        ci->mlock_on &= ~CMODE_K;
        notice_lang(s_ChanServ, u, CHAN_SET_MLOCK_K_REQUIRED);
    }
#endif

    /* Since we always enforce mode r there is no way to have no
     * mode lock at all.
     */
#if defined(IRC_HYBRID)
    /* James: Hybrid doesn't HAVE mode r, so now you have to check :P */
    if (get_mlock_modes(ci, 0))
#endif
        notice_lang(s_ChanServ, u, CHAN_MLOCK_CHANGED, ci->name,
                    get_mlock_modes(ci, 0));


    /* Implement the new lock. */
    if (ci->c)
        check_modes(ci->c);
    return MOD_CONT;
}

static int do_set_noexpire(User * u)
{
    char *chan = strtok(NULL," ");
    char *param = strtok(NULL," ");
    ChannelInfo *ci;


    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
	return 0;
    }

    
if(!check_access(u, ci, CA_NOEXPIRE))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }

    if(!param)
    {
	syntax_error(s_ChanServ, u, "NOEXPIRE",
                     CHAN_SET_NOEXPIRE_SYNTAX);
	return 0;
	}	


    if (!is_services_admin(u)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        return MOD_CONT;
    }
    if (stricmp(param, "ON") == 0) {
        ci->flags |= CI_NO_EXPIRE;
        notice_lang(s_ChanServ, u, CHAN_SET_NOEXPIRE_ON, ci->name);
    } else if (stricmp(param, "OFF") == 0) {
        ci->flags &= ~CI_NO_EXPIRE;
        notice_lang(s_ChanServ, u, CHAN_SET_NOEXPIRE_OFF, ci->name);
    } else {
        syntax_error(s_ChanServ, u, "SET NOEXPIRE",
                     CHAN_SET_NOEXPIRE_SYNTAX);
    }
    return MOD_CONT;
}

/*************************************************************************/

/* `last' is set to the last index this routine was called with
 * `perm' is incremented whenever a permission-denied error occurs
 */

static int access_del(User * u, ChanAccess * access, int *perm, int uacc)
{
    if (!access->in_use)
        return 0;
    if (!is_services_admin(u) && uacc <= access->level) {
        (*perm)++;
        return 0;
    }
    access->nc = NULL;
    access->in_use = 0;
    return 1;
}

static int access_del_callback(User * u, int num, va_list args)
{
    ChannelInfo *ci = va_arg(args, ChannelInfo *);
    int *last = va_arg(args, int *);
    int *perm = va_arg(args, int *);
    int uacc = va_arg(args, int);
    if (num < 1 || num > ci->accesscount)
        return 0;
    *last = num;
    return access_del(u, &ci->access[num - 1], perm, uacc);
}


static int access_list(User * u, int index, ChannelInfo * ci,
                       int *sent_header)
{
    ChanAccess *access = &ci->access[index];
    char *xop;

    if (!access->in_use)
        return 0;

    if (!*sent_header) {
        notice(s_ChanServ, u->nick, "Access list for %s", ci->name);
        *sent_header = 1;
    }

    if (ci->flags & CI_XOP) {
        xop = get_xop_level(access->level);
        notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_XOP_FORMAT, index + 1,
                    xop, access->nc->display);
    } else {
//        notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_AXS_FORMAT, index + 1,
//                    access->level, access->nc->display);
        notice(s_ChanServ, u->nick, "USERID:%d NICK:%s", index + 1, access->nc->display);
        if (access->level < 610) {
          if (access->level == 600)
            notice(s_ChanServ, u->nick, "ULEVEL:HELPER PLEVEL: 0 SLEVEL: 0 AUTOOP:%s",access->auto_option);
          else  
            notice(s_ChanServ, u->nick, "ULEVEL:%d PLEVEL: 0 SLEVEL: 0  AUTOOP:%s", access->level,access->auto_option);
        } else
        notice(s_ChanServ, u->nick, "ULEVEL:CSERVICE PLEVEL: 0 SLEVEL: 0 AUTOOP:%s",access->auto_option);
        notice(s_ChanServ, u->nick, "COMMENT:%s", access->comment);
        notice(s_ChanServ, u->nick, " ", access->level);

    }
    return 1;
}



static int access_list_callback(User * u, int num, va_list args)
{
    ChannelInfo *ci = va_arg(args, ChannelInfo *);
    int *sent_header = va_arg(args, int *);
    if (num < 1 || num > ci->accesscount)
        return 0;
    return access_list(u, num - 1, ci, sent_header);
}

static int do_remuser(User * u)
{
    char *chan = strtok(NULL, " ");
    char *nick = strtok(NULL, " ");
                                            
    ChannelInfo *ci;
    NickAlias *na; 
    NickCore *nc;
    ChanAccess *access;     
                                        
    int is_servadmin = is_services_admin(u);
                                                            
    int i;

    if (!chan || !nick) {
	  notice_lang(s_ChanServ,u,CHAN_REMUSER_SYNTAX);
      return MOD_CONT;
    }

    if (!(ci = cs_findchan(chan))) {
      notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
      notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_ACCESS_CHANGE)
      && !is_servadmin) {
      notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } 
    if (readonly) {
      notice_lang(s_ChanServ, u, CHAN_ACCESS_DISABLED);
      return MOD_CONT;
    }

    if (ci->accesscount == 0) {
      notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_EMPTY, chan);
      return MOD_CONT;
    }

    /* Special case: is it a number/list?  Only do search if it isn't. */
    if (isdigit(*nick) && strspn(nick, "1234567890,-") == strlen(nick)) {
      int count, deleted, last = -1, perm = 0;
      deleted = process_numlist(nick, &count, access_del_callback, u,
                                      ci, &last, &perm, get_access(u, ci));
      if (!deleted) {
        if (perm) {
          notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        } else if (count == 1) {
          notice_lang(s_ChanServ, u, CHAN_ACCESS_NO_SUCH_ENTRY,
            last, ci->name);
        } else {
          notice_lang(s_ChanServ, u, CHAN_ACCESS_NO_MATCH,
            ci->name);
        }
      } else if (deleted == 1) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_DELETED_ONE,
          ci->name);
      } else {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_DELETED_SEVERAL,
          deleted, ci->name);
      }
    } else {
      na = findnick(nick);
      if (!na) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_REGISTERED, nick);
        return MOD_CONT;
      }
      nc = na->nc;
      for (i = 0; i < ci->accesscount; i++) {
        if (ci->access[i].nc == nc)
          break;
        }
        if (i == ci->accesscount) {
          notice_lang(s_ChanServ, u, CHAN_ACCESS_NOT_FOUND, nick,
            chan);
          return MOD_CONT;
        }
        access = &ci->access[i];
        if (!is_servadmin && get_access(u, ci) <= access->level) {
          //notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        } else {
          notice_lang(s_ChanServ, u, CHAN_ACCESS_DELETED,
            access->nc->display, ci->name);
          alog("%s: %s!%s@%s (level %d) deleted access of %s (group %s) on %s", s_ChanServ, u->nick, u->username, GetHost(u), get_access(u, ci), na->nick, access->nc->display, chan);
          access->nc = NULL;
          access->in_use = 0;
        }
      }
      return MOD_CONT;
}

static int do_adduser(User * u)
{
    char *chan = strtok(NULL, " ");
    char *nick = strtok(NULL, " ");
    char *s = strtok(NULL, " ");
    
    ChannelInfo *ci;    
    NickAlias *na;
    NickCore *nc;
    ChanAccess *access;
   
    int is_servadmin = is_services_admin(u);
    
    short level = 0, ulev;
    int i;
  
    if (!chan || !nick || !s) {
	  notice_lang(s_ChanServ,u,CHAN_ADDUSER_SYNTAX);
      return MOD_CONT;
    }
    
    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_ACCESS_CHANGE)
               && !is_servadmin) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    }
    if (readonly) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_DISABLED);
        return MOD_CONT;
    }

    level = atoi(s);
    ulev = get_access(u, ci);

    if (!is_servadmin && level >= ulev) {
       // notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        return MOD_CONT;
    }

    if (level == 0) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_NONZERO);
        return MOD_CONT;
    } else if (level <= ACCESS_INVALID || level >= ACCESS_FOUNDER) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_RANGE,
                    ACCESS_INVALID + 1, ACCESS_FOUNDER - 1);
        return MOD_CONT;
    }

    na = findnick(nick);
    if (!na) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_NICKS_ONLY);
        return MOD_CONT;
    }
    if (na->status & NS_VERBOTEN) {
        notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, nick);
        return MOD_CONT;
    }

    nc = na->nc;
	for (access = ci->access, i = 0; i < ci->accesscount;
         access++, i++) {
        if (access->nc == nc) {
            /* Don't allow lowering from a level >= ulev */
            if (!is_servadmin && access->level >= ulev) {
                notice_lang(s_ChanServ, u, PERMISSION_DENIED);
                return MOD_CONT;
            }

            if (access->level == level) {
		notice_lang(s_ChanServ, u, CHAN_ACCESS_ALREADY_ADDED);
                return MOD_CONT;
            }
		notice_lang(s_ChanServ, u, CHAN_ACCESS_ALREADY_ADDED);
            return MOD_CONT;
        }
    }
    for (i = 0; i < ci->accesscount; i++) {
        if (!ci->access[i].in_use)
            break;
    }
    if (i == ci->accesscount) {
        if (i < CSAccessMax) {
            ci->accesscount++;
            ci->access =
                srealloc(ci->access,
                         sizeof(ChanAccess) * ci->accesscount);
        } else {
            notice_lang(s_ChanServ, u, CHAN_ACCESS_REACHED_LIMIT,
                        CSAccessMax);
            return MOD_CONT;
        }
    }

    access = &ci->access[i];
    access->nc = nc;
    access->in_use = 1;
    access->level = level;
    access->last_seen = 0;
	access->comment = "";
	access->auto_option="F";

    notice_lang(s_ChanServ, u, CHAN_ACCESS_ADDED, nc->display,
                ci->name, access->level);
				
	notice_lang(s_ChanServ, finduser(nick), CHAN_ACCESS_ADDED2,u->na->nc->display, nc->display,
                ci->name, access->level);

    return MOD_CONT;
}


static int do_moduser(User * u)
{
    char *chan = strtok(NULL, " ");
    char *nick = strtok(NULL, " ");
    char *s = strtok(NULL, " ");
    
    ChannelInfo *ci;    
    NickAlias *na;
    NickCore *nc;
    ChanAccess *access;
   
    int is_servadmin = is_services_admin(u);
    
    short level = 0, ulev;
    int i;
  
    if (!chan || !nick || !s) {
	  notice_lang(s_ChanServ,u,CHAN_MODUSER_SYNTAX);
      return MOD_CONT;
    }
    
    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_ACCESS_CHANGE)
               && !is_servadmin) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    }
    if (readonly) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_DISABLED);
        return MOD_CONT;
    }

    level = atoi(s);
    ulev = get_access(u, ci);

    if (!is_servadmin && level >= ulev) {
       // notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        return MOD_CONT;
    }

    if (level == 0) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_NONZERO);
        return MOD_CONT;
    } else if (level <= ACCESS_INVALID || level >= ACCESS_FOUNDER) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_RANGE,
                    ACCESS_INVALID + 1, ACCESS_FOUNDER - 1);
        return MOD_CONT;
    }

    na = findnick(nick);
    if (!na) {
        notice_lang(s_ChanServ, u, CHAN_ACCESS_NICKS_ONLY);
        return MOD_CONT;
    }
    if (na->status & NS_VERBOTEN) {
        notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, nick);
        return MOD_CONT;
    }

    nc = na->nc;
    for (access = ci->access, i = 0; i < ci->accesscount;
         access++, i++) {
        if (access->nc == nc) {
            /* Don't allow lowering from a level >= ulev */
            if (!is_servadmin && access->level >= ulev) {
                notice_lang(s_ChanServ, u, PERMISSION_DENIED);
                return MOD_CONT;
            }

            if (access->level == level) {
                notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_UNCHANGED,
                            access->nc->display, chan, level);
                return MOD_CONT;
            }
            access->level = level;
            notice_lang(s_ChanServ, u, CHAN_ACCESS_LEVEL_CHANGED,
                        access->nc->display, chan, level);
            return MOD_CONT;
        }
    }

    for (i = 0; i < ci->accesscount; i++) {
        if (!ci->access[i].in_use)
            break;
    }
    if (i == ci->accesscount) {
        if (i < CSAccessMax) {
            ci->accesscount++;
            ci->access =
                srealloc(ci->access,
                         sizeof(ChanAccess) * ci->accesscount);
        } else {
            notice_lang(s_ChanServ, u, CHAN_ACCESS_REACHED_LIMIT,
                        CSAccessMax);
            return MOD_CONT;
        }
    }
    notice_lang(s_ChanServ, u, CHAN_ACCESS_NOT_ALREADY_ADDED);
    return MOD_CONT;
}






static int do_access(User * u)
{
    char *chan = strtok(NULL, " ");
    char *nick = strtok(NULL, " ");

    
    ChannelInfo *ci;

    int i,k,j ;
    int sent_header = 0;
	
	i = k = j = 0;
    if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
    //alog("GFY: %s", count);	
    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else {
        if (ci->accesscount == 0) {
            notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_EMPTY, chan);
            return MOD_CONT;
        }
       if (nick && strspn(nick, "1234567890,-") == strlen(nick) && findnick(nick)) {
           process_numlist(nick, NULL, access_list_callback, u, ci,&sent_header);
        } else {
	  

			if(nick && !findnick(nick))
			{
				
				k = atoi(nick);
				if (k != 0)
					k = (k*-1)-1;
				if (ci->accesscount > 10)
					j = k+10;
				else
				   j= ci->accesscount;
				nick = NULL;
				
				
			}
			else if(nick && findnick(nick))
			{
				k=0;
				j=ci->accesscount;
				
			}
			else if(!nick)
			{
				k=0;
				if (ci->accesscount > 10)
					j = 10;
				else
				   j= ci->accesscount;

				
			}
			
			if (k <0 )
			{
			notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_EMPTY, chan);
            return MOD_CONT;
			}
			
			
						
			for(i=k; i < j ; i++)
			{
				
                if (nick && nick[0] == '=' && atoi(nick+1) != ci->access[i].level)
                  continue;
                if (nick && nick[0] != '=' && ci->access[i].nc
                    && !match_wild_nocase(nick, ci->access[i].nc->display))
                    continue;
                access_list(u, i, ci, &sent_header);
            }
        }
        if (!sent_header) {
            notice_lang(s_ChanServ, u, CHAN_ACCESS_NO_MATCH, chan);
        } else {
			if (j >= ci->accesscount)
	            notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_FOOTER, ci->name);
			else
			   notice_lang(s_ChanServ, u, CHAN_ACCESS_LIST_FOOTER2, j+1);
        }
    }
    return MOD_CONT;
}


/*************************************************************************/

/* Is the mask stuck? */

AutoKick *is_stuck(ChannelInfo * ci, char *mask)
{
    int i;
    AutoKick *akick;

    for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
        if (!(akick->flags & AK_USED) || (akick->flags & AK_ISNICK)
            || !(akick->flags & AK_STUCK))
            continue;
        /* Example: mask = *!*@*.org and akick->u.mask = *!*@*.epona.org */
        if (match_wild_nocase(mask, akick->u.mask))
            return akick;
#ifdef IRC_DREAMFORGE
        /* Example: mask = *!*@irc.epona.org and akick->u.mask = *!*@*.epona.org */
        if (match_wild_nocase(akick->u.mask, mask))
            return akick;
#endif
    }

    return NULL;
}

/* Ban the stuck mask in a safe manner. */

void stick_mask(ChannelInfo * ci, AutoKick * akick)
{
    int i;
    char *av[2];

    for (i = 0; i < ci->c->bancount; i++) {
        /* If akick is already covered by a wider ban.
           Example: c->bans[i] = *!*@*.org and akick->u.mask = *!*@*.epona.org */
        if (match_wild_nocase(ci->c->bans[i], akick->u.mask))
            return;

#ifdef IRC_DREAMFORGE
        /* If akick is wider than a ban already in place.
           Example: c->bans[i] = *!*@irc.epona.org and akick->u.mask = *!*@*.epona.org */
        if (match_wild_nocase(akick->u.mask, ci->c->bans[i]))
            return;
#endif
    }

    /* Falling there means set the ban */

    av[0] = sstrdup("+b");
    av[1] = akick->u.mask;
    send_cmd(whosends(ci), "MODE %s +b %s", ci->c->name, akick->u.mask);
    chan_set_modes(s_ChanServ, ci->c, 2, av, 1);
    free(av[0]);
}

/* Ban the stuck mask in a safe manner. */

void stick_all(ChannelInfo * ci)
{
    int i;
    char *av[2];
    AutoKick *akick;

    for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
        if (!(akick->flags & AK_USED) || (akick->flags & AK_ISNICK)
            || !(akick->flags & AK_STUCK))
            continue;

        av[0] = sstrdup("+b");
        av[1] = akick->u.mask;
        send_cmd(whosends(ci), "MODE %s +b %s", ci->c->name,
                 akick->u.mask);
        chan_set_modes(s_ChanServ, ci->c, 2, av, 1);
        free(av[0]);
    }
}

/* `last' is set to the last index this routine was called with */
static int akick_del(User * u, AutoKick * akick)
{
    if (!(akick->flags & AK_USED))
        return 0;
    if (akick->flags & AK_ISNICK) {
        akick->u.nc = NULL;
    } else {
        free(akick->u.mask);
        akick->u.mask = NULL;
    }
    if (akick->reason) {
        free(akick->reason);
        akick->reason = NULL;
    }
    if (akick->creator) {
        free(akick->creator);
        akick->creator = NULL;
    }
    akick->addtime = 0;
    akick->flags = 0;
    return 1;
}

static int akick_del_callback(User * u, int num, va_list args)
{
    ChannelInfo *ci = va_arg(args, ChannelInfo *);
    int *last = va_arg(args, int *);
    if (num < 1 || num > ci->akickcount)
        return 0;
    *last = num;
    return akick_del(u, &ci->akick[num - 1]);
}


static int akick_list(User * u, int index, ChannelInfo * ci,
                      int *sent_header)
{
    AutoKick *akick = &ci->akick[index];

    if (!(akick->flags & AK_USED))
        return 0;
    if (!*sent_header) {
        notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, ci->name);
        *sent_header = 1;
    }

    notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_FORMAT, index + 1,
                ((akick->flags & AK_ISNICK) ? akick->u.nc->
                 display : akick->u.mask),
                (akick->reason ? akick->
                 reason : getstring(u->na, NO_REASON)));
    return 1;
}

static int akick_list_callback(User * u, int num, va_list args)
{
    ChannelInfo *ci = va_arg(args, ChannelInfo *);
    int *sent_header = va_arg(args, int *);
    if (num < 1 || num > ci->akickcount)
        return 0;
    return akick_list(u, num - 1, ci, sent_header);
}

static int akick_view(User * u, int index, ChannelInfo * ci,
                      int *sent_header)
{
    AutoKick *akick = &ci->akick[index];
    char timebuf[64];
    struct tm tm;

    if (!(akick->flags & AK_USED))
        return 0;
    if (!*sent_header) {
        notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_HEADER, ci->name);
        *sent_header = 1;
    }

    if (akick->addtime) {
        tm = *localtime(&akick->addtime);
        strftime_lang(timebuf, sizeof(timebuf), u,
                      STRFTIME_SHORT_DATE_FORMAT, &tm);
    } else {
        snprintf(timebuf, sizeof(timebuf), getstring(u->na, UNKNOWN));
    }

    notice_lang(s_ChanServ, u,
                ((akick->
                  flags & AK_STUCK) ? CHAN_AKICK_VIEW_FORMAT_STUCK :
                 CHAN_AKICK_VIEW_FORMAT), index + 1,
                ((akick->flags & AK_ISNICK) ? akick->u.nc->
                 display : akick->u.mask),
                akick->creator ? akick->creator : getstring(u->na,
                                                            UNKNOWN),
                timebuf,
                (akick->reason ? akick->
                 reason : getstring(u->na, NO_REASON)));
    return 1;
}

static int akick_view_callback(User * u, int num, va_list args)
{
    ChannelInfo *ci = va_arg(args, ChannelInfo *);
    int *sent_header = va_arg(args, int *);
    if (num < 1 || num > ci->akickcount)
        return 0;
    return akick_view(u, num - 1, ci, sent_header);
}


static int do_akick(User * u)
{
    char *chan = strtok(NULL, " ");
    char *cmd = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    ChannelInfo *ci = NULL;
    AutoKick *akick;
    int i;

    reset_levels(ci);
    
    if (!cmd || (!mask && (!stricmp(cmd, "ADD") || !stricmp(cmd, "STICK")
                           || !stricmp(cmd, "UNSTICK")
                           || !stricmp(cmd, "DEL")))) {

        syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_AKICK) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (stricmp(cmd, "ADD") == 0) {

        NickAlias *na = findnick(mask);
        NickCore *nc = NULL;
        char *nick, *user, *host;

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (!na) {
            split_usermask(mask, &nick, &user, &host);
            mask =
                scalloc(strlen(nick) + strlen(user) + strlen(host) + 3, 1);
            sprintf(mask, "%s!%s@%s", nick, user, host);
            free(nick);
            free(user);
            free(host);
        } else {
            if (na->status & NS_VERBOTEN) {
                notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, mask);
                return MOD_CONT;
            }
            nc = na->nc;
        }

        /* Check excepts BEFORE we get this far */
#if defined (IRC_ULTIMATE) || defined(IRC_ULTIMATE3) || defined(IRC_UNREAL) || defined(IRC_VIAGRA) || defined(IRC_HYBRID)
        if (is_excepted_mask(ci, mask) == 1) {
            notice_lang(s_ChanServ, u, CHAN_EXCEPTED, mask, chan);
            return MOD_CONT;
        }
#endif

        for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
            if (!(akick->flags & AK_USED))
                continue;
            if ((akick->flags & AK_ISNICK) ? akick->u.nc == nc
                : stricmp(akick->u.mask, mask) == 0) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_ALREADY_EXISTS,
                            (akick->flags & AK_ISNICK) ? akick->u.nc->
                            display : akick->u.mask, chan);
                return MOD_CONT;
            }
        }

        for (i = 0; i < ci->akickcount; i++) {
            if (!(ci->akick[i].flags & AK_USED))
                break;
        }
        if (i == ci->akickcount) {
            if (ci->akickcount >= CSAutokickMax) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_REACHED_LIMIT,
                            CSAutokickMax);
                return MOD_CONT;
            }
            ci->akickcount++;
            ci->akick =
                srealloc(ci->akick, sizeof(AutoKick) * ci->akickcount);
        }
        akick = &ci->akick[i];
        akick->flags = AK_USED;
        if (nc) {
            akick->flags |= AK_ISNICK;
            akick->u.nc = nc;
        } else {
            akick->u.mask = mask;
        }
        akick->creator = sstrdup(u->nick);
        akick->addtime = time(NULL);
        if (reason) {
            if (strlen(reason) > 200)
                reason[200] = '\0';
            akick->reason = sstrdup(reason);
        } else {
            akick->reason = NULL;
        }
        notice_lang(s_ChanServ, u, CHAN_AKICK_ADDED, mask, chan);

    } else if (stricmp(cmd, "STICK") == 0) {
        NickAlias *na;
        NickCore *nc;

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, ci->name);
            return MOD_CONT;
        }

        na = findnick(mask);
        nc = (na ? na->nc : NULL);

        for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
            if (!(akick->flags & AK_USED) || (akick->flags & AK_ISNICK))
                continue;
            if (!stricmp(akick->u.mask, mask))
                break;
        }

        if (i == ci->akickcount) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_NOT_FOUND, mask,
                        ci->name);
            return MOD_CONT;
        }

        akick->flags |= AK_STUCK;
        notice_lang(s_ChanServ, u, CHAN_AKICK_STUCK, akick->u.mask,
                    ci->name);

        if (ci->c)
            stick_mask(ci, akick);
    } else if (stricmp(cmd, "UNSTICK") == 0) {
        NickAlias *na;
        NickCore *nc;

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, ci->name);
            return MOD_CONT;
        }

        na = findnick(mask);
        nc = (na ? na->nc : NULL);

        for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
            if (!(akick->flags & AK_USED) || (akick->flags & AK_ISNICK))
                continue;
            if (!stricmp(akick->u.mask, mask))
                break;
        }

        if (i == ci->akickcount) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_NOT_FOUND, mask,
                        ci->name);
            return MOD_CONT;
        }

        akick->flags &= ~AK_STUCK;
        notice_lang(s_ChanServ, u, CHAN_AKICK_UNSTUCK, akick->u.mask,
                    ci->name);

    } else if (stricmp(cmd, "DEL") == 0) {

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, chan);
            return MOD_CONT;
        }

        /* Special case: is it a number/list?  Only do search if it isn't. */
        if (isdigit(*mask) && strspn(mask, "1234567890,-") == strlen(mask)) {
            int count, deleted, last = -1;
            deleted = process_numlist(mask, &count, akick_del_callback, u,
                                      ci, &last);
            if (!deleted) {
                if (count == 1) {
                    notice_lang(s_ChanServ, u, CHAN_AKICK_NO_SUCH_ENTRY,
                                last, ci->name);
                } else {
                    notice_lang(s_ChanServ, u, CHAN_AKICK_NO_MATCH,
                                ci->name);
                }
            } else if (deleted == 1) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED_ONE,
                            ci->name);
            } else {
                notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED_SEVERAL,
                            deleted, ci->name);
            }
        } else {
            NickAlias *na = findnick(mask);
            NickCore *nc = (na ? na->nc : NULL);

            for (akick = ci->akick, i = 0; i < ci->akickcount;
                 akick++, i++) {
                if (!(akick->flags & AK_USED))
                    continue;
                if (((akick->flags & AK_ISNICK) && akick->u.nc == nc)
                    || (!(akick->flags & AK_ISNICK)
                        && stricmp(akick->u.mask, mask) == 0))
                    break;
            }
            if (i == ci->akickcount) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_NOT_FOUND, mask,
                            chan);
                return MOD_CONT;
            }
            notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED, mask, chan);
            akick_del(u, akick);
        }

    } else if (stricmp(cmd, "LIST") == 0) {
        int sent_header = 0;

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, chan);
            return MOD_CONT;
        }
        if (mask && isdigit(*mask) &&
            strspn(mask, "1234567890,-") == strlen(mask)) {
            process_numlist(mask, NULL, akick_list_callback, u, ci,
                            &sent_header);
        } else {
            for (akick = ci->akick, i = 0; i < ci->akickcount;
                 akick++, i++) {
                if (!(akick->flags & AK_USED))
                    continue;
                if (mask) {
                    if (!(akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.mask))
                        continue;
                    if ((akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.nc->display))
                        continue;
                }
                akick_list(u, i, ci, &sent_header);
            }
        }
        if (!sent_header)
            notice_lang(s_ChanServ, u, CHAN_AKICK_NO_MATCH, chan);

    } else if (stricmp(cmd, "VIEW") == 0) {
        int sent_header = 0;

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, chan);
            return MOD_CONT;
        }
        if (mask && isdigit(*mask) &&
            strspn(mask, "1234567890,-") == strlen(mask)) {
            process_numlist(mask, NULL, akick_view_callback, u, ci,
                            &sent_header);
        } else {
            for (akick = ci->akick, i = 0; i < ci->akickcount;
                 akick++, i++) {
                if (!(akick->flags & AK_USED))
                    continue;
                if (mask) {
                    if (!(akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.mask))
                        continue;
                    if ((akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.nc->display))
                        continue;
                }
                akick_view(u, i, ci, &sent_header);
            }
        }
        if (!sent_header)
            notice_lang(s_ChanServ, u, CHAN_AKICK_NO_MATCH, chan);

    } else if (stricmp(cmd, "ENFORCE") == 0) {
        Channel *c = findchan(ci->name);
        struct c_userlist *cu = NULL;
        struct c_userlist *next;
        char *argv[3];
        int count = 0;

        if (!c) {
            notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, ci->name);
            return MOD_CONT;
        }

        cu = c->users;

        while (cu) {
            next = cu->next;
            if (check_kick(cu->user, c->name)) {
                argv[0] = sstrdup(c->name);
                argv[1] = sstrdup(cu->user->nick);
                argv[2] = sstrdup(CSAutokickReason);

                do_kick(s_ChanServ, 3, argv);

                free(argv[2]);
                free(argv[1]);
                free(argv[0]);

                count++;
            }
            cu = next;
        }

        notice_lang(s_ChanServ, u, CHAN_AKICK_ENFORCE_DONE, chan, count);

    } else if (stricmp(cmd, "CLEAR") == 0) {

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
            if (!(akick->flags & AK_USED))
                continue;
            akick_del(u, akick);
        }

        free(ci->akick);
        ci->akick = NULL;
        ci->akickcount = 0;

        notice_lang(s_ChanServ, u, CHAN_AKICK_CLEAR);

    } else {
        syntax_error(s_ChanServ, u, "AKICK", CHAN_AKICK_SYNTAX);
    }
    return MOD_CONT;
}

static int do_unban(User * u)
{
    char *chan = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
//    char *reason = strtok(NULL, "");
    ChannelInfo *ci;
    AutoKick *akick;
    int i;

if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	
	

    if (!(ci=cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_AKICK) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (mask) {

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, chan);
            return MOD_CONT;
        }

        /* Special case: is it a number/list?  Only do search if it isn't. */
        if (isdigit(*mask) && strspn(mask, "1234567890,-") == strlen(mask)) {
            int count, deleted, last = -1;
            deleted = process_numlist(mask, &count, akick_del_callback, u,
                                      ci, &last);
            if (!deleted) {
                if (count == 1) {
                    notice_lang(s_ChanServ, u, CHAN_AKICK_NO_SUCH_ENTRY,
                                last, ci->name);
                } else {
                    notice_lang(s_ChanServ, u, CHAN_AKICK_NO_MATCH,
                                ci->name);
                }
            } else if (deleted == 1) {
				notice_lang(s_ChanServ,u,PERMISSION_OK);
                notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED_ONE,
                            ci->name);
            } else {
				notice_lang(s_ChanServ,u,PERMISSION_OK);
                notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED_SEVERAL,
                            deleted, ci->name);
            }
        } else {
            NickAlias *na = findnick(mask);
            NickCore *nc = (na ? na->nc : NULL);

            for (akick = ci->akick, i = 0; i < ci->akickcount;
                 akick++, i++) {
                if (!(akick->flags & AK_USED))
                    continue;
                if (((akick->flags & AK_ISNICK) && akick->u.nc == nc)
                    || (!(akick->flags & AK_ISNICK)
                        && stricmp(akick->u.mask, mask) == 0))
                    break;
            }
            if (i == ci->akickcount) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_NOT_FOUND, mask,
                            chan);
                return MOD_CONT;
            }
			notice_lang(s_ChanServ,u,PERMISSION_OK);
            notice_lang(s_ChanServ, u, CHAN_AKICK_DELETED, mask, chan);
            akick_del(u, akick);
        }

    }  else {
        syntax_error(s_ChanServ, u, "UNBAN", CHAN_UNBANS_SYNTAX);
    }
    return MOD_CONT;

}


static int do_ban(User * u)
{
    char *chan = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    ChannelInfo *ci;
    AutoKick *akick;
    int i;

    
   if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
    if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!check_access(u, ci, CA_AKICK) && !is_services_admin(u)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if(mask){

        NickAlias *na = findnick(mask);
        NickCore *nc = NULL;
        char *nick, *user, *host;

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }

        if (!na) {
            split_usermask(mask, &nick, &user, &host);
            mask =
                scalloc(strlen(nick) + strlen(user) + strlen(host) + 3, 1);
            sprintf(mask, "%s!%s@%s", nick, user, host);
            free(nick);
            free(user);
            free(host);
        } else {
            if (na->status & NS_VERBOTEN) {
                notice_lang(s_ChanServ, u, NICK_X_FORBIDDEN, mask);
                return MOD_CONT;
            }
            nc = na->nc;
        }

        /* Check excepts BEFORE we get this far */
#if defined (IRC_ULTIMATE) || defined(IRC_ULTIMATE3) || defined(IRC_UNREAL) || defined(IRC_VIAGRA) || defined(IRC_HYBRID)
        if (is_excepted_mask(ci, mask) == 1) {
            notice_lang(s_ChanServ, u, CHAN_EXCEPTED, mask, chan);
            return MOD_CONT;
        }
#endif

        for (akick = ci->akick, i = 0; i < ci->akickcount; akick++, i++) {
            if (!(akick->flags & AK_USED))
                continue;
            if ((akick->flags & AK_ISNICK) ? akick->u.nc == nc
                : stricmp(akick->u.mask, mask) == 0) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_ALREADY_EXISTS,
                            (akick->flags & AK_ISNICK) ? akick->u.nc->
                            display : akick->u.mask, chan);
                return MOD_CONT;
            }
        }

        for (i = 0; i < ci->akickcount; i++) {
            if (!(ci->akick[i].flags & AK_USED))
                break;
        }
        if (i == ci->akickcount) {
            if (ci->akickcount >= CSAutokickMax) {
                notice_lang(s_ChanServ, u, CHAN_AKICK_REACHED_LIMIT,
                            CSAutokickMax);
                return MOD_CONT;
            }
            ci->akickcount++;
            ci->akick =
                srealloc(ci->akick, sizeof(AutoKick) * ci->akickcount);
        }
        akick = &ci->akick[i];
        akick->flags = AK_USED;
        if (nc) {
            akick->flags |= AK_ISNICK;
            akick->u.nc = nc;
        } else {
            akick->u.mask = mask;
        }
        akick->creator = sstrdup(u->nick);
        akick->addtime = time(NULL);
        if (reason) {
            if (strlen(reason) > 200)
                reason[200] = '\0';
            akick->reason = sstrdup(reason);
        } else {
            akick->reason = NULL;
        }
        notice_lang(s_ChanServ, u, CHAN_AKICK_ADDED, mask, chan);

    
    }
    else
    {
	syntax_error(s_ChanServ, u, "BAN", CHAN_AKICK_SYNTAX);
    }
    return MOD_CONT;
}



static int do_banlist(User * u)
{
    char *chan = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
    ChannelInfo *ci;
    AutoKick *akick;
    int i;
    int sent_header = 0;

if(!chan)
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	ci = cs_findchan(chan);
	if(!(ci))
	{
		notice_lang(s_ChanServ,u,CHAN_X_NOT_REGISTERED,chan);
		return 0;
	}

        if (readonly) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_DISABLED);
            return MOD_CONT;
        }
	


        if (ci->akickcount == 0) {
            notice_lang(s_ChanServ, u, CHAN_AKICK_LIST_EMPTY, chan);
            return MOD_CONT;
        }
	if (mask && isdigit(*mask) &&
            strspn(mask, "1234567890,-") == strlen(mask)) {
            process_numlist(mask, NULL, akick_view_callback, u, ci,
                            &sent_header);
        } else {
        
            for (akick = ci->akick, i = 0; i < ci->akickcount;
                 akick++, i++) {
                if (!(akick->flags & AK_USED))
                    continue;
                if (mask) {
                    if (!(akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.mask))
                        continue;
                    if ((akick->flags & AK_ISNICK)
                        && !match_wild_nocase(mask, akick->u.nc->display))
                        continue;
                }
                akick_view(u, i, ci, &sent_header);
            }
        }
        if (!sent_header)
            notice_lang(s_ChanServ, u, CHAN_AKICK_NO_MATCH, chan);
  
    return MOD_CONT;
}



/*************************************************************************/

/* SADMINS and users, who have identified for a channel, can now cause it's
 * Enstry Message and Successor to be displayed by supplying the ALL parameter.
 * Syntax: INFO channel [ALL]
 * -TheShadow (29 Mar 1999)
 */

static int do_info(User * u)
{
    char *chan = strtok(NULL, " ");
    char *param = strtok(NULL, " ");
    ChannelInfo *ci;
    char buf[BUFSIZE], *end;
    struct tm *tm;
    int need_comma = 0;
    const char *commastr = getstring(u->na, COMMA_SPACE);
    int is_servadmin = is_services_admin(u);
    int show_all = 0;

    if (!chan) {
        syntax_error(s_ChanServ, u, "INFO", CHAN_INFO_SYNTAX);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        if (is_oper(u) && ci->forbidby)
            notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN_OPER, chan,
                        ci->forbidby,
                        (ci->forbidreason ? ci->
                         forbidreason : getstring(u->na, NO_REASON)));
        else
            notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!ci->founder) {
        /* Paranoia... this shouldn't be able to happen */
        delchan(ci);
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else {

        /* Should we show all fields? Only for sadmins and identified users */
        if (param && stricmp(param, "ALL") == 0 &&
            (check_access(u, ci, CA_INFO) || is_servadmin))
            show_all = 1;

        notice_lang(s_ChanServ, u, CHAN_INFO_HEADER, chan);
        notice_lang(s_ChanServ, u, CHAN_INFO_NO_FOUNDER,
                    ci->founder->display);

        if (ci->successor)
            notice_lang(s_ChanServ, u, CHAN_INFO_NO_SUCCESSOR,
                        ci->successor->display);

        notice_lang(s_ChanServ, u, CHAN_INFO_DESCRIPTION, ci->desc);
        tm = localtime(&ci->time_registered);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
        notice_lang(s_ChanServ, u, CHAN_INFO_TIME_REGGED, buf);
        tm = localtime(&ci->last_used);
        strftime_lang(buf, sizeof(buf), u, STRFTIME_DATE_TIME_FORMAT, tm);
        notice_lang(s_ChanServ, u, CHAN_INFO_LAST_USED, buf);
        if (ci->last_topic && (show_all || (!(ci->mlock_on & CMODE_s)
                                            && (!ci->c
                                                || !(ci->c->
                                                     mode & CMODE_s))))) {
            notice_lang(s_ChanServ, u, CHAN_INFO_LAST_TOPIC,
                        ci->last_topic);
            notice_lang(s_ChanServ, u, CHAN_INFO_TOPIC_SET_BY,
                        ci->last_topic_setter);
        }

        if (ci->entry_message && show_all)
            notice_lang(s_ChanServ, u, CHAN_INFO_ENTRYMSG,
                        ci->entry_message);
						
		if (ci->entry_message2 && show_all)
            notice_lang(s_ChanServ, u, CHAN_INFO_ENTRYMSG,
                        ci->entry_message2);				
						
        if (ci->url && show_all)
            notice_lang(s_ChanServ, u, CHAN_INFO_URL, ci->url);
        if (ci->email && show_all)
            notice_lang(s_ChanServ, u, CHAN_INFO_EMAIL, ci->email);

        if (show_all) {
           // notice_lang(s_ChanServ, u, CHAN_INFO_BANTYPE, ci->bantype);

            end = buf;
            *end = 0;
            if (ci->flags & CI_KEEPTOPIC) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s",
                                getstring(u->na, CHAN_INFO_OPT_KEEPTOPIC));
                need_comma = 1;
            }
            if (ci->flags & CI_OPNOTICE) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_OPNOTICE));
                need_comma = 1;
            }
            if (ci->flags & CI_PRIVATE) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_PRIVATE));
                need_comma = 1;
            }
            if (ci->flags & CI_RESTRICTED) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na,
                                          CHAN_INFO_OPT_RESTRICTED));
                need_comma = 1;
            }
            if (ci->flags & CI_SECURE) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_SECURE));
                need_comma = 1;
            }
            if (ci->flags & CI_SECUREOPS) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_SECUREOPS));
                need_comma = 1;
            }
            //if (ci->flags & CI_SECUREFOUNDER) {
              //  end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                //                need_comma ? commastr : "",
                  //              getstring(u->na,
                    //                      CHAN_INFO_OPT_SECUREFOUNDER));
               // need_comma = 1;
           // }
            //if ((ci->flags & CI_SIGNKICK)
              //  || (ci->flags & CI_SIGNKICK_LEVEL)) {
                //end +=
                  //  snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                    //         need_comma ? commastr : "", getstring(u->na,
                                                           //        CHAN_INFO_OPT_SIGNKICK));
                //need_comma = 1;
            //}
            if (ci->flags & CI_TOPICLOCK) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_TOPICLOCK));
                need_comma = 1;
            }
            if (ci->flags & CI_XOP) {
                end += snprintf(end, sizeof(buf) - (end - buf), "%s%s",
                                need_comma ? commastr : "",
                                getstring(u->na, CHAN_INFO_OPT_XOP));
                need_comma = 1;
            }
            notice_lang(s_ChanServ, u, CHAN_INFO_OPTIONS,
                        *buf ? buf : getstring(u->na, CHAN_INFO_OPT_NONE));
            notice_lang(s_ChanServ, u, CHAN_INFO_MODE_LOCK,
                        get_mlock_modes(ci, 1));

        }

        if ((ci->flags & CI_NO_EXPIRE) && show_all)
            notice_lang(s_ChanServ, u, CHAN_INFO_NO_EXPIRE);
        if (ci->flags & CI_SUSPENDED) {
            notice_lang(s_ChanServ, u, CHAN_X_SUSPENDED, ci->forbidby,
                        (ci->forbidreason ? ci->
                         forbidreason : getstring(u->na, NO_REASON)));
        }

        if (!show_all && (check_access(u, ci, CA_INFO) || is_servadmin))
            notice_lang(s_ChanServ, u, NICK_INFO_FOR_MORE, s_ChanServ,
                        ci->name);

    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_list(User * u)
{
    char *pattern = strtok(NULL, " ");
    ChannelInfo *ci;
    int nchans, i;
    char buf[BUFSIZE];
    int is_servadmin = is_services_admin(u);
    int count = 0, from = 0, to = 0;
    char *tmp = NULL;
    char *s = NULL;
    char *keyword;
    int32 matchflags = 0;


    if (CSListOpersOnly && (!u || !is_oper(u))) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
        return MOD_CONT;
    }

    if (!pattern) {
        syntax_error(s_ChanServ, u, "LIST",
                     is_servadmin ? CHAN_LIST_SERVADMIN_SYNTAX :
                     CHAN_LIST_SYNTAX);
    } else {

        if (pattern) {
            if (pattern[0] == '#') {
                tmp = myStrGetOnlyToken((pattern + 1), '-', 0); /* Read FROM out */
                if (!tmp) {
                    return MOD_CONT;
                }
                for (s = tmp; *s; s++) {
                    if (!isdigit(*s)) {
                        return MOD_CONT;
                    }
                }
                from = atoi(tmp);
                tmp = myStrGetTokenRemainder(pattern, '-', 1);  /* Read TO out */
                if (!tmp) {
                    return MOD_CONT;
                }
                for (s = tmp; *s; s++) {
                    if (!isdigit(*s)) {
                        return MOD_CONT;
                    }
                }
                to = atoi(tmp);
                pattern = sstrdup("*");
            }
        }

        nchans = 0;

        while (is_servadmin && (keyword = strtok(NULL, " "))) {
            if (stricmp(keyword, "FORBIDDEN") == 0)
                matchflags |= CI_VERBOTEN;
            if (stricmp(keyword, "SUSPENDED") == 0)
                matchflags |= CI_SUSPENDED;
            if (stricmp(keyword, "NOEXPIRE") == 0)
                matchflags |= CI_NO_EXPIRE;
        }

        notice_lang(s_ChanServ, u, CHAN_LIST_HEADER, pattern);
        for (i = 0; i < 256; i++) {
            for (ci = chanlists[i]; ci; ci = ci->next) {
                if (!is_servadmin && ((ci->flags & CI_PRIVATE)
                                      || (ci->flags & CI_VERBOTEN)))
                    continue;
                if ((matchflags != 0) && !(ci->flags & matchflags))
                    continue;

                if (stricmp(pattern, ci->name) == 0
                    || match_wild_nocase(pattern, ci->name)) {
                    if ((((count + 1 >= from) && (count + 1 <= to))
                         || ((from == 0) && (to == 0)))
                        && (++nchans <= CSListMax)) {
                        char noexpire_char = ' ';
                        if (is_servadmin && (ci->flags & CI_NO_EXPIRE))
                            noexpire_char = '!';

                        if (ci->flags & CI_VERBOTEN) {
                            snprintf(buf, sizeof(buf),
                                     "%-20s  [Forbidden]", ci->name);
                        } else if (ci->flags & CI_SUSPENDED) {
                            snprintf(buf, sizeof(buf),
                                     "%-20s  [Suspended]", ci->name);
                        } else {
                            snprintf(buf, sizeof(buf), "%-20s  %s",
                                     ci->name, ci->desc ? ci->desc : "");
                        }

                        notice_user(s_ChanServ, u, "  %c%s",
                                    noexpire_char, buf);
                    }
                    count++;
                }
            }
        }
        notice_lang(s_ChanServ, u, CHAN_LIST_END,
                    nchans > CSListMax ? CSListMax : nchans, nchans);
    }
    return MOD_CONT;

}

/*************************************************************************/

static int do_invite(User * u)
{
    char *chan = strtok(NULL, " ");
	char *nick = strtok(NULL, " ");
    Channel *c;
    ChannelInfo *ci;

    if (!chan) {
        syntax_error(s_ChanServ, u, "INVITE", CHAN_INVITE_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (!u || !check_access(u, ci, CA_INVITE)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {
		if(!nick)
		{
        send_cmd(whosends(c->ci), "INVITE %s %s", u->nick, chan);
          notice_lang(s_ChanServ, u, PERMISSION_OK);
		}
		else
		{
			send_cmd(whosends(c->ci), "INVITE %s %s", nick, chan);
		}
    }
    return MOD_CONT;
}

/*************************************************************************/

/* do_util: not a command, but does the job of other */

static int do_util(User * u, CSModeUtil * util)
{
    char *av[2];
    char *chan = strtok(NULL, " ");
    char *nick[256]; /*max 256 utenti passati*/
	int i = 0;
	int k = 0;
	int flag = 0;
	
    Channel *c;
    ChannelInfo *ci;
    User *u2;
	 int is_same;

	
	if (!chan)
	{
	 notice_lang(s_ChanServ, u, ACCESS_DENIED);
	 return 0;
	 }
	nick[i] = strtok(NULL, " ");
    while ( nick[i] != NULL)
  	{	
		
		//notice(s_ChanServ, u->nick, "NICK:%s",nick[i]);
		i = i + 1;
	    nick[i] = strtok (NULL, " ");
	
  	}


	for (k = 0  ; k < i || i == 0;k++)
	{
	
    if (!chan) {
        struct u_chanlist *uc;

        av[0] = util->mode;
        av[1] = u->nick;

        /* Sets the mode to the user on every channels he is on. */
		
        for (uc = u->chans; uc; uc = uc->next) {
            if ((ci = uc->chan->ci) && !(ci->flags & CI_VERBOTEN)
                && check_access(u, ci, util->levelself)) {
				flag = 1;
                send_cmd(whosends(ci), "MODE %s %s %s", uc->chan->name,
                         util->mode, u->nick);
                chan_set_modes(s_ChanServ, uc->chan, 2, av, 1);

                if (util->notice && ci->flags & util->notice)
                    notice(whosends(ci), chan,
                           "%s command used for %s by %s", util->name,
                           u->nick, u->nick);
            }
        }
		//notice_lang(s_ChanServ,u,PERMISSION_OK);

        return MOD_CONT;
    } else if (!nick[0]) {
		i = i +1;
        nick[0] = u->nick;
    }

    is_same = (nick[k] == u->nick) ? 1 : (stricmp(nick[k], u->nick) == 0);
	
    if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, ci->name);
    } else if (is_same ? !(u2 = u) : !(u2 = finduser(nick[k]))) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, nick[k]);
    } else if (!is_on_chan(c, u2)) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_ON_CHAN, u2->nick, c->name);
    } else if (is_same ? !check_access(u, ci, util->levelself) :
               !check_access(u, ci, util->level)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (*util->mode == '-' && !is_same && (ci->flags & CI_PEACE)
               && (get_access(u2, ci) >= get_access(u, ci))) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
#if defined (IRC_ULTIMATE) || defined (IRC_ULTIMATE3)
    } else if (*util->mode == '-' && is_protected(u2)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
#endif
    } else {
        send_cmd(whosends(ci), "MODE %s %s %s", c->name, util->mode,
                 u2->nick);
				 		flag = 1;

        av[0] = util->mode;
        av[1] = u2->nick;
        chan_set_modes(s_ChanServ, c, 2, av, 1);

        if (util->notice && ci->flags & util->notice)
            notice(whosends(ci), c->name, "%s command used for %s by %s",
                   util->name, u2->nick, u->nick);
		
    }
	}
	if (flag == 1) 
	notice_lang(s_ChanServ,u,PERMISSION_OK);
    return MOD_CONT;
}

/*************************************************************************/

static int do_op(User * u)
{

    return do_util(u, &csmodeutils[MUT_OP]);
}


/*************************************************************************/

static int do_deop(User * u)
{
    return do_util(u, &csmodeutils[MUT_DEOP]);
}

/*************************************************************************/

static int do_voice(User * u)
{
    return do_util(u, &csmodeutils[MUT_VOICE]);
}


/*************************************************************************/

static int do_devoice(User * u)
{

    return do_util(u, &csmodeutils[MUT_DEVOICE]);
}

/*************************************************************************/

#ifdef HAS_HALFOP

static int do_halfop(User * u)
{
    return do_util(u, &csmodeutils[MUT_HALFOP]);
}

/*************************************************************************/

static int do_dehalfop(User * u)
{
    return do_util(u, &csmodeutils[MUT_DEHALFOP]);
}

#endif

/*************************************************************************/

#if defined(IRC_UNREAL) || defined(IRC_ULTIMATE3) || defined(IRC_VIAGRA)

static int do_protect(User * u)
{
    return do_util(u, &csmodeutils[MUT_PROTECT]);
}

/*************************************************************************/

static int do_deprotect(User * u)
{
    return do_util(u, &csmodeutils[MUT_DEPROTECT]);
}

/*************************************************************************/
#endif

#if defined(IRC_UNREAL) || defined(IRC_VIAGRA)

static int do_owner(User * u)
{
    char *av[2];
    char *chan = strtok(NULL, " ");

    Channel *c;
    ChannelInfo *ci;

    if (!chan) {
        struct u_chanlist *uc;

        av[0] = sstrdup("+q");
        av[1] = u->nick;

        /* Sets the mode to the user on every channels he is on. */

        for (uc = u->chans; uc; uc = uc->next) {
            if ((ci = uc->chan->ci) && !(ci->flags & CI_VERBOTEN)
                && is_founder(u, ci)) {
                send_cmd(whosends(ci), "MODE %s %s %s", uc->chan->name,
                         av[0], u->nick);
                chan_set_modes(s_ChanServ, uc->chan, 2, av, 1);
            }
        }

        free(av[0]);
        return MOD_CONT;
    }

    if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, ci->name);
    } else if (!is_on_chan(c, u)) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_ON_CHAN, u->nick, c->name);
    } else if (!is_founder(u, ci)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else {
        send_cmd(whosends(ci), "MODE %s +q %s", c->name, u->nick);

        av[0] = sstrdup("+q");
        av[1] = u->nick;
        chan_set_modes(s_ChanServ, c, 2, av, 1);
        free(av[0]);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_deowner(User * u)
{
    char *av[2];
    char *chan = strtok(NULL, " ");

    Channel *c;
    ChannelInfo *ci;

    if (!chan) {
        struct u_chanlist *uc;

        av[0] = sstrdup("-q");
        av[1] = u->nick;

        /* Sets the mode to the user on every channels he is on. */

        for (uc = u->chans; uc; uc = uc->next) {
            if ((ci = uc->chan->ci) && !(ci->flags & CI_VERBOTEN)
                && is_founder(u, ci)) {
                send_cmd(whosends(ci), "MODE %s %s %s", uc->chan->name,
                         av[0], u->nick);
                chan_set_modes(s_ChanServ, uc->chan, 2, av, 1);
            }
        }

        free(av[0]);
        return MOD_CONT;
    }

    if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, ci->name);
    } else if (!is_on_chan(c, u)) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_ON_CHAN, u->nick, c->name);
    } else if (!is_founder(u, ci)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else {
        send_cmd(whosends(ci), "MODE %s -q %s", c->name, u->nick);

        av[0] = sstrdup("-q");
        av[1] = u->nick;
        chan_set_modes(s_ChanServ, c, 2, av, 1);
        free(av[0]);
    }
    return MOD_CONT;
}

#endif

/*************************************************************************/

static int do_cs_kick(User * u)
{
    char *chan = strtok(NULL, " ");
    char *params = strtok(NULL, " ");
    char *reason = strtok(NULL, "");

    Channel *c;
    ChannelInfo *ci;
    User *u2;

    int is_same;

    if (!reason) {
        reason = "Requested";
    } else {
        if (strlen(reason) > 200)
            reason[200] = '\0';
    }

    if (!chan) {
        struct u_chanlist *uc, *next;

        /* Kicks the user on every channels he is on. */

        for (uc = u->chans; uc; uc = next) {
            next = uc->next;
            if ((ci = uc->chan->ci) && !(ci->flags & CI_VERBOTEN)
                && check_access(u, ci, CA_KICKME)) {
                char *av[3];

                if ((ci->flags & CI_SIGNKICK)
                    || ((ci->flags & CI_SIGNKICK_LEVEL)
                        && !check_access(u, ci, CA_KICK)))
                    send_cmd(whosends(ci), "KICK %s %s :%s (%s)", ci->name,
                             u->nick, reason, u->nick);
                else
                    send_cmd(whosends(ci), "KICK %s %s :%s", ci->name,
                             u->nick, reason);
                av[0] = ci->name;
                av[1] = u->nick;
                av[2] = reason;
                do_kick(s_ChanServ, 3, av);
            }
        }

        return MOD_CONT;
    } else if (!params) {
        params = u->nick;
    }

    is_same = (params == u->nick) ? 1 : (stricmp(params, u->nick) == 0);

    if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (is_same ? !(u2 = u) : !(u2 = finduser(params))) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_IN_USE, params);
    } else if (!is_on_chan(c, u2)) {
        notice_lang(s_ChanServ, u, NICK_X_NOT_ON_CHAN, u2->nick, c->name);
    } else if (!is_same ? !check_access(u, ci, CA_KICK) :
               !check_access(u, ci, CA_KICKME)) {
        notice_lang(s_ChanServ, u, ACCESS_DENIED);
    } else if (!is_same && (ci->flags & CI_PEACE)
               && (get_access(u2, ci) >= get_access(u, ci))) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
#if defined (IRC_ULTIMATE) || defined (IRC_ULTIMATE3)
    } else if (is_protected(u2)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
#endif
    } else {
        char *av[3];

        if ((ci->flags & CI_SIGNKICK)
            || ((ci->flags & CI_SIGNKICK_LEVEL)
                && !check_access(u, ci, CA_SIGNKICK)))
            send_cmd(whosends(ci), "KICK %s %s :%s (%s)", ci->name, params,
                     reason, u->nick);
        else
            send_cmd(whosends(ci), "KICK %s %s :%s", ci->name, params,
                     reason);
        av[0] = ci->name;
        av[1] = params;
        av[2] = reason;
        do_kick(s_ChanServ, 3, av);
    }
    return MOD_CONT;
}


/*************************************************************************/

static int do_cs_topic(User * u)
{
    char *chan = strtok(NULL, " ");
    char *topic = strtok(NULL, "");

    Channel *c;
    ChannelInfo *ci;
    
    if (!chan) {
        syntax_error(s_ChanServ, u, "TOPIC", CHAN_TOPIC_SYNTAX);
    } else if (!(c = findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_IN_USE, chan);
    } else if (!(ci = c->ci)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, c->name);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, ci->name);
    } else if (!is_services_admin(u) && !check_access(u, ci, CA_TOPIC)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {

        if (ci->last_topic)
            free(ci->last_topic);
        ci->last_topic = topic ? sstrdup(topic) : NULL;
        strscpy(ci->last_topic_setter, u->nick, NICKMAX);
        ci->last_topic_time = time(NULL);

        if (c->topic)
            free(c->topic);
        c->topic = topic ? sstrdup(topic) : NULL;
        strscpy(c->topic_setter, u->nick, NICKMAX);
#if defined(IRC_DREAMFORGE) && !defined(IRC_ULTIMATE) && !defined(IRC_UNREAL)
        c->topic_time = c->topic_time - 1;
#else
        c->topic_time = ci->last_topic_time;
#endif

        if (is_services_admin(u))
            alog("%s: %s!%s@%s changed topic of %s as services admin.",
                 s_ChanServ, u->nick, u->username, u->host, c->name);
#ifdef IRC_HYBRID
        if (whosends(ci) == s_ChanServ) {
            send_cmd(NULL, "SJOIN %ld %s + :%s", time(NULL), c->name,
                     s_ChanServ);
            send_cmd(NULL, "MODE %s +o %s", c->name, s_ChanServ);

        }
        send_cmd(whosends(ci), "TOPIC %s :%s", c->name,
                 c->topic ? c->topic : "");
		notice_lang(s_ChanServ,u,PERMISSION_OK);		
        if (whosends(ci) == s_ChanServ) {
            send_cmd(s_ChanServ, "PART %s", c->name);
        }
#else
        send_cmd(whosends(ci), "TOPIC %s %s %lu :%s", c->name, u->nick,
                 c->topic_time, topic ? topic : "");
				 					notice_lang(s_ChanServ,u,PERMISSION_OK);
#endif
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_getpass(User * u)
{
#ifndef USE_ENCRYPTION
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
#endif

    /* Assumes that permission checking has already been done. */
#ifdef USE_ENCRYPTION
    notice_lang(s_ChanServ, u, CHAN_GETPASS_UNAVAILABLE);
#else
    if (!chan) {
        syntax_error(s_ChanServ, u, "GETPASS", CHAN_GETPASS_SYNTAX);
    } else if (!(ci = cs_findchan(chan))) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else if (CSRestrictGetPass && !is_services_root(u)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else {
        alog("%s: %s!%s@%s used GETPASS on %s",
             s_ChanServ, u->nick, u->username, GetHost(u), ci->name);
        if (WallGetpass) {
            //wallops(s_ChanServ, "\2%s\2 used GETPASS on channel \2%s\2",
              //      u->nick, chan);
        }
        notice_lang(s_ChanServ, u, CHAN_GETPASS_PASSWORD_IS,
                    chan, ci->founderpass);
    }
#endif
    return MOD_CONT;
}

/*************************************************************************/

static int do_sendpass(User * u)
{
#ifndef USE_ENCRYPTION
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    NickCore *founder;
#endif

#ifdef USE_ENCRYPTION
    notice_lang(s_ChanServ, u, CHAN_SENDPASS_UNAVAILABLE);
#else
    
    if(!check_access(u, ci, CA_GENPASS))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
   
    if (!chan) {
        syntax_error(s_ChanServ, u, "GENPASS", CHAN_SENDPASS_SYNTAX);
    } else if (RestrictMail && !is_oper(u)) {
        notice_lang(s_ChanServ, u, PERMISSION_DENIED);
    } else if (!(ci = cs_findchan(chan)) || !(founder = ci->founder)) {
        notice_lang(s_ChanServ, u, CHAN_X_NOT_REGISTERED, chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_lang(s_ChanServ, u, CHAN_X_FORBIDDEN, chan);
    } else {
        char buf[BUFSIZE];
        MailInfo *mail;

        snprintf(buf, sizeof(buf),
                 getstring2(founder, CHAN_SENDPASS_SUBJECT), ci->name);
        mail = MailBegin(u, founder, buf, s_ChanServ);
        if (!mail)
            return MOD_CONT;

        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_HEAD));
        fprintf(mail->pipe, "\n\n");
        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_LINE_1),
                ci->name);
        fprintf(mail->pipe, "\n\n");
        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_LINE_2),
                ci->founderpass);
        fprintf(mail->pipe, "\n\n");
        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_LINE_3));
        fprintf(mail->pipe, "\n\n");
        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_LINE_4));
        fprintf(mail->pipe, "\n\n");
        fprintf(mail->pipe, getstring2(founder, CHAN_SENDPASS_LINE_5),
                NetworkName);
        fprintf(mail->pipe, "\n.\n");

        MailEnd(mail);

        alog("%s: %s!%s@%s used SENDPASS on %s", s_ChanServ, u->nick,
             u->username, GetHost(u), chan);
        notice_lang(s_ChanServ, u, CHAN_SENDPASS_OK, chan);
    }
#endif
    return MOD_CONT;
}

/*************************************************************************/

static int do_forbid(User * u)
{
    ChannelInfo *ci;
    char *chan = strtok(NULL, " ");
    char *reason = strtok(NULL, "");

    Channel *c;

    /* Assumes that permission checking has already been done. */
    if (!chan || (ForceForbidReason && !reason)) {
        syntax_error(s_ChanServ, u, "FORBID",
                     (ForceForbidReason ? CHAN_FORBID_SYNTAX_REASON :
                      CHAN_FORBID_SYNTAX));
        return MOD_CONT;
    }
    if (readonly)
        notice_lang(s_ChanServ, u, READ_ONLY_MODE);
    if ((ci = cs_findchan(chan)) != NULL)
        delchan(ci);
    ci = makechan(chan);
	if(!check_access(u, ci, CA_FORBID))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
    if (ci) {
        ci->flags |= CI_VERBOTEN;
        ci->forbidby = sstrdup(u->nick);
        if (reason)
            ci->forbidreason = sstrdup(reason);

        if ((c = findchan(ci->name))) {
            struct c_userlist *cu, *next;
            char *av[3];

            for (cu = c->users; cu; cu = next) {
                next = cu->next;

                if (is_oper(cu->user))
                    continue;

                av[0] = c->name;
                av[1] = cu->user->nick;
                av[2] = reason ? reason : "CHAN_FORBID_REASON";
                send_cmd(s_ChanServ, "KICK %s %s :%s", av[0], av[1],
                         av[2]);
                do_kick(s_ChanServ, 3, av);
            }
        }

        if (WallForbid)
            //wallops(s_ChanServ, "\2%s\2 used FORBID on channel \2%s\2",
              //      u->nick, ci->name);

        alog("%s: %s set FORBID for channel %s", s_ChanServ, u->nick,
             ci->name);
        notice_lang(s_ChanServ, u, CHAN_FORBID_SUCCEEDED, chan);
    } else {
        alog("%s: Valid FORBID for %s by %s failed", s_ChanServ, ci->name,
             u->nick);
        notice_lang(s_ChanServ, u, CHAN_FORBID_FAILED, chan);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_suspend(User * u)
{
    ChannelInfo *ci;
    char *chan = strtok(NULL, " ");
    char *reason = strtok(NULL, "");

    Channel *c;

    /* Assumes that permission checking has already been done. */
    if (!chan || (ForceForbidReason && !reason)) {
        syntax_error(s_ChanServ, u, "SCHAN",
                     (ForceForbidReason ? CHAN_SUSPEND_SYNTAX_REASON :
                      CHAN_SUSPEND_SYNTAX));
        return MOD_CONT;
    }
    if (readonly)
        notice_lang(s_ChanServ, u, READ_ONLY_MODE);
    if ((ci = cs_findchan(chan)) == NULL)
        ci = makechan(chan);
	
	if(!check_access(u, ci, CA_SCHAN))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}
	
    if (ci) {
		
        ci->flags |= CI_SUSPENDED;
        ci->forbidby = sstrdup(u->nick);
        if (reason)
            ci->forbidreason = sstrdup(reason);

        if ((c = findchan(ci->name))) {
            struct c_userlist *cu, *next;
            char *av[3];

            for (cu = c->users; cu; cu = next) {
                next = cu->next;

                if (is_oper(cu->user))
                    continue;

                av[0] = c->name;
                av[1] = cu->user->nick;
                av[2] = reason ? reason : "CHAN_SUSPEND_REASON";
                send_cmd(s_ChanServ, "KICK %s %s :%s", av[0], av[1],
                         av[2]);
                do_kick(s_ChanServ, 3, av);
            }
        }

        if (WallForbid)
           // wallops(s_ChanServ, "\2%s\2 used SUSPEND on channel \2%s\2",
           //         u->nick, ci->name);

        alog("%s: %s set SUSPEND for channel %s", s_ChanServ, u->nick,
             ci->name);
        notice_lang(s_ChanServ, u, CHAN_SUSPEND_SUCCEEDED, chan);
    } else {
        alog("%s: Valid SUSPEND for %s by %s failed", s_ChanServ, ci->name,
             u->nick);
        notice_lang(s_ChanServ, u, CHAN_SUSPEND_FAILED, chan);
    }
    return MOD_CONT;
}

static int do_suspends(User * u)
{
	return 0;    
}

static int do_chanlimit(User * u)                                                
{
        return 0;   
}



/*************************************************************************/

static int do_unsuspend(User * u)
{
    ChannelInfo *ci;
    char *chan = strtok(NULL, " ");

    /* Assumes that permission checking has already been done. */
    if (!chan) {
        syntax_error(s_ChanServ, u, "UNSCHAN", CHAN_UNSUSPEND_SYNTAX);
        return MOD_CONT;
    }
    if (chan[0] != '#') {
        notice_lang(s_ChanServ, u, CHAN_UNSUSPEND_ERROR);
        return MOD_CONT;
    }
    if (readonly)
        notice_lang(s_ChanServ, u, READ_ONLY_MODE);

    ci = cs_findchan(chan);
	
	if(!check_access(u, ci, CA_UNSCHAN))
	{
	notice_lang(s_ChanServ,u,ACCESS_DENIED);
	return 0;	
	}

    if (ci) {
        ci->flags &= ~CI_SUSPENDED;
        ci->forbidreason = NULL;
        ci->forbidby = NULL;

        if (WallForbid)
            //wallops(s_ChanServ, "\2%s\2 used UNSUSPEND on channel \2%s\2",
             //       u->nick, ci->name);

        alog("%s: %s set UNSUSPEND for channel %s", s_ChanServ, u->nick,
             ci->name);
        notice_lang(s_ChanServ, u, CHAN_UNSUSPEND_SUCCEEDED, chan);
    } else {
        alog("%s: Valid UNSUSPEND for %s by %s failed", s_ChanServ,
             chan, u->nick);
        notice_lang(s_ChanServ, u, CHAN_UNSUSPEND_FAILED, chan);
    }
    return MOD_CONT;
}

/*************************************************************************/

static int do_status(User * u)
{
    ChannelInfo *ci;
    User *u2;
    char *nick, *chan;

    chan = strtok(NULL, " ");
    nick = strtok(NULL, " ");
    if (!nick || strtok(NULL, " ")) {
        notice_user(s_ChanServ, u, "STATUS ERROR Syntax error");
        return MOD_CONT;
    }
    if (!(ci = cs_findchan(chan))) {
        char *temp = chan;
        chan = nick;
        nick = temp;
        ci = cs_findchan(chan);
    }

if(!check_access(u, ci, CA_STATUS))  
        {
        notice_lang(s_ChanServ,u,ACCESS_DENIED);
        return 0;
        }
    if (!ci) {
        notice_user(s_ChanServ, u,
                    "STATUS ERROR Channel %s not registered", chan);
    } else if (ci->flags & CI_VERBOTEN) {
        notice_user(s_ChanServ, u, "STATUS ERROR Channel %s forbidden",
                    chan);
        return MOD_CONT;
    } else if ((u2 = finduser(nick)) != NULL) {
        notice_user(s_ChanServ, u, "STATUS %s %s %d", chan, nick,
                    get_access(u2, ci));
    } else {                    /* !u2 */
        notice_user(s_ChanServ, u, "STATUS ERROR Nick %s not online",
                    nick);
    }
    return MOD_CONT;
}

#ifdef USE_MODULES

int do_modload(User * u)
{
    char *name;
    Module *m;

    name = strtok(NULL, "");
    if (!name) {
        syntax_error(s_ChanServ, u, "MODLOAD", OPER_MODULE_LOAD_SYNTAX);
        return MOD_CONT;
    }
    m = findModule(name);
    if (!m) {
        m = createModule(name);
        mod_current_module = m;
        mod_current_user = u;
        mod_current_op = 1;
    } else {
        notice_lang(s_ChanServ, u, OPER_MODULE_LOAD_FAIL, name);
    }
    return MOD_CONT;
}

int do_modunload(User * u)
{
    char *name;
    Module *m;

    name = strtok(NULL, "");
    if (!name) {
        syntax_error(s_ChanServ, u, "MODUNLOAD",
                     OPER_MODULE_UNLOAD_SYNTAX);
        return MOD_CONT;
    }
    m = findModule(name);
    if (m) {
        mod_current_user = u;
        mod_current_module = m;
        mod_current_op = 2;
    } else {
        notice_lang(s_ChanServ, u, OPER_MODULE_REMOVE_FAIL, name);
    }
    return MOD_CONT;
}

int do_modlist(User * u)
{
    int idx;
    int count = 0;
    struct tm tm;
    char timebuf[64];
    ModuleHash *current = NULL;

    for (idx = 0; idx != MAX_CMD_HASH; idx++) {
        for (current = MODULE_HASH[idx]; current; current = current->next) {
            tm = *localtime(&current->m->time);

            strftime_lang(timebuf, sizeof(timebuf), u,
                          STRFTIME_DATE_TIME_FORMAT, &tm);
            notice_lang(s_ChanServ, u, OPER_MODULE_LIST, current->name);
            count++;
        }
    }
    if (count == 0) {
        notice_lang(s_ChanServ, u, OPER_MODULE_NO_LIST);
    }

    return MOD_CONT;
}

int do_modinfo(User * u)
{
    char *file;
    struct tm tm;
    char timebuf[64];
    Module *m;
    int idx = 0;
    int display = 0;

    file = strtok(NULL, "");
    if (!file) {
        syntax_error(s_ChanServ, u, "MODINFO", OPER_MODULE_INFO_SYNTAX);
        return MOD_CONT;
    }
    m = findModule(file);
    if (m) {
        tm = *localtime(&m->time);
        strftime_lang(timebuf, sizeof(timebuf), u,
                      STRFTIME_DATE_TIME_FORMAT, &tm);
        notice_lang(s_ChanServ, u, OPER_MODULE_INFO_LIST, m->name,
                    m->version ? m->version : "?",
                    m->author ? m->author : "?", timebuf);
        for (idx = 0; idx < MAX_CMD_HASH; idx++) {
            display += showModuleCmdLoaded(HOSTSERV[idx], m->name, u);
            display += showModuleCmdLoaded(OPERSERV[idx], m->name, u);
            display += showModuleCmdLoaded(NICKSERV[idx], m->name, u);
            display += showModuleCmdLoaded(CHANSERV[idx], m->name, u);
            display += showModuleCmdLoaded(BOTSERV[idx], m->name, u);
            display += showModuleCmdLoaded(MEMOSERV[idx], m->name, u);
            display += showModuleCmdLoaded(HELPSERV[idx], m->name, u);
            display += showModuleMsgLoaded(IRCD[idx], m->name, u);

        }
    }
    if (display == 0) {
        notice_lang(s_ChanServ, u, OPER_MODULE_NO_INFO, file);
    }
    return MOD_CONT;
}

static int showModuleCmdLoaded(CommandHash * cmdList, char *mod_name,
                               User * u)
{
    Command *c;
    CommandHash *current;
    int display = 0;

    for (current = cmdList; current; current = current->next) {
        for (c = current->c; c; c = c->next) {
            if ((c->mod_name) && (stricmp(c->mod_name, mod_name) == 0)) {
                notice_lang(s_ChanServ, u, OPER_MODULE_CMD_LIST,
                            c->service, c->name);
                display++;
            }
        }
    }
    return display;
}

static int showModuleMsgLoaded(MessageHash * msgList, char *mod_name,
                               User * u)
{
    Message *msg;
    MessageHash *mcurrent;
    int display = 0;
    for (mcurrent = msgList; mcurrent; mcurrent = mcurrent->next) {
        for (msg = mcurrent->m; msg; msg = msg->next) {
            if ((msg->mod_name) && (stricmp(msg->mod_name, mod_name) == 0)) {
                notice_lang(s_ChanServ, u, OPER_MODULE_MSG_LIST,
                            msg->name);
                display++;
            }
        }
    }
    return display;
}

#endif

/*************************************************************************/
