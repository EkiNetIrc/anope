/*
 * (C) 2003-2013 Anope Team
 * Contact us at team@anope.org
 *
 * Please read COPYING and README for further details.
 */

#include "../../webcpanel.h"

namespace WebCPanel
{

namespace ChanServ
{

void BuildChanList(NickAlias *na, TemplateFileServer::Replacements &replacements)
{
	std::deque<ChannelInfo *> queue;
	na->nc->GetChannelReferences(queue);
	for (unsigned i = 0; i < queue.size(); ++i)
	{
		ChannelInfo *ci = queue[i];

		if (na->nc != ci->GetFounder() && ci->AccessFor(na->nc).empty())
			continue;

		replacements["CHANNEL_NAMES"] = ci->name;
		replacements["ESCAPED_CHANNEL_NAMES"] = HTTPUtils::URLEncode(ci->name);
	}
}

}

}

