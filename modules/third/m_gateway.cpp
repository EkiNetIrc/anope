// Modify the following as necessary and put it in your modules.conf file:

//	/*
//	 * m_gateway
//	 *
//	 * Allow to set a custom vHost to gateway that don't support CGI:IRC
//	 * to spoof the host.
//	 */
//	module
//	{
//		name = "m_gateway"
//
//		/*
//		 * If set, Anope will check clients when coming back from a netsplit (recommanded).
//		 */
//		check_on_netburst = yes
//
//		/*
//		 * This block can be duplicated as many times as necessary.
//		 */
//		gateway
//		{
//			name = "irccloud.com"
//
//			/*
//			 * Here, you can indicate ips will matches for this gateway.
//			 * The "host" block can be duplicate as many times as necessary.
//			 * Don't forget to add connection limits exceptions for these hosts if necessary.
//			 */
//			host { ip = "192.184.9.*"; }
//			host { ip = "192.184.10.*"; }
//			host { ip = "170.178.187.131"; }
//			host { ip = "2604:8300:100:200b::*"; }
//			host { ip = "2001:67c:2f08::*"; }
//
//			/*
//			 * The vhost that will be applied.
//			 * %a is the account name (SASL authentification required)
//			 * %n is the nick of the user (you should use %a instead if possible)
//			 * %u is the ident/username of the user
//			 * %i is the IP of the user
//			 *
//			 * Invalid characters will be striped from the final vhost. If the vhost remain
//			 * invalid or if the vhost length is too long, the vhost will ne be applied.
//			 */
//			vhost = "%u.irccloud.com"
//
//			/*
//			 * If set, the user will be autorised to connect through this gateway only when
//			 * identified via SASL. If your IRCd have a similar option, you should consider
//			 * to enable it instead of this option to limit the amount of kills sent by Anope.
//			 */
//			#need_account = true
//		}
//	}

///// *****END OF CONFIG***** /////

#include "module.h"

struct Gateway
{
	Anope::string name;
	std::vector<Anope::string> ips;
	Anope::string vhost;
	bool need_account;
};

class ModuleMGateway : public Module
{
	std::vector<Gateway> gateways;
	bool check_on_netburst;

 public:
	ModuleMGateway (const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}

	void OnReload(Configuration::Conf *conf) anope_override
	{
		Configuration::Block *block = conf->GetModule(this);
		this->check_on_netburst = block->Get<bool>("check_on_netburst");

		this->gateways.clear();
		for (int i = 0; i < block->CountBlock("gateway"); ++i)
		{
			Configuration::Block *gw = block->GetBlock("gateway", i);
			Gateway gateway;

			for (int j = 0; j < gw->CountBlock("host"); ++j)
			{
				Configuration::Block *host = gw->GetBlock("host", j);
				Anope::string ip = host->Get<Anope::string>("ip");

				gateway.ips.push_back(ip);
			}

			gateway.name = gw->Get<Anope::string>("name");
			gateway.vhost = gw->Get<Anope::string>("vhost");
			gateway.need_account = gw->Get<bool>("need_account");

			// If we use the account name placeholder (%a), we need to be identified
			if (gateway.vhost.find("%a") != Anope::string::npos)
				gateway.need_account = true;

			if (!gateway.ips.empty() && !gateway.vhost.empty())
				this->gateways.push_back(gateway);
		}
	}

	void OnUserConnect(User *u, bool &exempt) anope_override
	{
		if (!this->check_on_netburst && !u->server->IsSynced())
			return;

		const NickAlias *na = NickAlias::Find(u->nick);
		if (na && u->Account() == na->nc && na->HasVhost())
		{
			// If the user has a vhost, we do nothing (otherwise, we may overwrite the vhost)
			return;
		}

		Anope::string addr = u->ip.addr();

		for (unsigned i = 0; i < this->gateways.size(); ++i)
		{
			const Gateway &gw = this->gateways[i];

			for (unsigned j = 0; j < gw.ips.size(); ++j)
			{
				if (Anope::Match(addr, gw.ips[j]))
				{
					if (gw.need_account && !u->Account())
					{
						IRCD->SendNotice(Me->GetSID(), u->GetUID(), "*** ERROR: You need to identify via SASL to use this gateway.");
						IRCD->SendSVSKillInternal(Me, u, "SASL access only");
						return;
					}

					Anope::string vhost = gw.vhost;
					vhost = vhost.replace_all_cs("%a", u->Account() ? u->Account()->display : "");
					vhost = vhost.replace_all_cs("%n", u->nick.lower());
					vhost = vhost.replace_all_cs("%u", u->GetIdent());
					vhost = vhost.replace_all_cs("%i", addr);

					if (vhost.length() > Config->GetBlock("networkinfo")->Get<unsigned>("hostlen"))
					{
						Log() << "ERROR: [m_gateway] Vhost too long: " << vhost;
						return;
					}

					if (!IRCD->IsHostValid(vhost))
					{
						// We try to remove invalid characters
						const Anope::string vhostchars = Config->GetBlock("networkinfo")->Get<const Anope::string>("vhost_chars");

						for (unsigned k = 0; k < vhost.length(); ++k)
						{
							if (vhostchars.find_first_of(vhost[k]) == Anope::string::npos)
							{
								vhost.erase(k);
								--k;
							}
						}

						// Now, we recheck our vhost
						if (!IRCD->IsHostValid(vhost))
						{
							// Sorry, the vhost is still invalid...
							Log() << "ERROR: [m_gateway] Invalid vhost: " << vhost;
							return;
						}
					}

					BotInfo *HostServ = Config->GetClient("HostServ");
					u->SetMode(HostServ, "CLOAK");
					IRCD->SendVhost(u, "", vhost);
					u->SetCloakedHost(vhost);

					return;
				}
			}
		}
	}
};

MODULE_INIT(ModuleMGateway)
