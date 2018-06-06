#ifdef WIN32
#pragma warning( disable : 4786 )
#endif
#include <fstream>
#include <sstream>
#include <ctime>
#include <cassert>

#include "../../encode/base64.h"
#include "../../utils/utils.h"

#include "mail.h"

using namespace shove::encode;
using namespace shove::utils;

namespace shove
{
namespace net
{
namespace smtp
{

Mail::Mail(const char* TOaddress, const char* FROMaddress, const char* Subject,
        const vector<char>& Message, const char* Nameserver, unsigned short Port, bool MXLookup) :
        type(LOGIN), subject(Subject), server(getserveraddress(TOaddress)), nameserver(Nameserver), port(htons(Port)), lookupMXRecord(MXLookup), auth(false)
{
    setsender(FROMaddress);
    addrecipient(TOaddress);
    setmessage(Message);

    initNetworking();
}

Mail::Mail(const char* TOaddress, const char* FROMaddress, const char* Subject,
        const char* Message, const char* Nameserver, unsigned short Port, bool MXLookup) :
        type(LOGIN), subject(Subject), server(getserveraddress(TOaddress)), nameserver(Nameserver), port(htons(Port)),
        lookupMXRecord(MXLookup), auth(false)
{
    setsender(FROMaddress);
    addrecipient(TOaddress);
    setmessage(Message);

    initNetworking();
}

Mail::Mail(bool MXLookup, unsigned short Port) : type(LOGIN), port(htons(Port)), lookupMXRecord(MXLookup), auth(false)
{
    initNetworking();
}

Mail::~Mail()
{
}

bool Mail::setmessage(const string& newmessage)
{
    if (!newmessage.length())
        return false;

    message.clear();
    for (string::size_type i = 0; i < newmessage.length(); ++i)
        message.push_back(newmessage[i]);

    checkRFCcompat();

    return true;
}

bool Mail::setmessage(const vector<char>& newmessage)
{
    if (!newmessage.size())
        return false;

    message = newmessage;

    checkRFCcompat();

    return true;
}

bool Mail::setmessageHTML(const string& newmessage)
{
    if (!newmessage.length())
        return false;

    stringToVector<char>(Base64::encode((ubyte*)newmessage.c_str(), newmessage.size()), messageHTML);

    return true;
}

bool Mail::setmessageHTML(const vector<char>& newmessage)
{
    if (!newmessage.size())
        return false;

    string str = vectorToString<char>(newmessage);
    stringToVector<char>(Base64::encode((ubyte*)str.c_str(), str.size()), messageHTML);

    return true;
}

bool Mail::setmessageHTMLfile(const string& filename)
{
    if (!filename.length())
        return false;

    string str = readFile(filename);
    stringToVector<char>(Base64::encode((ubyte*)str.c_str(), str.size()), messageHTML);

    return true;
}

void Mail::checkRFCcompat()
{
    vector<char>::iterator it;

    for (it = message.begin(); it != message.end(); ++it)
    {
        if (*it == '\n')
        {
            if (it == message.begin())
            {
                it = message.insert(it, '\r');
                ++it;

                continue;
            }
            if ((*(it - 1) != '\r'))
            {
                it = message.insert(it, '\r');
                ++it;
            }
        }
    }

    if (message.size() == 1)
    {
        if (*(message.begin()) == '.')
            message.push_back('.');
    }
    else if (message.size() == 2)
    {
        if (*(message.begin()) == '.')
        {
            it = message.begin();
            it = message.insert(it, '.');
        }
    }
    else
    {
        if (*(message.begin()) == '.')
        {
            it = message.begin();
            it = message.insert(it, '.');
        }
        for (it = message.begin() + 2; it != message.end(); ++it)
        {
            if (*it == '\n')
            {
                if (((it + 1) != message.end()) && (*(it + 1) == '.'))
                {
                    it = message.insert(it + 1, '.');
                    ++it;
                }
            }
        }
    }

    if (message.size() < 1000)
        return;

    int count(1);

    for (it = message.begin(); it < message.end(); ++it, ++count)
    {
        if (*it == '\r')
        {
            count = 0;
            ++it;

            continue;
        }
        else if (count >= 998)
        {
            ++it;
            if (*it != ' ')
            {
                vector<char>::iterator pos = it;
                for (int j = 0; j < 997; ++j, --pos)
                {
                    if (*pos == ' ')
                    {
                        it = ++pos;
                        break;
                    }
                }
            }
            if (it < message.end())
                it = message.insert(it, '\r');
            ++it;
            if (it < message.end())
                it = message.insert(it, '\n');
            count = 0;
        }
    }

    count = 1;

    if (messageHTML.size())
    {
        for (it = messageHTML.begin(); it < messageHTML.end(); ++it, ++count)
        {
            if (*it == '\r')
            {
                count = 0;
                ++it;
                continue;
            }
            else if (count >= 998)
            {
                ++it;
                if (*it != ' ')
                {
                    vector<char>::iterator pos = it;
                    for (int j = 0; j < 997; ++j, --pos)
                    {
                        if (*pos == ' ')
                        {
                            it = ++pos;
                            break;
                        }
                    }
                }
                if (it < messageHTML.end())
                    it = messageHTML.insert(it, '\r');
                ++it;
                if (it < messageHTML.end())
                    it = messageHTML.insert(it, '\n');
                count = 0;
            }
        }
    }
}

bool Mail::setsubject(const string& newSubject)
{
    if (!newSubject.length())
        return false;

    subject = newSubject;
    return true;
}

bool Mail::setserver(const string& nameserver_or_smtpserver)
{
    if (!nameserver_or_smtpserver.length())
        return false;

    nameserver = nameserver_or_smtpserver;
    return true;
}

bool Mail::setsender(const string& newsender)
{
    if (!newsender.length())
        return false;

    Address newaddress(parseaddress(newsender));

    fromAddress = newaddress;
    return true;
}

bool Mail::addrecipient(const string& newrecipient, short recipient_type)
{
    if (recipients.size() >= 100)
        return false;

    if (newrecipient.length())
    {
        if (!recipients.size())
        {
            server = getserveraddress(newrecipient);
        }

        Address newaddress = parseaddress(newrecipient);

        if (recipient_type > Bcc || recipient_type < TO)
            recipient_type = Bcc;

        recipients.push_back(make_pair(newaddress, recipient_type));

        return true;
    }

    return false;
}

bool Mail::removerecipient(const string& recipient)
{
    if (recipient.length())
    {
        vector<pair<Address, short> >::iterator it(
                recipients.begin());

        for (; it < recipients.end(); ++it)
        {
            if ((*it).first.address == recipient)
            {
                recipients.erase(it);
                return true;
            }
        }
    }

    return false;
}

void Mail::clearrecipients()
{
    recipients.clear();
}

void Mail::clearattachments()
{
    attachments.clear();
}

void Mail::reset()
{
    recipients.clear();
    attachments.clear();

    server = "";
    message.clear();
    messageHTML.clear();
    returnstring = "";
}

void Mail::send()
{
    operator()();
}

void Mail::operator()()
{
    returnstring = "";

    if (!recipients.size())
    {
        returnstring = "451 Requested action aborted: local error who am I mailing";
        return;
    }
    if (!fromAddress.address.length())
    {
        returnstring = "451 Requested action aborted: local error who am I";
        return;
    }
    if (!nameserver.length())
    {
        returnstring = "451 Requested action aborted: local error no SMTP/name server/smtp server";
        return;
    }

    vector < SOCKADDR_IN > adds;
    if (lookupMXRecord)
    {
        if (!gethostaddresses(adds))
        {
            returnstring = "451 Requested action aborted: No MX records ascertained";
            return;
        }
    }
    else
    {
        SOCKADDR_IN addr(nameserver, port, AF_INET);
        hostent* host = 0;

        if (addr)
        {
            host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
        }
        else
            host = gethostbyname(nameserver.c_str());

        if (!host)
        {
            returnstring = "451 Requested action aborted: local error in processing";
            return;
        }

        copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
        adds.push_back(addr);
    }

    SOCKET s;

    if (!Socket(s, AF_INET, SOCK_STREAM, 0))
    {
        returnstring = "451 Requested action aborted: socket function error";
        return;
    }

    if (!adds.size())
    {
        returnstring = "451 Requested action aborted: No MX records ascertained";
    }

    const string OK("250");
    const vector<char> smtpheader(makesmtpmessage());
    const int buffsize(1024);
    char buff[buffsize] = "";

    for (vector<SOCKADDR_IN>::const_iterator address = adds.begin(); address < adds.end(); ++address)
    {
        if (!Connect(s, *address))
        {
            returnstring = "554 Transaction failed: server connect error.";

            continue;
        }

        int len1;

        if (!Recv(len1, s, buff, buffsize - 1, 0))
        {
            returnstring = "554 Transaction failed: server connect response error.";
            continue;
        }

        char hn[buffsize] = "";

        if (gethostname(hn, buffsize))
        {
            strcpy(hn, "flibbletoot");
        }

        string commandline(string("EHLO ") + hn + string("\r\n"));

        if (!Send(len1, s, commandline.c_str(), commandline.length(), 0))
        {
            returnstring = "554 Transaction failed: EHLO send";
            continue;
        }
        if (!Recv(len1, s, buff, buffsize - 1, 0))
        {
            returnstring = "554 Transaction failed: EHLO receipt";
            continue;
        }

        buff[len1] = '\0';
        string greeting = returnstring = buff;

        if (returnstring.substr(0, 3) != OK)
        {
            if (auth)
            {
                returnstring = "554 possibly trying to use AUTH without ESMTP server, ERROR!";

                continue;
            }

            commandline[0] = 'H';
            commandline[1] = 'E';

            if (!Send(len1, s, commandline.c_str(), commandline.length(), 0))
            {
                returnstring = "554 Transaction failed: HELO send";
                continue;
            }

            if (!Recv(len1, s, buff, buffsize - 1, 0))
            {
                returnstring = "554 Transaction failed: HELO receipt";
                continue;
            }

            buff[len1] = '\0';

            returnstring = buff;
            if (returnstring.substr(0, 3) != OK)
            {
                if (Send(len1, s, "QUIT\r\n", 6, 0))
                {
                    char dummy[buffsize];
                    Recv(len1, s, dummy, buffsize - 1, 0);
                }

                Closesocket(s);
                continue;
            }
        }

        if (auth)
            if (!authenticate(greeting, s))
                continue;

        commandline = "MAIL FROM:<" + fromAddress.address + ">\r\n";
        if (!Send(len1, s, commandline.c_str(), commandline.length(), 0))
        {
            returnstring = "554 MAIL FROM sending error";
            continue;
        }

        if (!Recv(len1, s, buff, buffsize - 1, 0))
        {
            returnstring = "554 MAIL FROM receipt error";
            continue;
        }

        buff[len1] = '\0';
        returnstring = buff;
        if (returnstring.substr(0, 3) != OK)
        {
            if (Send(len1, s, "QUIT\r\n", 6, 0))
            {
                char dummy[buffsize];
                Recv(len1, s, dummy, buffsize - 1, 0);
            }

            Closesocket(s);
            continue;
        }

        for (recipient_const_iter recip = recipients.begin(); recip < recipients.end(); ++recip)
        {
            commandline = "RCPT TO: <" + (*recip).first.address + ">\r\n";

            if (!Send(len1, s, commandline.c_str(), commandline.length(), 0))
            {
                returnstring = "554 Transaction failed";
                continue;
            }

            if (!Recv(len1, s, buff, buffsize - 1, 0))
            {
                returnstring = buff;
                continue;
            }

            buff[len1] = '\0';
            returnstring = buff;

            if (returnstring.substr(0, 3) != OK)
            {
                continue;
            }
        }

        if (!Send(len1, s, "DATA\r\n", 6, 0))
        {
            returnstring = "554 DATA send error";
            continue;
        }

        if (!Recv(len1, s, buff, buffsize - 1, 0))
        {
            returnstring = "554 DATA, server response error";
            continue;
        }

        buff[len1] = '\0';
        returnstring = buff;

        if (returnstring.substr(0, 3) != "354")
        {
            if (Send(len1, s, "QUIT\r\n", 6, 0))
            {
                char dummy[buffsize];
                Recv(len1, s, dummy, buffsize - 1, 0);
            }

            Closesocket(s);
            continue;
        }

        if (!Send(len1, s, &smtpheader[0], smtpheader.size(), 0))
        {
            returnstring = "554 DATA, server response error (actual send)";
            continue;
        }

        if (!Recv(len1, s, buff, buffsize - 1, 0))
        {
            returnstring = "554 DATA, server response error (actual send)";
            continue;
        }

        buff[len1] = '\0';
        returnstring = buff;
        if (returnstring.substr(0, 3) != OK)
        {
            if (Send(len1, s, "QUIT\r\n", 6, 0))
            {
                char dummy[buffsize];
                Recv(len1, s, dummy, buffsize - 1, 0);
            }

            Closesocket(s);
            continue;
        }

        if (Send(len1, s, "QUIT\r\n", 6, 0))
        {
            char dummy[buffsize];
            Recv(len1, s, dummy, buffsize - 1, 0);
        }

        if (returnstring.substr(0, 3) != "221")
        {
        }

        Closesocket(s);
        break;
    }
}

vector<char> Mail::makesmtpmessage() const
{
    string sender(fromAddress.address);

    if (sender.length())
    {
        string::size_type pos(sender.find("@"));
        if (pos != string::npos)
        {
            sender = sender.substr(0, pos);
        }
    }

    vector<char> ret;
    string headerline;

    if (fromAddress.name.length())
    {
        headerline = "From: " + fromAddress.address + " (" + fromAddress.name+ ") \r\n"
                     "Reply-To: " + fromAddress.address + "\r\n";
        ret.insert(ret.end(), headerline.begin(), headerline.end());
    }
    else
    {
        headerline = "From: " + fromAddress.address + "\r\n"
                     "Reply-To: " + fromAddress.address + "\r\n";
        ret.insert(ret.end(), headerline.begin(), headerline.end());
    }

    headerline.clear();

    vector<string> to, cc, bcc;

    for (recipient_const_iter recip = recipients.begin(); recip < recipients.end(); ++recip)
    {
        if (recip->second == TO)
        {
            to.push_back(recip->first.address);
        }
        else if (recip->second == Cc)
        {
            cc.push_back(recip->first.address);
        }
        else if (recip->second == Bcc)
        {
            bcc.push_back(recip->first.address);
        }
    }

    vec_str_const_iter it;
    int count = to.size();

    if (count)
        headerline += "To: ";

    for (it = to.begin(); it < to.end(); ++it)
    {
        headerline += *it;
        if (count > 1 && ((it + 1) < to.end()))
            headerline += ",\r\n ";
        else
            headerline += "\r\n";
    }

    count = cc.size();
    if (count)
        headerline += "Cc: ";

    for (it = cc.begin(); it < cc.end(); ++it)
    {
        headerline += *it;

        if (count > 1 && ((it + 1) < cc.end()))
            headerline += ",\r\n ";
        else
            headerline += "\r\n";
    }


    ret.insert(ret.end(), headerline.begin(), headerline.end());

    const string boundary("bounds=_NextP_0056wi_0_8_ty789432_tp");
    bool MIME(false);

    if (attachments.size() || messageHTML.size())
        MIME = true;

    if (MIME)
    {
        headerline = "MIME-Version: 1.0\r\n"
                     "Content-Type: multipart/mixed;\r\n"
                     "\tboundary=\"" + boundary + "\"\r\n";
        ret.insert(ret.end(), headerline.begin(), headerline.end());
        headerline.clear();
    }

    time_t t;
    time(&t);
    char timestring[128] = "";

    if (strftime(timestring, 127, "Date: %d %b %y %H:%M:%S %Z", localtime(&t)))
    {
        headerline = timestring;
        headerline += "\r\n";
        ret.insert(ret.end(), headerline.begin(), headerline.end());
    }

    headerline = "Subject: " + subject + "\r\n\r\n";
    ret.insert(ret.end(), headerline.begin(), headerline.end());

    if (MIME)
    {
        headerline = "This is a MIME encapsulated message\r\n\r\n";
        headerline += "--" + boundary + "\r\n";

        if (!messageHTML.size())
        {
            headerline += "Content-type: text/plain; charset=gb2312\r\n"
                          "Content-transfer-encoding: 7BIT\r\n\r\n";

            ret.insert(ret.end(), headerline.begin(), headerline.end());
            ret.insert(ret.end(), message.begin(), message.end());
            headerline = "\r\n\r\n--" + boundary + "\r\n";
        }
        else
        {
            const string innerboundary("inner_jfd_0078hj_0_8_part_tp");
            headerline += "Content-Type: multipart/alternative;\r\n"
                          "\tboundary=\"" + innerboundary + "\"\r\n";
            headerline += "\r\n\r\n--" + innerboundary + "\r\n";
            headerline += "Content-type: text/plain; charset=gb2312\r\n"
                          "Content-transfer-encoding: 7BIT\r\n\r\n";
            ret.insert(ret.end(), headerline.begin(), headerline.end());
            ret.insert(ret.end(), message.begin(), message.end());
            headerline = "\r\n\r\n--" + innerboundary + "\r\n";
            headerline += "Content-type: text/html; charset=gb2312\r\n"
                          "Content-Transfer-Encoding: base64\r\n\r\n";
            ret.insert(ret.end(), headerline.begin(), headerline.end());
            ret.insert(ret.end(), messageHTML.begin(), messageHTML.end());
            headerline = "\r\n\r\n--" + innerboundary + "--\r\n";

            if (!attachments.size())
                headerline += "\r\n--" + boundary + "--\r\n";
            else
                headerline += "\r\n--" + boundary + "\r\n";
        }

        ret.insert(ret.end(), headerline.begin(), headerline.end());
        headerline.clear();

        for (vec_pair_char_str_const_iter it1 = attachments.begin(); it1 != attachments.end(); ++it1)
        {
            if (it1->second.length() > 3)
            {
                string typ(it1->second.substr(it1->second.length() - 4, 4));

                if (typ == ".gif")
                {
                    headerline += "Content-Type: image/gif;\r\n";
                }
                else if (typ == ".jpg" || typ == "jpeg")
                {
                    headerline += "Content-Type: image/jpg;\r\n";
                }
                else if (typ == ".txt")
                {
                    headerline += "Content-Type: plain/txt;\r\n";
                }
                else if (typ == ".bmp")
                {
                    headerline += "Content-Type: image/bmp;\r\n";
                }
                else if (typ == ".htm" || typ == "html")
                {
                    headerline += "Content-Type: plain/htm;\r\n";
                }
                else if (typ == ".png")
                {
                    headerline += "Content-Type: image/png;\r\n";
                }
                else if (typ == ".exe")
                {
                    headerline += "Content-Type: application/X-exectype-1;\r\n";
                }
                else
                {
                    headerline += "Content-Type: application/X-other-1;\r\n";
                }
            }
            else
            {
                headerline += "Content-Type: application/X-other-1;\r\n";
            }

            headerline += "\tname=\"" + it1->second + "\"\r\n";
            headerline += "Content-Transfer-Encoding: base64\r\n";
            headerline += "Content-Disposition: attachment; filename=\"" + it1->second + "\"\r\n\r\n";
            ret.insert(ret.end(), headerline.begin(), headerline.end());
            headerline.clear();

            ret.insert(ret.end(), it1->first.begin(), it1->first.end());

            if ((it1 + 1) == attachments.end())
                headerline += "\r\n\r\n--" + boundary + "--\r\n";
            else
                headerline += "\r\n\r\n--" + boundary + "\r\n";

            ret.insert(ret.end(), headerline.begin(), headerline.end());
            headerline.clear();
        }
    }
    else
        ret.insert(ret.end(), message.begin(), message.end());

    headerline = "\r\n.\r\n";
    ret.insert(ret.end(), headerline.begin(), headerline.end());

    return ret;
}

bool Mail::attach(const string& filename)
{
    if (!filename.length())
        return false;

    vector<char> filedata;
    string str = readFile(filename);
    stringToVector<char>(Base64::encode((ubyte*)str.c_str(), str.size()), filedata);

    string fn(filename);
    string::size_type p = fn.find_last_of('/');

    if (p == string::npos)
        p = fn.find_last_of('\\');

    if (p != string::npos)
    {
        p += 1;
        fn = fn.substr(p, fn.length() - p);
    }

    attachments.push_back(make_pair(filedata, fn));

    return true;
}

bool Mail::removeattachment(const string& filename)
{
    if (!filename.length())
        return false;

    if (!attachments.size())
        return false;

    string fn(filename);
    string::size_type p = fn.find_last_of('/');

    if (p == string::npos)
        p = fn.find_last_of('\\');

    if (p != string::npos)
    {
        p += 1;
        fn = fn.substr(p, fn.length() - p);
    }

    vector<pair<vector<char>, string> >::iterator it;

    for (it = attachments.begin(); it < attachments.end(); ++it)
    {
        if ((*it).second == fn)
        {
            attachments.erase(it);

            return true;
        }
    }

    return false;
}

string Mail::getserveraddress(const string& toaddress) const
{
    if (toaddress.length())
    {
        string::size_type pos(toaddress.find("@"));

        if (pos != string::npos)
        {
            if (++pos < toaddress.length())
                return toaddress.substr(pos, toaddress.length() - pos);
        }
    }
    return "";
}

bool Mail::gethostaddresses(vector<SOCKADDR_IN>& adds)
{
    adds.clear();

    SOCKADDR_IN addr(nameserver, htons(DNS_PORT), AF_INET);

    hostent* host = 0;
    if (addr)
        host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
    else
        host = gethostbyname(nameserver.c_str());

    if (!host)
    {
        addr = SOCKADDR_IN(server, port);
        addr.ADDR.sin_port = port;

        if (addr)
        {
            host = gethostbyaddr(addr.get_sin_addr(), sizeof(addr.ADDR.sin_addr), AF_INET);
        }
        else
            host = gethostbyname(server.c_str());

        if (!host)
        {
            returnstring = "550 Requested action not taken: mailbox unavailable";

            return false;
        }

        copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
        adds.push_back(addr);

        return true;
    }
    else
        copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());

