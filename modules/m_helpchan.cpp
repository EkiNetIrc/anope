/*
 *
 * (C) 2003-2016 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 */

#include "module.h"

class HelpChannel : public Module
{
 public:
	HelpChannel(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, VENDOR)
	{
	}

	EventReturn OnChannelModeSet(Channel *c, MessageSource &, ChannelMode *mode, const Anope::string &param) anope_override
	{
		if (mode->name == "VOICE" && c && c->ci && c->name.equals_ci(Config->GetModule(this)->Get<const Anope::string>("helpchannel")))
		{
			User *u = User::Find(param);

			if (u && c->ci->AccessFor(u).HasPriv("VOICEME"))
				u->SetMode(Config->GetClient("OperServ"), "HELPOP");
		}

		return EVENT_CONTINUE;
	}
};

MODULE_INIT(HelpChannel)
