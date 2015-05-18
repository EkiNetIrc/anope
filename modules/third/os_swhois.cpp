/*
 * OS_swhois
 *
 *
 * Many thanks to the anope team.  This code is based heavily on the os_info
 * module's code.
 *
 * Update the SWhois entry if your ircd supoprts it.
 * Syntax:
 *  /os swhois <nick> ADD <swhois entry here>
 *  /os swhois <nick> CLEAR
 *  /os swhois <nick> LIST
 *
 * Configuration:
 *   useaccounti:   if set to yes (default), will cause the SWHOIS message to be associeated with the user's 
 *                  account rather than just the nick.
 *   notifyonadd:   if set to yes will notify the user that their swhois is activated when the oper adds it.
 *   notifyonlogin: if set to yes will notify the user that their swhois is active when they identify/login.
 *
 *
 * Add the following to your services config file (operserv.conf normally)

module { 
  name = "os_swhois";
  useaccount = yes;
  notifyonadd = no;
  notifyonlogin = no;
}
command { service ="OperServ"; name="SWHOIS"; command="operserv/swhois"; permission = "operserv/swhois"; }

 */

#include "module.h"


struct SWhoisMsg : Serializable
{
   Anope::string nick;
   Anope::string swhois;
   Anope::string creator;
   time_t when;

   SWhoisMsg() : Serializable("SWhoisMsg"), when(0) { }
   SWhoisMsg(const Anope::string &un, const Anope::string &sw, const Anope::string &c, time_t w) : 
     Serializable("SWhoisMsg"), nick(un), swhois(sw), creator(c), when(w) { }

   ~SWhoisMsg();

   void Serialize(Serialize::Data &data) const anope_override
   {
      data["nick"] << nick;
      data["swhois"] << swhois;
      data["creator"] << creator;
      data["when"] << when;
   }

   static Serializable *Unserialize(Serializable *obj, Serialize::Data &data);
};

struct SWhoisMsgs : Serialize::Checker<std::vector<SWhoisMsg *> >
{
   SWhoisMsgs(Extensible *) : Serialize::Checker<std::vector<SWhoisMsg *> >("SWhoisMsg") { }

   ~SWhoisMsgs()
   {
      for (unsigned i = (*this)->size(); i > 0; --i)
         delete (*this)->at(i - 1);
   }

   static Extensible *Find(const Anope::string &nick)
   {
      NickAlias *na = NickAlias::Find(nick);
      if (na)
         return na->nc;
      return NULL;
   }
};

SWhoisMsg::~SWhoisMsg()
{
   Extensible *e = SWhoisMsgs::Find(nick);
   if (e)
   {
      SWhoisMsgs *op = e->GetExt<SWhoisMsgs>("swhoismsg");
      if (op)
      {
         std::vector<SWhoisMsg *>::iterator it = std::find((*op)->begin(), (*op)->end(), this);
         if (it != (*op)->end())
            (*op)->erase(it);
      }
   }
}

Serializable *SWhoisMsg::Unserialize(Serializable *obj, Serialize::Data &data)
{
   Anope::string snick;
   data["nick"] >> snick;

   Extensible *e = SWhoisMsgs::Find(snick);
   if (!e)
      return NULL;

   SWhoisMsgs *oi = e->Require<SWhoisMsgs>("swhoismsg");
   SWhoisMsg *o;
   if (obj)
      o = anope_dynamic_static_cast<SWhoisMsg *>(obj);
   else
   {
      o = new SWhoisMsg();
      o->nick = snick;
   }
   data["swhois"] >> o->swhois;
   data["creator"] >> o->creator;
   data["when"] >> o->when;

   if (!obj)
      (*oi)->push_back(o);
   return o;
}

class CommandSWhois : public Command
{