    SOCKET s;

    if (!Socket(s, AF_INET, SOCK_DGRAM, 0))
    {
        returnstring = "451 Requested action aborted: socket function error";

        return false;
    }

    if (!Connect(s, addr))
    {
        returnstring = "451 Requested action aborted: dns server unavailable";

        return false;
    }

    unsigned char dns[512] = { 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 };
    int dnspos = 12;
    string::size_type stringpos(0);
    string::size_type next(server.find("."));

    if (next != string::npos)
    {
        while (stringpos < server.length())
        {
            string part(server.substr(stringpos, next - stringpos));
            dns[dnspos] = part.length();
            ++dnspos;

            for (string::size_type i = 0; i < part.length(); ++i, ++dnspos)
            {
                dns[dnspos] = part[i];
            }

            stringpos = ++next;
            next = server.find(".", stringpos);

            if (next == string::npos)
            {
                part = server.substr(stringpos, server.length() - stringpos);
                dns[dnspos] = part.length();
                ++dnspos;

                for (string::size_type i = 0; i < part.length(); ++i, ++dnspos)
                {
                    dns[dnspos] = part[i];
                }

                break;
            }
        }
    }
    else
    {
        dns[dnspos] = server.length();
        ++dnspos;

        for (string::size_type i = 0; i < server.length(); ++i, ++dnspos)
        {
            dns[dnspos] = server[i];
        }
    }

