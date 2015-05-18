/* OperServ KillClones functions
 *
 * (C) 2014 Azander
 *
 * Please read COPYING and README for further details.
 *
 * Based on the original code of Anope.
 * Based on the original code of Epona by Lara.
 * Based on the original code of Services by Andy Church.
 *
 * Much of this comes from the akill routines in operserv along with 
 * additional code from other commands.  
 *
 * This command kills all clones of a specified nick, and sets an akill
 * their host.  If it encounters an ircop while killing the clones it 
 * will not add the akill, or kill that one clinet connection.  
 * 
 * Command is: /msg OperServ CLONEKILL nick
 *
 * Add the following configuration to your operserv confg file:
 *
 * module { name = "os_killclones"; akillexpire = "5m"; }
 * command { service = "OperServ"; name = "CLONEKILL"; command = "operserv/killclones"; permission = "operserv/killclones"; }
 *
 *
 */

#include "module.h"
#include "modules/os_session.h"

static ServiceReference<XLineManager> akills("XLineManager", "xlinemanager/sgline");

class CommandOSKillClones : public Command
{

 public:
	CommandOSKillClones(Module *creator) : Command(creator, "operserv/killclones", 1, 2)
	{
		this->SetDesc(_("Kill a user and their clones"));
		this->SetSyntax(_("\037user\037 [\037reason\037]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		const Anope::string &nick = params[0];
		Anope::string reason = params.size() > 1 ? params[1] : "";
		time_t expires = Config->GetModule("os_killclones")->Get<time_t>("akillexpire", "5m");
		expires += Anope::CurTime;
		bool isoper = false;
		Anope::string opernick = "";

		User *u2 = User::Find(nick, true);
		if (u2 == NULL)
			source.Reply(NICK_X_NOT_IN_USE, nick.c_str());
		else if (u2->IsProtected() || u2->server == Me)
			source.Reply(ACCESS_DENIED);
		else
		{
			if (reason.empty())
				reason = "Clone attack protection.";
			if (Config->GetModule("operserv")->Get<bool>("addakiller"))
				reason = "(" + source.GetNick() + ") " + reason;
			Log(LOG_ADMIN, source, this) << "on " << u2->nick << " hostmask *@" << u2->host << " for " << reason;
/*  THIS IS THE ITERATION CODE!  Check it out */
			Anope::string rreason = "Clonekilled: " + reason;
			XLine *x = new XLine("*@"+u2->host, source.GetNick(), expires, rreason);
			if (Config->GetModule("operserv")->Get<bool>("akillids"))
				x->id = XLineManager::GenerateUID();

			for (user_map::const_iterator it = UserListByNick.begin(); it != UserListByNick.end(); ++it)
			{
				User *user = it->second;
				if (!user->HasMode("OPER") && user->server != Me && user->host == u2->host)
				{
					Log(LOG_ADMIN, source, this) << "Killing " << user->nick.c_str() ;
					user->Kill(Me->GetName(), rreason);
				}
				if (user->HasMode("OPER") && user->host == u2->host)
				{
					isoper = true;
					opernick.append(user->nick.c_str());
				}
			}
			if (!isoper)
			{
				Log(LOG_ADMIN, source, this) << "Akill added. Mask:" << x->mask.c_str() << " ID:" << x->id;
				akills->AddXLine(x);
			}
			else
			{
				Log(LOG_ADMIN, source, this) << "Akill not added for "<< x->mask.c_str() << " User is ircop "<< opernick << " ,using clones.";
				source.Reply(_("Cannot add this kline: %s.  User is an ircop. (%s)"),x->mask.c_str(),opernick.c_str());
			}
		}
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Allows you to kill a user from the network.\n"
				"It also seeks out and kills all their clones.\n"
				"Parameters are the same as for the standard /KILL\n"
				"command.\n"
				" \n"
				"If the clone detector sees an ircop in the list of\n"
				"matching clones it will nto kill that connection and\n"
				"it will nto add the akill blocking the return of\n"
			       	"the others\n"
				"\n"));
		return true;
	}
};

class OSKillClones : public Module
{
	CommandOSKillClones commandoskillclones;

 public:
	OSKillClones(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, VENDOR),
		commandoskillclones(this)
	{
		this->SetAuthor("Azander");
		this->SetVersion("1.0");
	}
};

MODULE_INIT(OSKillClones)
