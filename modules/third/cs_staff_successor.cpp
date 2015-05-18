#include "module.h"

class ModuleCsStaffSuccessor : public Module
{
 public:
	ModuleCsStaffSuccessor (const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}

	void SetSuccessor(ChannelInfo *ci)
	{
		NickCore *successor = NickCore::Find(Config->GetModule("cs_staff_successor")->Get<const Anope::string>("Successor", "OperServ"));

		if (successor != NULL && ci->GetSuccessor() != successor && ci->GetFounder() != successor)
		{
			ci->SetSuccessor(successor);
		}
	}

	/*void OnChanRegistered(ChannelInfo *ci) anope_override
	{
		this->SetSuccessor(ci);
	}*/

	void OnDelNick(NickAlias *na) anope_override
	{
		std::deque<ChannelInfo *> chans;
		na->nc->GetChannelReferences(chans);

		for (unsigned i = 0; i < chans.size(); ++i)
		{
			ChannelInfo *ci = chans[i];

			if (ci->GetFounder() == na->nc && ci->GetSuccessor() == NULL)
			{
				this->SetSuccessor(ci);
			}
		}
	}

	/*EventReturn OnSetChannelOption(CommandSource &source, Command *cmd, ChannelInfo *ci, const Anope::string &setting) anope_override
	{
		if (cmd == CommandCSSetPersist && !setting.equals_ci("ON") && ci->GetSuccessor() == NULL)
		{
			this->SetSuccessor(ci);
		}

		if (cmd == CommandCSSetSuccessor && setting == "")
		{
			this->SetSuccessor(ci);
		}

		return EVENT_ALLOW;
	}*/
};

MODULE_INIT(ModuleCsStaffSuccessor)