    if (server[server.length() - 1] == '.')
        dns[dnspos] = 0;
    else
        dns[dnspos++] = 0;

    dns[dnspos++] = 0;
    dns[dnspos++] = 15;

    dns[dnspos++] = 0;
    dns[dnspos++] = 1;

    int ret;

    if (!Send(ret, s, (char*) dns, dnspos, 0))
    {
        returnstring = "451 Requested action aborted: server seems to have disconnected.";

        Closesocket(s);
        return false;
    }

    if (Recv(ret, s, (char*) dns, 512, 0))
    {
        Closesocket(s);

        if (dnspos > 12)
        {
            unsigned short numsitenames    = ((unsigned short) dns[4]  << 8) | dns[5];
            unsigned short numanswerRR     = ((unsigned short) dns[6]  << 8) | dns[7];
            unsigned short numauthorityRR  = ((unsigned short) dns[8]  << 8) | dns[9];
            unsigned short numadditionalRR = ((unsigned short) dns[10] << 8) | dns[11];

            if (!(dns[3] & 0x0F))
            {
                int pos = 12;

                string questionname;

                if (numsitenames)
                {
                    parsename(pos, dns, questionname);
                    pos += 4;
                }

                vector < string > names;
                in_addr address;
                string name;

                int num = 0;

                for (; num < numanswerRR; ++num)
                {
                    name = "";
                    parseRR(pos, dns, name, address);

                    if (name.length())
                        names.push_back(name);
                }

                for (num = 0; num < numauthorityRR; ++num)
                {
                    name = "";
                    parseRR(pos, dns, name, address);

                    if (name.length())
                        names.push_back(name);
                }

                for (num = 0; num < numadditionalRR; ++num)
                {
                    name = "";
                    parseRR(pos, dns, name, address);

                    if (name.length())
                        names.push_back(name);
                }

                addr.ADDR.sin_family = AF_INET;
                addr.ADDR.sin_port = port; // smtp port!! 25
                hostent* host = 0;

                for (vec_str_const_iter it = names.begin(); it < names.end(); ++it)
                {
                    host = gethostbyname(it->c_str());

                    if (!host)
                    {
                        addr.zeroaddress();
                        continue; // just skip it!!!
                    }

                    copy(host->h_addr_list[0], host->h_addr_list[0] + host->h_length, addr.get_sin_addr());
                    adds.push_back(addr);
                }

                return true;
            }
        }
    }
    else
        Closesocket(s);

