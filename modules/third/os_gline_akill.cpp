#include "module.h"

static ServiceReference<XLineManager> akills("XLineManager", "xlinemanager/sgline");

class ModuleOsGlineAkill : public Module
{
 public:
	ModuleOsGlineAkill (const Anope::string &modname, const Anope::string &creator) : Module(modname, creator)
	{
		this->SetAuthor("Zoddo");
		this->SetVersion("1.0.0");
	}

	EventReturn OnMessage(MessageSource &source, Anope::string &command, std::vector<Anope::string> &param) anope_override
	{
		if (command != "TKL" || param[1] != "G")
		{
			return EVENT_CONTINUE;
		}

		if (param[0] == "+")
		// Server send: TKL + G * 127.0.0.1 Zoddo!~Zoddo@ekinetirc/staff/zoddo 1403529633 1403443233 reason
		{
			XLine *x;
			Anope::string id;
			if ((x = akills->HasEntry(param[2] + '@' + param[3]))) // We already know the TKL? Probably updating
			{
				if (x->expires == atoi(param[5].c_str()) && x->reason + " (ID: " + x->id + ")" == param[7]) // Hum...Identifcal?? Return from netsplit perhaps
				{
					return EVENT_CONTINUE;
				}
				else
				{
					id = x->id;
					akills->DelXLine(x);
				}
			}
			else
			{
				id = XLineManager::GenerateUID();
				Log(Config->GetClient("OperServ")) << "AKILL: Assigning ID " << id << " to the " << param[1]<<"LINE " << (param[2] + '@' + param[3]) << " just added by " << param[4];
			}

			x = new XLine(param[2] + '@' + param[3], param[4], atoi(param[5].c_str()), param[7], id);

			akills->AddXLine(x);
			akills->Send(NULL, x);
		}

		if (param[0] == "-")
		// Server send: TKL - G * 127.0.0.1 Zoddo!~Zoddo@ekinetirc/staff/zoddo
		{
			XLine *x;

			while ((x = akills->HasEntry(param[2] + '@' + param[3])))
			{
				akills->DelXLine(x);
			}
		}

		return EVENT_CONTINUE;
	}
};

MODULE_INIT(ModuleOsGlineAkill)
