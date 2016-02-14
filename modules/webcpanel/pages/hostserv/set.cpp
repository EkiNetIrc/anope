/*
 * (C) 2003-2016 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 */

#include "../../webcpanel.h"

WebCPanel::HostServ::Set::Set(const Anope::string &cat, const Anope::string &u) : WebPanelProtectedPage (cat, u) 
{
}

bool WebCPanel::HostServ::Set::OnRequest(HTTPProvider *server, const Anope::string &page_name, HTTPClient *client, HTTPMessage &message, HTTPReply &reply, NickAlias *na, TemplateFileServer::Replacements &replacements) 
{
	if (!na->nc->o || !na->nc->o->ot->HasCommand("hostserv/setall") || !ServiceReference<Command>("Command", "hostserv/setall"))
	{
		replacements["NOACCESS"];
	}
	else
	{
		if (message.post_data.count("nickname") > 0)
		{
			std::vector<Anope::string> params;
			params.push_back(HTTPUtils::URLDecode(message.post_data["nickname"]));

			if (message.post_data.count("vhost") > 0)
			{
				params.push_back(HTTPUtils::URLDecode(message.post_data["vhost"]));
				WebPanel::RunCommand(na->nc->display, na->nc, "HostServ", "hostserv/setall", params, replacements);
			}
			else
			{
				WebPanel::RunCommand(na->nc->display, na->nc, "HostServ", "hostserv/delall", params, replacements);
			}
		}
	}
	TemplateFileServer page("hostserv/set.html");
	page.Serve(server, page_name, client, message, reply, replacements);
	return true;
}