    return false;
}


bool Mail::parseRR(int& pos, const unsigned char dns[], string& name, in_addr& address)
{
    if (pos < 12)
        return false;
    if (pos > 512)
        return false;

    int len = dns[pos];

    if (len >= 192)
    {
        int pos1 = dns[++pos];
        len = dns[pos1];
    }
    else
    {
        parsename(pos, dns, name);
    }

    unsigned short a = ((unsigned short) dns[++pos] << 8);
    unsigned short b = dns[++pos];
    unsigned short Type = a | b;
    a = ((unsigned short) dns[++pos] << 8);
    b = dns[++pos];

    pos += 4;
    a = ((unsigned short) dns[++pos] << 8);
    b = dns[++pos];
    unsigned short Datalen = a | b;

    if (Type == 15)
    {
        a = ((unsigned short) dns[++pos] << 8);
        b = dns[++pos];

        len = dns[++pos];

        if (len >= 192)
        {
            int pos1 = dns[++pos];
            parsename(pos1, dns, name);
        }
        else
            parsename(pos, dns, name);
    }
    else if (Type == 12)
    {
        pos += Datalen + 1;
    }
    else if (Type == 2)
    {
        pos += Datalen + 1;
    }
    else if (Type == 1)
    {
        pos += Datalen + 1;
    }
    else
    {
        pos += Datalen + 1;
    }

    return true;
}

void Mail::parsename(int& pos, const unsigned char dns[], string& name)
{
    int len = dns[pos];

    if (len >= 192)
    {
        int pos1 = ++pos;
        ++pos;
        parsename(pos1, dns, name);
    }
    else
    {
        for (int i = 0; i < len; ++i)
            name += dns[++pos];
        len = dns[++pos];

        if (len != 0)
            name += ".";

        if (len >= 192)
        {
            int pos1 = dns[++pos];
            ++pos;
            parsename(pos1, dns, name);
        }
        else if (len > 0)
        {
            parsename(pos, dns, name);
        }
        else if (len == 0)
            ++pos;
    }
}

const string& Mail::response() const
{
    return returnstring;
}

Mail::Address Mail::parseaddress(const string& addresstoparse)
{
    Address newaddress;

    if (!addresstoparse.length())
        return newaddress;

    if (addresstoparse.find("@") != string::npos)
    {
        newaddress.address = addresstoparse;
        return newaddress;
    }

    if (((addresstoparse.find('<') != string::npos) && (addresstoparse.find('>') == string::npos)) || ((addresstoparse.find('>') != string::npos) && (addresstoparse.find('<') == string::npos)))
    {
        return newaddress;
    }

    if ((addresstoparse.find('<') != string::npos) && (addresstoparse.find('>') != string::npos))
    {
        string::size_type sta = addresstoparse.find('<');
        string::size_type end = addresstoparse.find('>');

        newaddress.address = addresstoparse.substr(sta + 1, end - sta - 1);

        if (sta > 0)
        {
            newaddress.name = addresstoparse.substr(0, sta);
            return newaddress;
        }
        else
        {
            if (end >= addresstoparse.length() - 1)
                return newaddress;

            end += 2;
            if (end >= addresstoparse.length())
                return newaddress;

            newaddress.name = addresstoparse.substr(end, addresstoparse.length() - end);

            if (newaddress.name[newaddress.name.length() - 1] == ' ')
                newaddress.name = newaddress.name.substr(0, newaddress.name.length() - 1);

            return newaddress;
        }
    }

    newaddress.address = addresstoparse;

    return newaddress;
}

void Mail::authtype(const enum authtype Type)
{
    assert(Type == LOGIN || Type == PLAIN);
    type = Type;
}

void Mail::username(const string& User)
{
    auth = (User.length() != 0);
    user = User;
}

void Mail::password(const string& Pass)
{
    pass = Pass;
}

bool Mail::authenticate(const string& servergreeting, const SOCKET& s)
{
    assert(auth && user.length());
    int len(0);

    if (!user.length())
    {
        Send(len, s, "QUIT\r\n", 6, 0);

        return false;
    }

    string at;
    if (type == LOGIN)
        at = "LOGIN";
    else if (type == PLAIN)
        at = "PLAIN";
    else
    {
        assert(false);
        returnstring = "554 jwSMTP only handles LOGIN or PLAIN authentication at present!";
        Send(len, s, "QUIT\r\n", 6, 0);

        return false;
    }

    string greeting(servergreeting);

    for (string::size_type pos = 0; pos < greeting.length(); ++pos)
    {
        greeting[pos] = toupper(greeting[pos]);
    }

    if (greeting.find(at) == string::npos)
    {
        returnstring = "554 jwSMTP only handles LOGIN or PLAIN authentication at present!";
        Send(len, s, "QUIT\r\n", 6, 0);
        return false;
    }

    const int buffsize(1024);
    char buff[buffsize];

    if (type == LOGIN)
    {
        greeting = "auth " + at + "\r\n";

        if (!Send(len, s, greeting.c_str(), greeting.length(), 0))
        {
            returnstring = "554 send failure: \"auth " + at + "\"";
            return false;
        }

        if (!Recv(len, s, buff, buffsize, 0))
        {
            returnstring = "554 receive failure: waiting on username question!";
            return false;
        }

        buff[len] = '\0';
        returnstring = buff;

        if (returnstring.substr(0, 16) != "334 VXNlcm5hbWU6")
        {
            Send(len, s, "QUIT\r\n", 6, 0);
            return false;
        }

        greeting = Base64::encode((ubyte*)user.c_str(), user.size()) + "\r\n";

        if (!Send(len, s, greeting.c_str(), greeting.length(), 0))
        {
            returnstring = "554 send failure: sending username";
            return false;
        }

        if (!Recv(len, s, buff, buffsize, 0))
        {
            returnstring = "554 receive failure: waiting on password question!";
            return false;
        }

        buff[len] = '\0';
        returnstring = buff;

        if (returnstring.substr(0, 16) != "334 UGFzc3dvcmQ6")
        {
            Send(len, s, "QUIT\r\n", 6, 0);
            return false;
        }

        greeting = Base64::encode((ubyte*)pass.c_str(), pass.size()) + "\r\n";

        if (!Send(len, s, greeting.c_str(), greeting.length(), 0))
        {
            returnstring = "554 send failure: sending password";
            return false;
        }

        if (!Recv(len, s, buff, buffsize, 0))
        {
            returnstring = "554 receive failure: waiting on auth login response!";
            return false;
        }

        buff[len] = '\0';
        returnstring = buff;

        if (returnstring.substr(0, 3) == "235")
            return true;
    }
    else if (type == PLAIN)
    {
        vector<char> enc;
        string::size_type pos = 0;
        for (; pos < user.length(); ++pos)
            enc.push_back(user[pos]);
        enc.push_back('\0');
        for (pos = 0; pos < user.length(); ++pos)
            enc.push_back(user[pos]);
        enc.push_back('\0');
        for (pos = 0; pos < pass.length(); ++pos)
            enc.push_back(pass[pos]);

        string str = vectorToString<char>(enc);
        stringToVector<char>(Base64::encode((ubyte*)str.c_str(), str.size()), enc);

        greeting = "auth plain ";
        for (vector<char>::const_iterator it1 = enc.begin(); it1 < enc.end(); ++it1)
            greeting += *it1;
        greeting += "\r\n";

        if (!Send(len, s, greeting.c_str(), greeting.length(), 0))
        {
            returnstring = "554 send failure: sending login:plain authenication info";
            return false;
        }
        if (!Recv(len, s, buff, buffsize, 0))
        {
            returnstring = "554 receive failure: waiting on auth plain autheticated response!";
            return false;
        }
        buff[len] = '\0';
        returnstring = buff;

        if (returnstring.substr(0, 3) == "235")
            return true;
    }

    Send(len, s, "QUIT\r\n", 6, 0);
    return false;
}

}
}
}
