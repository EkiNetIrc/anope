/* ChanServ core functions
 *
 * (C) 2003-2010 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * $Id$
 *
 */
/*************************************************************************/

#include "module.h"

class CommandCSSetPersist : public Command
{
 public:
	CommandCSSetPersist(const ci::string &cname, const std::string &cpermission = "") : Command(cname, 2, 2, cpermission)
	{
	}

	CommandReturn Execute(User *u, const std::vector<ci::string> &params)
	{
		ChannelInfo *ci = cs_findchan(params[0]);
		assert(ci);

		ChannelMode *cm = ModeManager::FindChannelModeByName(CMODE_PERM);

		if (params[0] == "ON")
		{
			if (!ci->HasFlag(CI_PERSIST))
			{
				ci->SetFlag(CI_PERSIST);

				/* Channel doesn't exist, create it internally */
				if (!ci->c)
				{
					new Channel(ci->name);
					if (ci->bi)
						bot_join(ci);
				}

				/* No botserv bot, no channel mode */
				if (!ci->bi && !cm)
				{
					/* Give them ChanServ
					 * Yes, this works fine with no Config.s_BotServ
					 */
				      ChanServ->Assign(NULL, ci);
				}

				/* Set the perm mode */
				if (cm && ci->c && !ci->c->HasMode(CMODE_PERM))
				{
					ci->c->SetMode(NULL, cm);
				}
			}

			notice_lang(Config.s_ChanServ, u, CHAN_SET_PERSIST_ON, ci->name.c_str());
		}
		else if (params[0] == "OFF")
		{
			if (ci->HasFlag(CI_PERSIST))
			{
				ci->UnsetFlag(CI_PERSIST);

				/* Unset perm mode */
				if (cm && ci->c && ci->c->HasMode(CMODE_PERM))
					ci->c->RemoveMode(NULL, cm);
				if (Config.s_BotServ && ci->bi && ci->c->users.size() == Config.BSMinUsers - 1)
					ircdproto->SendPart(ci->bi, ci->c, NULL);

				/* No channel mode, no BotServ, but using ChanServ as the botserv bot
				 * which was assigned when persist was set on
				 */
				if (!cm && !Config.s_BotServ && ci->bi)
				{
					/* Unassign bot */
					ChanServ->UnAssign(NULL, ci);   
				}

				if (ci->c && ci->c->users.empty())
					delete ci->c;
			}

			notice_lang(Config.s_ChanServ, u, CHAN_SET_PERSIST_OFF, ci->name.c_str());
		}
		else
			this->OnSyntaxError(u, "PERSIST");

		return MOD_CONT;
	}

	bool OnHelp(User *u, const ci::string &)
	{
		notice_help(Config.s_ChanServ, u, CHAN_HELP_SET_PERSIST, "SET");
		return true;
	}

	void OnSyntaxError(User *u, const ci::string &)
	{
		syntax_error(Config.s_ChanServ, u, "SET PERSIST", CHAN_SET_PERSIST_SYNTAX);
	}

	void OnServHelp(User *u)
	{
		notice_lang(Config.s_ChanServ, u, CHAN_HELP_CMD_SET_PERSIST);
	}
};

class CommandCSSASetPersist : public CommandCSSetPersist
{
 public:
	CommandCSSASetPersist(const ci::string &cname) : CommandCSSetPersist(cname, "chanserv/saset/persist")
	{
	}

	bool OnHelp(User *u, const ci::string &)
	{
		notice_help(Config.s_ChanServ, u, CHAN_HELP_SET_PERSIST, "SASET");
		return true;
	}

	void OnSyntaxError(User *u, const ci::string &)
	{
		syntax_error(Config.s_ChanServ, u, "SASET PERSIST", CHAN_SASET_PERSIST_SYNTAX);
	}
};

class CSSetPersist : public Module
{
 public:
	CSSetPersist(const std::string &modname, const std::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Anope");
		this->SetVersion("$Id$");
		this->SetType(CORE);

		Command *c = FindCommand(ChanServ, "SET");
		if (c)
			c->AddSubcommand(new CommandCSSetPersist("PERSIST"));
		
		c = FindCommand(ChanServ, "SASET");
		if (c)
			c->AddSubcommand(new CommandCSSASetPersist("PERSIST"));
	}

	~CSSetPersist()
	{
		Command *c = FindCommand(ChanServ, "SET");
		if (c)
			c->DelSubcommand("PERSIST");

		c = FindCommand(ChanServ, "SASET");
		if (c)
			c->DelSubcommand("PERSIST");
	}
};

MODULE_INIT(CSSetPersist)