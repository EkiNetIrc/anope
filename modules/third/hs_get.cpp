// Modify the following as necessary and put it in your modules.conf file:

//	/*
//	 * hs_get
//	 *
//	 * Allow users to get a generic/predefined vhost.
//	 */
//	module
//	{
//		name = "hs_get"
//
//		/*
//		 * The vhost that can be get by users.
//		 * %a will be replaced by the account name
//		 * %r will be replaced by the "random" value set below if some caracters have been striped from the account name or by nothing.
//		 */
//		vhost = "%a%r.user"
//
//		/*
//		 * The random value that replace the %r in "vhost" paramter if caracters have been striped from the account name.
//		 * %n will be replaced by a pseudo-random number (really, it's the hex-encoded account registration time).
//		 */
//		random = "-%n"
//	}
//	command { service = "HostServ"; name = "GET"; command = "hostserv/get"; }
//	command { service = "HostServ"; name = "GIVE"; command = "hostserv/give"; permission = "hostserv/set"; }

///// *****END OF CONFIG***** /////

#include "module.h"

static Anope::string getVhost(Module *module, CommandSource &source, NickCore *nc, bool ignore_error = false);

class CommandHSGet : public Command
{
 public:
	CommandHSGet(Module *creator, const Anope::string &sname = "hostserv/get") : Command(creator, sname, 0, 0)
	{
		this->SetDesc(_("Get a predefined vhost"));
		this->RequireUser(true);
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		if (Anope::ReadOnly)
		{
			source.Reply(READ_ONLY_MODE);
			return;
		}

		NickAlias *na = NickAlias::Find(source.GetAccount()->display);

		if (source.GetAccount()->HasExt("UNCONFIRMED"))
		{
			source.Reply(_("You must confirm your account before you may request a vhost."));
			return;
		}

		if (na->HasVhost() || source.GetAccount()->HasExt("NOVHOST"))
		{
			source.Reply(_("Please contact the network staff to get a vhost."));
			return;
		}

		Anope::string host = getVhost(this->module, source, source.GetAccount());

		if (host.empty() || host.length() > Config->GetBlock("networkinfo")->Get<unsigned>("hostlen"))
		{
			source.Reply(HOST_SET_TOOLONG, Config->GetBlock("networkinfo")->Get<unsigned>("hostlen"));
			return;
		}

		Log(LOG_COMMAND, source, this) << "to set his vhost to " << host;
		na->SetVhost("", host, source.GetNick());
		FOREACH_MOD(OnSetVhost, (na));
		source.Reply(_("Your vHost is now set to \002%s\002."), host.c_str());
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Set your vhost to \002%s\002."), getVhost(this->module, source, source.GetAccount(), true).replace_all_cs("%r", "").c_str());

		return true;
	}
};

class CommandHSGive : public Command
{
 public:
	CommandHSGive(Module *creator, const Anope::string &sname = "hostserv/give") : Command(creator, sname, 1, 2)
	{
		this->SetDesc(_("Give the predefined vhost to a user"));
		this->SetSyntax(_("\037nick\037 [FORCE]"));
		this->RequireUser(true);
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		if (Anope::ReadOnly)
		{
			source.Reply(READ_ONLY_MODE);
			return;
		}

		const Anope::string &nick = params[0];

		NickAlias *na = NickAlias::Find(nick);
		if (na == NULL)
		{
			source.Reply(NICK_X_NOT_REGISTERED, nick.c_str());
			return;
		}

		if (na->nc->HasExt("NOVHOST") && (params.size() < 2 || params[1] != "FORCE"))
		{
			source.Reply(_("\002%s\002 isn't allowed to have a vhost."), na->nc->display.c_str());
			return;
		}

		Anope::string host = getVhost(this->module, source, na->nc);

		if (host.empty() || host.length() > Config->GetBlock("networkinfo")->Get<unsigned>("hostlen"))
		{
			source.Reply(HOST_SET_TOOLONG, Config->GetBlock("networkinfo")->Get<unsigned>("hostlen"));
			return;
		}

		Log(LOG_ADMIN, source, this) << "to set the vhost of " << na->nc->display << " to " << host;
		na->SetVhost("", host, source.GetNick());
		FOREACH_MOD(OnSetVhost, (na));
		source.Reply(_("VHost for \002%s\002 set to \002%s\002."), na->nc->display.c_str(), host.c_str());
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Give the predefined vhost to a user."));

		return true;
	}
};

class HSGet : public Module
{
	CommandHSGet commandhsget;
	CommandHSGive commandhsgive;
public:
	HSGet(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
		commandhsget(this), commandhsgive(this)
	{
		if (!IRCD || !IRCD->CanSetVHost)
			throw ModuleException("Your IRCd does not support vhosts");

		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}
};

static Anope::string getVhost(Module *module, CommandSource &source, NickCore *nc, bool ignore_error)
{
	Anope::string host = Config->GetModule(module)->Get<Anope::string>("vhost", "%a.user");
	host = host.replace_all_cs("%a", nc ? nc->display.lower() : "$account");

	if (nc && !IRCD->IsHostValid(host.replace_all_cs("%r", "")))
	{
		// We try to remove invalid characters
		const Anope::string vhostchars = Config->GetBlock("networkinfo")->Get<const Anope::string>("vhost_chars");

		for (unsigned k = 0; k < host.length(); ++k)
		{
			if (vhostchars.find_first_of(host[k]) == Anope::string::npos && host[k] != '%')
			{
				if (host[k] == '_' && vhostchars.find_first_of('-') != Anope::string::npos)
				{
					host[k] = '-';
					continue;
				}

				host.erase(k, 1);
				--k;
			}
		}

		// Now, we recheck our vhost
		if (!IRCD->IsHostValid(host.replace_all_cs("%r", "")) && !ignore_error)
		{
			// Sorry, the vhost is still invalid...
			source.Reply(HOST_SET_ERROR);
			return "";
		}

		NickAlias *na = NickAlias::Find(nc->display);
		Anope::string random = Config->GetModule(module)->Get<Anope::string>("random", "%n");
		std::stringstream hex_tr;
		hex_tr << std::hex << na->time_registered;
		host = host.replace_all_cs("%r", random.replace_all_cs("%n", hex_tr.str()));
	}

	host = host.replace_all_cs("%r", "");

	return host;
}

MODULE_INIT(HSGet)
