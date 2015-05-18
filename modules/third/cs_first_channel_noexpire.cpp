#include "module.h"

class ModuleCsFirstChannelNoExpire : public Module
{
 public:
	ModuleCsFirstChannelNoExpire (const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}

	void OnPreChanExpire(ChannelInfo *ci, bool &expire) anope_override
	{
		if (ci->name[1] != '#')
		{
			expire = false;
		}
	}

	EventReturn OnPreCommand(CommandSource &source, Command *command, std::vector<Anope::string> &params) anope_override
	{
		if ((source.command != "REGISTER" || params.size() < 1) || source.service != Config->GetClient("ChanServ"))
		{
			return EVENT_CONTINUE;
		}

		if (params[0][1] != '#')
		{
			Anope::string::size_type length_namespace = params[0].find('-');
			if (length_namespace != Anope::string::npos)
			{
				Anope::string chnamespace = params[0].substr(0, length_namespace);

				Channel *c = Channel::Find(chnamespace);
				if (c == NULL || c->ci->GetFounder()->display != source.GetAccount()->display)
				{
					Log(LOG_COMMAND, source, command) << "to try to register the channel " << params[0] << " (blocked due to namespacing policy)";
					source.Reply("Unable to register this channel due to namespacing policy");
					return EVENT_STOP;
				}
			}
			else
			{
				Log(LOG_COMMAND, source, command) << "to register the namespace " << params[0];
				source.Reply("You have registered the namespace " + params[0].substr(1) + ".");
				source.Reply(" ");
				source.Reply("Note that if you don't own a project for this");
				source.Reply("namespace, please drop this channel and register");
				source.Reply("a non-primary channel (starting with \"##\").");
				source.Reply(" ");
				source.Reply("Primary channels can be suspended & dropped");
				source.Reply("if it's created by a non-project owner.");
				source.Reply(" ");
			}
		}

		return EVENT_CONTINUE;
	}
};

MODULE_INIT(ModuleCsFirstChannelNoExpire)
