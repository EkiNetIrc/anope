#include "module.h"
#define VER "1.0"
/**
 * -----------------------------------------------------------------------------
 * Name    : os_superadmin
 * Author  : Cronus <Cronus@Nite-Serv.com>
 * Date    : 6/10/2014 (Last update: 6/10/2014)
 * Version : 1.0
 * -----------------------------------------------------------------------------
 * Requires    : Anope 2.0.0
 * Tested      : Anope 2.0.0
 * -----------------------------------------------------------------------------
 * This module adds the OperServ SUPERADMIN command that allows you to LIST
 * all SuperAdmin's and also has the capability to make any user a SuperAdmin 
 * or turn off any user's SuperAdmin.
 * -----------------------------------------------------------------------------
 *
 *   1.0  -  Initial release
 *
 * -----------------------------------------------------------------------------
 *
 * Credits:
 *
 * Adam, Attila, Azander for their assistance in creating this.
 *
 * -----------------------------------------------------------------------------
 * Config:
 *
 * module { name = "os_superadmin" }
 * command { service = "OperServ"; name="SUPERADMIN"; command = "operserv/superadmin"; permission = "operserv/superadmin"; } 
 *
 **/

class CommandOSSuperAdmin : public Command
{
public:
	CommandOSSuperAdmin(Module *creator) : Command(creator, "operserv/superadmin", 1, 2)
	{
		this->SetDesc(_("List all SA's and optionally grant or remove SA status"));
		this->SetSyntax(_("{LIST|ON|OFF} [\037nickname\037]"));
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params)
	{
		const Anope::string &subcommand = params.size() > 0 ? params[0] : "LIST";
		const Anope::string &nick = params.size() > 1 ? params[1] : "";
		
		User *u = User::Find(nick, true);
		
		if (subcommand.equals_ci("LIST"))
		{
			int count = 0, tcount = 0;
			
			source.Reply(_("SuperAdmin list:"));
			
			for (user_map::const_iterator it = UserListByNick.begin(); it != UserListByNick.end(); ++it)
			{
				User *user = it->second;
				++tcount;
				if (user->super_admin)
				{
					source.Reply(_("%s	%s@%s"), user->nick.c_str(), user->GetVIdent().c_str(), user->GetDisplayedHost().c_str());
					++count;
				}
			}
			source.Reply(_("End of list - %d/%d users found with SuperAdmin enabled."), count, tcount);
		}
		else if (subcommand.equals_ci("ON") && !params.empty() && u)
		{
			Log(LOG_OVERRIDE, source, this, NULL) << "to set SUPERADMIN on " << u->nick;
			source.Reply(_("You have enabled SUPERADMIN on user \002%s\002"), nick.c_str());
			u->super_admin = true;
		}
		else if (subcommand.equals_ci("OFF") && !params.empty() && u)
		{
			Log(LOG_OVERRIDE, source, this, NULL) << "to set SUPERADMIN off " << u->nick;
			source.Reply(_("You have disabled SUPERADMIN on user \002%s\002"), nick.c_str());
			u->super_admin = false;
		}
		else
			this->OnSyntaxError(source, subcommand);
	} 
	bool OnHelp(CommandSource &source, const Anope::string &subcommand)
	{ 
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("List all SA's and optionally grant or remove SA status"));
		return true;
	}
};

class OSSuperAdmin : public Module
{
	CommandOSSuperAdmin commandossuperadmin;
	
public:
	OSSuperAdmin(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator),	
		commandossuperadmin(this)
	{
		this->SetAuthor("Cronus");
		this->SetVersion(VER);
	}
};

MODULE_INIT(OSSuperAdmin) 