 public:
   CommandSWhois(Module *creator) : Command(creator, "operserv/swhois",2,3)
   {
      this->SetDesc(_("Modify the Swhois value for a nick"));
      this->SetSyntax(_("\037nick\037 ADD \037swhois message\037"));
      this->SetSyntax(_("\037nick\037 CLEAR"));
      this->SetSyntax(_("\037nick\037 LIST"));
   }
   void Execute(CommandSource &source, const std::vector<Anope::string> &params) anope_override
   {
     const Anope::string &unick = params[0];
     const Anope::string &subcmd = params[1];
     const Anope::string &message = params.size() > 2 ? params[2] : "";
     bool notifyonadd = Config->GetModule(this->owner)->Get<bool>("notifyonadd","no");
     bool notifyonlogin = Config->GetModule(this->owner)->Get<bool>("notifyonlogin","no");
     bool useaccount = Config->GetModule(this->owner)->Get<bool>("useaccount","yes");
	  BotInfo *bi = BotInfo::Find(source.service->nick);
     Anope::string displaynick;

     Extensible *e;
     NickAlias *na = NickAlias::Find(unick);
     if (Anope::ReadOnly) 
     {
        source.Reply(READ_ONLY_MODE);
        return;
     }
     if (!na)
     {
        source.Reply(NICK_X_NOT_REGISTERED, unick.c_str());
        return;
     }
     e = na->nc;
     NickCore *nc = na->nc;
     if (useaccount)
     {
        displaynick = nc->display;
     }
     else 
     {
        displaynick = na->nick;
     }

     if (subcmd.equals_ci("add"))
     {
        if (message.empty())
        {
           this->OnSyntaxError(source, subcmd);
           return;
        }
        SWhoisMsgs *oi = e->Require<SWhoisMsgs>("swhoismsg");
        if (oi)
        {
           if ((*oi)->size() > 0)
           {
              e->Shrink<SWhoisMsgs>("swhoismsg"); /* clear old entry */
              oi = e->Require<SWhoisMsgs>("swhoismsg");
           }
        }
        (*oi)->push_back(new SWhoisMsg(displaynick, message, source.GetNick(), Anope::CurTime));
        if (useaccount)
        {
           source.Reply(_("Added Swhois to \002%s\002 (\002%s\002)."), unick.c_str(),displaynick.c_str());
           Log(LOG_ADMIN, source, this) << "to add SWHOIS info to " << unick << "(" << displaynick << ")";
        }
        else
        {
           source.Reply(_("Added Swhois to \002%s\002."), unick.c_str());
           Log(LOG_ADMIN, source, this) << "to add SWHOIS info to " << unick ;
        }
        if (User *u = User::Find(unick))
        {
           IRCD->SendSWhois(Config->GetClient("OperServ"),unick,message);
           if (notifyonadd || notifyonlogin)
           {
              u->SendMessage(bi, _("Your swhois of \002%s\002 is now activated."),message.c_str()); 
           }
        }
     }
     else if (subcmd.equals_ci("clear"))
     {
        SWhoisMsgs *oi = e->Require<SWhoisMsgs>("swhoismsg");
        if (!oi)
        {
           return;
        }
        e->Shrink<SWhoisMsgs>("swhoismsg");
        if (useaccount)
        {
           source.Reply(_("Cleared SWhois info for \002%s\002 (\002%s\002)."), unick.c_str(),displaynick.c_str());
           Log(LOG_ADMIN, source, this) << "to clear SWHOIS info on " << unick << "(" << displaynick << ")";
        }
        else
        {
           source.Reply(_("Cleared SWhois info for \002%s\002."), unick.c_str());
           Log(LOG_ADMIN, source, this) << "to clear SWHOIS info on " << unick ;
        }
        if (User::Find(unick))
        {
           IRCD->SendSWhois(Config->GetClient("OperServ"),unick,"");
        }
     }
     else if (subcmd.equals_ci("list"))
     {
        SWhoisMsgs *oi = e->Require<SWhoisMsgs>("swhoismsg"); 
        if (!oi)
           return;
        for (unsigned i = 0; i < (*oi)->size(); ++i)
        {
           SWhoisMsg *o = (*oi)->at(i);
           source.Reply(_("Swhois of \002%s\002 is \002%s\002."), unick.c_str(), o->swhois.c_str());
        }
     }
     else
     {
        this->SendSyntax(source);
        return;
     }

   }
   bool OnHelp(CommandSource &source, const Anope::string &subcommand) anope_override
   {
      Anope::string target;
      bool useaccount = Config->GetModule(this->owner)->Get<bool>("useaccount","yes");
      if (useaccount)
         target = "account";
      else
         target = "nick";

      this->SendSyntax(source);
      source.Reply(" ");
      source.Reply(_("Allows you to assign or remove the supplimental \n"
                     "whois (called swhois) for a nick."));
      source.Reply(" ");
      source.Reply(_("\002ADD\002 adds the new swhois message to the user's %s."),target.c_str());
      source.Reply(" ");
      source.Reply(_("\002CLEAR\002 remove the swhois message from the user's %s."),target.c_str());
      source.Reply(" ");
      source.Reply(_("\002LIST\002 shows the swhois message on the user's %s."),target.c_str());
      source.Reply(" ");
      return true;
   }

};

class OSSWhois : public Module
{
   CommandSWhois commandswhois;
   ExtensibleItem<SWhoisMsgs> *eml;
   Reference<BotInfo> OperServ;
   Serialize::Type *swhoismsg_type;

   void OnSwhois(User *u)
   {
      const bool &notifyonlogin = Config->GetModule(this)->Get<bool>("notifyonlogin");
      Extensible *e = SWhoisMsgs::Find(u->nick);
      if(!e)
      {
         return;
      }
      SWhoisMsgs *oi = eml->Get(e); 
      if (!oi)
         return;
      for (unsigned i = 0; i < (*oi)->size(); ++i)
      {
         SWhoisMsg *o = (*oi)->at(i);
         IRCD->SendSWhois(Config->GetClient("OperServ"),o->nick,o->swhois);
         if (notifyonlogin)
         {
            u->SendMessage(OperServ, _("Your swhois of \002%s\002 is now activated."), o->swhois.c_str());
         }
      }
   }

 public:
   OSSWhois(const Anope::string &modname, const Anope::string &creator) : Module(modname, creator, EXTRA),
   commandswhois(this)
   {
      this->SetAuthor("Azander");
      this->SetVersion("1.0.3");
      eml = new ExtensibleItem<SWhoisMsgs>(this, "swhoismsg");
      swhoismsg_type = new Serialize::Type("SWhoisMsg", SWhoisMsg::Unserialize);
   }

   ~OSSWhois()
   {
      delete eml;
      delete swhoismsg_type;
   }

   void OnUserLogin(User *u) anope_override
   {
      OnSwhois(u);
   }

};

MODULE_INIT(OSSWhois)
