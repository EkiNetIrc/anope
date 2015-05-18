/*--------------------------------------------------------------
* Name:    os_statsonid
* Author:  LEthaLity 'Lee' <lethality@anope.org>
* Date:    23rd February 2014
* Version: 2.0
* --------------------------------------------------------------
* This module displays some stats to users when they become a 
* Services Operator on /ns identify
* --------------------------------------------------------------
* This module does not add any commands and has no config options
* aside from loading it.
* --------------------------------------------------------------
* Configuration: operserv.conf
module { name = "os_statsonid" }
* --------------------------------------------------------------
*/

#include "module.h"

class OSStatsOnID : public Module
{
    ServiceReference<XLineManager> akills, snlines, sqlines;

public:
    OSStatsOnID(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, THIRD),
        akills("XLineManager", "xlinemanager/sgline"), snlines("XLineManager", "xlinemanager/snline"), sqlines("XLineManager", "xlinemanager/sqline")
    {
        this->SetAuthor("LEthaLity");
        this->SetVersion("2.0");
    }

    template<typename T> void GetHashStats(const T& map, size_t& entries)
	{
		entries = map.size();
	}

    void OnNickIdentify(User *u) anope_override
    {
        time_t uptime = Anope::CurTime - Anope::StartTime;
        size_t entries;
        int is_servoper = u->Account() && u->Account()->IsServicesOper();

        if (is_servoper)
        {
            u->SendMessage(Config->GetClient("OperServ"), "******** \002Services Stats Report\002 ********");
            u->SendMessage(Config->GetClient("OperServ"), "Uptime: %s", Anope::Duration(uptime, u->Account()).c_str());
            u->SendMessage(Config->GetClient("OperServ"), "Current users: \002%d\002 (\002%d\002 ops)", UserListByNick.size(), OperCount);
            u->SendMessage(Config->GetClient("OperServ"), "Maximum users: \002%d\002 (%s)", MaxUserCount, Anope::strftime(MaxUserTime, u->Account()).c_str());
            GetHashStats(*NickAliasList, entries);
            u->SendMessage(Config->GetClient("OperServ"), "Registered nicknames: \002%lu\002", entries);
            GetHashStats(*RegisteredChannelList, entries);
            u->SendMessage(Config->GetClient("OperServ"), "Registered channels: \002%lu\002", entries);
            if (akills)
                u->SendMessage(Config->GetClient("OperServ"), "Current akills: \002%d\002", akills->GetCount());
            if (snlines)
                u->SendMessage(Config->GetClient("OperServ"), "Current snlines: \002%d\002", snlines->GetCount());
            if (sqlines)
                u->SendMessage(Config->GetClient("OperServ"), "Current sqlines: \002%d\002", sqlines->GetCount());
            u->SendMessage(Config->GetClient("OperServ"), "************* \002Report End\002 *************");
        }
    }
};

MODULE_INIT(OSStatsOnID)
