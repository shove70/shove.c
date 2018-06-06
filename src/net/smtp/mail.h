#pragma once

#include <string>
#include <vector>
#include "compat.h"
#include <string.h>

using namespace std;

namespace shove
{
namespace net
{
namespace smtp
{

class Mail
{
public:

    Mail(const char* TOaddress, const char* FROMaddress, const char* Subject,
            const vector<char>& Message,
            const char* server = "127.0.0.1",
            unsigned short Port = SMTP_PORT,
            bool MXLookup = true);

    Mail(const char* TOaddress, const char* FROMaddress, const char* Subject,
            const char* Message,
            const char* server = "127.0.0.1",
            unsigned short Port = SMTP_PORT,
            bool MXLookup = true);

    Mail(bool MXLookup = false, unsigned short Port = SMTP_PORT);

    ~Mail();

    void operator()();
    void send();

    bool attach(const string& filename);
    bool removeattachment(const string& filename);

    bool setmessage(const string& newmessage);
    bool setmessage(const vector<char>& newmessage);

    bool setmessageHTML(const string& newmessage);
    bool setmessageHTML(const vector<char>& newmessage);

    bool setmessageHTMLfile(const string& filename);

    bool setsubject(const string& newSubject);
    bool setserver(const string& nameserver_or_smtpserver);
    bool setsender(const string& newsender);

    bool addrecipient(const string& newrecipient, short recipient_type = TO);
    bool removerecipient(const string& recipient);
    void clearrecipients();

    void clearattachments();
    void reset();

    const string& response() const;

    const static enum
    {
        TO, Cc, Bcc, SMTP_PORT = 25, DNS_PORT = 53
    } consts;

    enum authtype
    {
        LOGIN = 1, PLAIN
    } type;

    void authtype(const enum authtype Type);
    void username(const string& User);
    void password(const string& Pass);

private:

    vector<char> makesmtpmessage() const;
    void checkRFCcompat();
    string getserveraddress(const string& toaddress) const;
    bool gethostaddresses(vector<SOCKADDR_IN>& adds);
    bool parseRR(int& pos, const unsigned char dns[], string& name, in_addr& address);
    void parsename(int& pos, const unsigned char dns[], string& name);

    struct Address
    {
        string name;
        string address;
    };

    bool authenticate(const string& servergreeting, const SOCKET& s);

    typedef vector<pair<vector<char>, string> >::const_iterator vec_pair_char_str_const_iter;
    typedef vector<pair<Address, short> >::const_iterator recipient_const_iter;
    typedef vector<pair<Address, short> >::iterator recipient_iter;
    typedef vector<string>::const_iterator vec_str_const_iter;

    Address parseaddress(const string& addresstoparse);

    vector<pair<Address, short> > recipients;
    Address fromAddress;
    string subject;
    vector<char> message;
    vector<char> messageHTML;
    vector<pair<vector<char>, string> > attachments;
    string server;
    string nameserver;
    const unsigned short port;
    const bool lookupMXRecord;
    bool auth;
    string user;
    string pass;
    string returnstring;
};

}
}
}
