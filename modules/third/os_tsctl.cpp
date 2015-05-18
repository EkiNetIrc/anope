#include "module.h"

class CommandOsTsctl : public Command
{
 public:
	CommandOsTsctl(Module *creator, const Anope::string &sname = "operserv/tsctl") : Command(creator, sname, 0, 0)
	{
		this->SetDesc(_("Sync servers time"));
		this->SetSyntax("");
	}

	void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
	{
		UplinkSocket::Message() << "TSCTL SVSTIME " << time(NULL);

		source.Reply(_("Time synced."));
		Log(LOG_ADMIN, source, this) << "to sync the time of all servers of the network";
	}

	bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
	{
		this->SendSyntax(source);
		source.Reply(" ");
		source.Reply(_("Allows a Services Operator to sync the time of\n"
				"all servers of the network."));

		return true;
	}
};

class ModuleOsTsctl : public Module
{
	CommandOsTsctl commandostsctl;
 public:
	ModuleOsTsctl (const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD), commandostsctl(this)
	{
		if (!ModuleManager::FindModule("unreal")) 
		{ 
			Log() << "ERROR: You are not running UnrealIRCd, this module only works on UnrealIRCd.";
			return;
		}

		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}

	void OnNewServer(Server *s) anope_override
	{
		this->SyncTime();
	}

	void SyncTime()
	{
		UplinkSocket::Message() << "TSCTL SVSTIME " << time(NULL);
		Log(this) << "The time of all servers of the network is now synced !";
	}
};

MODULE_INIT(ModuleOsTsctl)
