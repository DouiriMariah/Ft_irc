#include "Server.hpp"

void Server::ft_quit(std::vector<std::string> cmd)
{
    if(!user.size())
        return;
    std::string msgi = user_tmp->second->getNickName() + " has quit";
    std::string msg;
    cmd[1][0] = ' ';
    for(size_t i = 1; i < cmd.size(); i++)
        msg = msg + " " + cmd[i];
    message(SERVER_NAME, "QUIT", user_tmp->second->getNickName(), msg, user_tmp->first);
    std::map<std::string, Channel*> *channel_list = user_tmp->second->getChan();
    if(channel_list->size() != 0)
    {
        std::map<std::string, Channel*> ::iterator users_channel = channel_list->begin();
        std::map<int, std::string> *access_user = users_channel->second->getAccessUser();
        while(users_channel != channel_list->end())
        {
            std::map<int, std::string > ::iterator to_erase;
            if((to_erase = access_user->find(user_tmp->first)) != access_user->end())
                access_user->erase(to_erase);
            if(!access_user->size())
            {
                chan.erase(chan.find(users_channel->first));
                delete(users_channel->second);
            }
            users_channel++;
        }
        channel_list->clear();
    }
    FD_CLR(user_tmp->first, &current_sockets);
    delete(user_tmp->second);
    user_tmp->second = NULL;
    user.erase(user_tmp);
    user_tmp = user.begin(); 
    while(user_tmp != user.end())
    {
        message(SERVER_NAME, "", msgi, msg, user_tmp->first);
        user_tmp++;
    }
}

void    Server::ft_cap(std::vector<std::string> cmds)
{
    (void)cmds;
    return;
}

void    Server::ft_nick(std::vector<std::string> cmds)
{
    if(user_tmp->second->getPass() < 2)
        return;
    if (cmds.size() == 1)
    {
        message(SERVER_NAME, "", "", "Your nickname is " + user_tmp->second->getNickName(), user_tmp->first); //inutile
        return ;
    }
    std::map<int, User *>::iterator it = user.begin();
    while(it != user.end())
    {
        if (it->second->getNickName() == cmds[1])
            break;
        it++;
    }
    if (cmds[1].find_first_of("#_") != std::string::npos)
        return ;
    if (it != user.end())
    {
        message(SERVER_NAME, "433", user_tmp->second->getNickName(), cmds[1], user_tmp->first); // Nick already in use
        return ;
    }
    if (user_tmp->second->getNickName() != "")
        message("nick", "NICK", "", cmds[1], user_tmp->first); 
    user_tmp->second->setNickName(cmds[1]);
}

void    Server::ft_join_bis(std::vector<std::string> cmds)
{
    std::string tmp;
    std::map<std::string, Channel *>::iterator ch = chan.find(cmds[1]);
    if (ch != chan.end())
    {
        if ((chan.find(cmds[1])->second->getMode()).find_first_of("i") != std::string::npos)
        {
            if (chan.find(cmds[1])->second->getAccessUser()->find(user_tmp->first)->second.find_first_of("i") == std::string::npos)
            {
                message(SERVER_NAME, "473", cmds[1], cmds[1], user_tmp->first);
                return ;
            }
        }
        ch->second->getAccessUser()->insert(std::pair<int, std::string>(user_tmp->first, tmp));
        user_tmp->second->getChan()->insert(std::pair<std::string, Channel *>(cmds[1], ch->second));
        std::string listusers;
        std::map<int, std::string>::iterator tt = ch->second->getAccessUser()->begin();
        while(tt != ch->second->getAccessUser()->end()) 
        {
            if (listusers.size() != 0)
                listusers += " ";
            if (tt->second.find_first_of("0o") != std::string::npos)
                listusers += "@";
            listusers += getUser(tt->first);
            tt++;
        }
        message("nick", "353", user_tmp->second->getNickName() + " = " + cmds[1], listusers, user_tmp->first);
        message("nick", "366", user_tmp->second->getNickName() + ' ' + cmds[1], "End of /NAMES list.", user_tmp->first);
        message(SERVER_NAME, "332", user_tmp->second->getNickName() + ' ' + cmds[1], chan.find(cmds[1])->second->getTopic(), user_tmp->first);
        messageToChannel("nick", "JOIN", "", cmds[1], cmds[1]);
        return ;
    }

    Channel *new_chan = new Channel(cmds[1]);
    chan.insert(std::pair<std::string, Channel *>(cmds[1], new_chan));
    user_tmp->second->getChan()->insert(std::pair<std::string, Channel *>(cmds[1], new_chan));
    tmp += "+0";
    if(cmds.size() > 2)
            tmp += cmds[2];
    new_chan->getAccessUser()->insert(std::pair<int, std::string>(user_tmp->first, tmp));
    message("nick", "353", user_tmp->second->getNickName() + " = " + cmds[1], "@" + user_tmp->second->getNickName(), user_tmp->first);
    message("nick", "366", user_tmp->second->getNickName() + ' ' + cmds[1], "", user_tmp->first);
    message(SERVER_NAME, "332", user_tmp->second->getNickName() + ' ' + cmds[1], chan.find(cmds[1])->second->getTopic(), user_tmp->first);
    message("nick", "JOIN", "", cmds[1], user_tmp->first);
}

void    Server::ft_join(std::vector<std::string> cmds)
{
    if (cmds.size() == 1)
    {
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
        return ;
    }
    std::stringstream multiple_chan(cmds[1]);
    std::string str;
    while (std::getline(multiple_chan, str, ',')) 
    {
        cmds[1] = str;
        ft_join_bis(cmds);
    }
}

void    Server::ft_part(std::vector<std::string> cmds)
{
    std::map<std::string, Channel *>::iterator ch = user_tmp->second->getChan()->find(cmds[1]);
    if (cmds.size() == 1)
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
    else if (ch != user_tmp->second->getChan()->end())
    {
        std::string sendmsg;
        for (size_t i = 2; i < cmds.size(); i++)
        {
            sendmsg += cmds[i];
            if (i + 1 < cmds.size())
                sendmsg += " ";
        }
        messageToChannel("nick", cmds[0], cmds[1], sendmsg, cmds[1]);
        user_tmp->second->getChan()->erase(ch);
        chan.find(cmds[1])->second->getAccessUser()->erase(user_tmp->first);
        if (chan.find(cmds[1])->second->getAccessUser()->size() == 0)
        {
            ch = chan.find(cmds[1]);
            delete ch->second;
            chan.erase(chan.find(ch->first));
        }
    }
    else
        message(SERVER_NAME, "403", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
}

void    Server::ft_list(std::vector<std::string> cmds)
{
    std::map<std::string, Channel*> ::iterator channel;
    std::string g;
    std::string topic = "Topic isn't set";
    if(cmds.size() == 1)
    {
        channel = chan.begin();
        while(channel != chan.end())
        {
            if(channel->second->getTopic() != "")
                topic = channel->second->getTopic();
            else
                topic = "Topic isn't set";
            std::stringstream msg;
            msg << channel->second->getAccessUser()->size();
            g = msg.str();
            message(SERVER_NAME, "322", 
                user_tmp->second->getNickName() + " " + channel->first + " " + 
                   g + ' ' + topic, "", user_tmp->first);
            channel++;
        }
    }
    else
    {
        std::stringstream msg;
        channel = chan.find(cmds[1]);
        if(channel == chan.end())
            return;
        if(channel->second->getTopic() != "")
            topic = channel->second->getTopic();
        msg << channel->second->getAccessUser()->size();
        g = msg.str();
        message(SERVER_NAME, "322", 
                user_tmp->second->getNickName() + " " + channel->first + " " + 
                   g, topic, user_tmp->first);
    }
    message(SERVER_NAME, "323", 
        user_tmp->second->getNickName(), "End of /LIST" , user_tmp->first);
}

void    Server::ft_kick(std::vector<std::string> cmds)
{
    std::map<std::string, Channel *>::iterator ch = user_tmp->second->getChan()->find(cmds[1]);
    if (ch != user_tmp->second->getChan()->end())
    {
        if (chan.find(cmds[1])->second->getAccessUser()->find(user_tmp->first)->second.find_first_of("0o") != std::string::npos)
        {
            std::map<int, User*>::iterator it = user.begin();
            while (it != user.end())
            {
                if (it->second->getNickName() == cmds[2])
                {
                    if (chan.find(cmds[1])->second->getAccessUser()->find(it->first)->second.find_first_of("0o") != std::string::npos)
                        ;
                    else
                    {
                        std::string sendmsg;
                        for (size_t i = 3; i < cmds.size(); i++)
                        {
                            sendmsg += cmds[i];
                            if (i + 1 < cmds.size())
                                sendmsg += " ";
                        }
                        message(it->second->getNickName() +"!" + it->second->getInfo(0) +"@"+it->second->getInfo(2), "PART", cmds[1], sendmsg, it->first);
                        messageToChannel("nick", cmds[0], cmds[1] + " " + it->second->getNickName(), sendmsg, cmds[1]);
                        it->second->getChan()->erase(it->second->getChan()->find(cmds[1]));
                        chan.find(cmds[1])->second->getAccessUser()->erase(it->first);
                    }
                    return ;
                }
                it++;
            }
        }    
    }
    else
        message(SERVER_NAME, "403", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
}

void Server::ft_names(std::vector<std::string> cmds)
{
    std::map<std::string, Channel*> ::iterator channel;
    if(cmds.size() != 1)
    {
        channel = chan.find(cmds[1]);
        if(channel != chan.end())
        {
            list:
                std::string list_names;
                std::map<int, std::string> ::iterator user_chan = channel->second->getAccessUser()->begin();
                while(user_chan != channel->second->getAccessUser()->end())
                {
                    if(user.find(user_chan->first) != user.end())
                    {
                        list_names = list_names + user_chan->second + user.find(user_chan->first)->second->getNickName();
                        // message("nick", "353", cmd[1], )
                    }
                    user_chan++;
                }
                message("nick", "353", user_tmp->second->getNickName() + "=" + cmds[1], list_names, user_tmp->first);
        }
    }
    else
    {
        channel = chan.begin();
        while(channel != chan.end())
        {
            cmds[1] = channel->first;
            goto list;
            channel++;
        }   
    }
    message("nick", "366", user_tmp->second->getNickName(), "End of names list", user_tmp->first);
}

void    Server::ft_motd()
{
    send(user_tmp->first, "Message of the day\n", 19, 0);
}

void Server::ft_channel_mode(std::vector<std::string> cmds)
{
    if (cmds[2].find_first_not_of("+-io") != std::string::npos || (cmds[2][0] != '+' && cmds[2][0] != '-') || (std::count(cmds[2].begin(), cmds[2].end(), '+') + std::count(cmds[2].begin(), cmds[2].end(), '-') != 1))
        message(SERVER_NAME, "472", user_tmp->second->getNickName(), cmds[2], user_tmp->first);
    std::string tmp = chan.find(cmds[1])->second->getMode();
    if (cmds[2].size() > 2)
    {
        message(SERVER_NAME, "", user_tmp->second->getNickName(), "We don't accept multiple changes at once in MODE", user_tmp->first);
        return ;
    }
    if (cmds.size() == 3)
    {
        if (cmds[2].find_first_of("+") != std::string::npos)
        {
            if (cmds[2].find_first_of("i") != std::string::npos && tmp.find_first_of("i") == std::string::npos)
                    tmp += 'i';
        }
        else if (cmds[2].find_first_of("-") != std::string::npos)
        {
            if (cmds[2].find_first_of("i") != std::string::npos && tmp.find_first_of("i") != std::string::npos)
                    tmp.erase(tmp.find('i'), 1);
        }
        if (tmp != chan.find(cmds[1])->second->getMode())
        {
            messageToChannel("nick", "MODE", cmds[1] + " +" + tmp, "", cmds[1]);
            chan.find(cmds[1])->second->setMode(tmp);
        }
    }
    else if (cmds.size() > 3)
    {
        std::map<int, std::string>::iterator tt = chan.find(cmds[1])->second->getAccessUser()->begin();
        while(tt != chan.find(cmds[1])->second->getAccessUser()->end()) 
        {
            if (getUser(tt->first) == cmds[3])
                break ;
            tt++;
        }
        if (tt == chan.find(cmds[1])->second->getAccessUser()->end())
        {
            message(SERVER_NAME, "401", user_tmp->second->getNickName(), "No corresponding user", user_tmp->first);
            return ;
        }
        std::string tmp1 = tt->second;
        if (cmds[2].find_first_of("+") != std::string::npos)
        {
            if (cmds[2].find_first_of("o") != std::string::npos)
            {
                if (tmp1.find_first_of("o") == std::string::npos)
                    tmp1 += 'o';
            }
        }
        else if (cmds[2].find_first_of("-") != std::string::npos) 
        {
            if (cmds[2].find_first_of("o") != std::string::npos)
            {
                if (tmp1.find_first_of("o") != std::string::npos)
                    tmp1.erase(tmp1.find('o'), 1);
            }
        }
        if (tmp1 != tt->second)
        {
            chan.find(cmds[1])->second->setAccessUser(tt->first, tmp1);
            message("nick", "MODE", cmds[3] + " +" + tmp1, "", tt->first);
        }
    }
}

void    Server::ft_mode(std::vector<std::string> cmds)
{
    if (cmds.size() < 2)
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), "Number of parameters is wrong", user_tmp->first);
    std::map<std::string, Channel*>::iterator it = user_tmp->second->getChan()->find(cmds[1]);
    std::map<int, User *>::iterator it1 = user.begin();
    while(it1 != user.end())
    {
        if (it1->second->getNickName() == cmds[1])
        {
            if (cmds.size() == 2)
                message("nick", "221", cmds[1] + " +" + chan.find(cmds[1])->second->getMode(), "", user_tmp->first);
            break;
        }
        it1++;
    }
    if (it != user_tmp->second->getChan()->end())
    {
        if (cmds.size() == 2)
            message("nick", "MODE", cmds[1] + " +" + chan.find(cmds[1])->second->getMode(), "", user_tmp->first);
        else if (it->second->getAccessUser()->find(user_tmp->first) != it->second->getAccessUser()->end() && it->second->getAccessUser()->find(user_tmp->first)->second.find_first_of("0o") == std::string::npos)
            message(SERVER_NAME, "", "", "You are not a channel operator on " + cmds[1] + " channel", user_tmp->first);
        else
            ft_channel_mode(cmds);
    } 
    else if (it == user_tmp->second->getChan()->end() && it1 == user.end())
        message(SERVER_NAME, "403", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
}

void    Server::ft_notice(std::vector<std::string> cmds)
{
    if (cmds.size() < 3)
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
    else
    {
        std::string sendmsg;
        for (size_t i = 2; i < cmds.size(); i++)
        {
            sendmsg += cmds[i];
            if (i + 1 < cmds.size())
                sendmsg += " ";
        }
        std::map<std::string, Channel*> ::iterator channel = chan.find(cmds[1]);
        if(channel == chan.end())
        {
            if(cmds[1][0] == '#')
                return;
            std::map<int, User *> ::iterator target_user = user.begin();
            while(target_user != user.end())
            {
                if(target_user->second->getNickName() == cmds[1])
                {
                    message("nick", cmds[0], user_tmp->second->getNickName(), sendmsg, target_user->first);
                    return;
                }
                target_user++;
            }
        }
        else
        {
            std::map<int, std::string> ::iterator users = channel->second->getAccessUser()->begin();
            while(users != channel->second->getAccessUser()->end())
            {
                if(users->first != user_tmp->first)
                    message("nick", cmds[0], user_tmp->second->getNickName() + " " + channel->first, sendmsg, users->first);
                users++;
            }
        }
    }
}

int Server::checkInfoUser()
{
    if(user.size() == 1 && !user_tmp->second->getInfoUser()->size())
        return(0);
    std::map<int, User*> ::iterator comp = user.begin();
    std::string nickname_user = user_tmp->second->getNickName();
    while(comp != user.end())
    {
        if((comp->first == user_tmp->first || nickname_user != comp->second->getNickName()))
            comp++;
        else
            return(comp->first);
    }
    return(0);
}

void Server::ft_user(std::vector<std::string> cmds)
{
    if(user_tmp->second->getPass() < 2)
        return;
    int cmp;
    std::string send_msg = "ERR_NEEDMOREPARAMS";
    if (cmds.size() != 6 )
        message(SERVER_NAME, "461", user_tmp->second->getNickName(),  send_msg , user_tmp->first);
    else if(!(cmp = checkInfoUser()) && user_tmp->second->getNickName() != "")
    {
        for (size_t i = 1; i != cmds.size(); i++)
            user_tmp->second->getInfoUser()->push_back(cmds[i]);
        message(SERVER_NAME, "001", user_tmp->second->getNickName(), "Welcome to the Internet Relay Network " + user_tmp->second->getNickName(), user_tmp->first);
    }
    else
        message(SERVER_NAME, "462", user_tmp->second->getNickName(),  "Already registered" , user_tmp->first);
}

void Server::ft_pass(std::vector<std::string> cmds)
{
    if(cmds.size() != 2)
    {
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), "Need more params", user_tmp->first);
        user_tmp->second->setPass(1);
    }
    else if(cmds[1] != password)
    {
        user_tmp->second->setPass(1);
        message(SERVER_NAME, "464", user_tmp->second->getNickName(), "Wrong password for this server", user_tmp->first);
    }
    else if(user_tmp->second->getPass() == 2)
    {
        user_tmp->second->setPass(1);
        message(SERVER_NAME, "462", user_tmp->second->getNickName(),  "Already registered" , user_tmp->first);
    }
    else
        user_tmp->second->setPass(2);
}

void Server::ft_topic(std::vector<std::string> cmds)
{
    if(cmds.size() == 1)
    {
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), "Need more params", user_tmp->first);
        return;
    }
    std::map<std::string, Channel*> ::iterator channel = chan.find(cmds[1]);
    if(channel == chan.end())
    {
        message(SERVER_NAME, "403", user_tmp->second->getNickName(), cmds[1], user_tmp->first);
        return;
    }
    if(cmds.size() == 2)
    {
        if(channel->second->getAccessUser()->find(user_tmp->first) != channel->second->getAccessUser()->end())
        {
            if(channel->second->getTopic() == "")
                message(SERVER_NAME, "331", cmds[1], cmds[1] + " : No topic is set!", user_tmp->first);
            else
                message(SERVER_NAME, "332", cmds[1], channel->second->getTopic(), user_tmp->first);
        }
        else
            message("nick", "442", cmds[1], "You're not on that channel", user_tmp->first);
    }
    else if(cmds.size() > 2 && channel->second->getAccessUser()->find(user_tmp->first)->second.find_first_of("0o") != std::string::npos)
    {
        if(cmds[2].find_first_not_of(" \0") == std::string::npos)
        {
            channel->second->setTopic("");
            messageToChannel("nick", cmds[0], cmds[1], "", cmds[1]);
        }
        else
        {
            channel->second->setTopic(cmds[2]);
            messageToChannel("nick", cmds[0], cmds[1], channel->second->getTopic(), cmds[1]);
        }
    }
}

void Server::ft_privmsg(std::vector<std::string>cmds)
{
    if(cmds.size() == 2)
    {
        message("nick", "412", user_tmp->second->getNickName(), "No message given", user_tmp->first);
        return;
    }
    std::string sendmsg;
    for (size_t i = 2; i < cmds.size(); i++)
    {
        sendmsg += cmds[i];
        if (i + 1 < cmds.size())
            sendmsg += " ";
    }
    std::map<std::string, Channel*> ::iterator channel = chan.find(cmds[1]);
    if(channel == chan.end())
    {
        if(cmds[1][0] == '#')
            return;
        std::map<int, User *> ::iterator target_user = user.begin();
        while(target_user != user.end())
        {
            if(target_user->second->getNickName() == cmds[1])
            {
                message("nick", cmds[0], user_tmp->second->getNickName(), sendmsg, target_user->first);
                break;
            }
            target_user++;
        }
        if(target_user == user.end())
            message("nick", "401", user_tmp->second->getNickName(), "ERR_NOSUCHNICK", user_tmp->first);
    }
    else
    {
        std::map<int, std::string> ::iterator users = channel->second->getAccessUser()->begin();
        while(users != channel->second->getAccessUser()->end())
        {
            if(users->first != user_tmp->first)
                message("nick", cmds[0], cmds[1], sendmsg, users->first);
            users++;
        }
    }
}

void Server::ft_oper(std::vector<std::string> cmds)
{
    if(cmds.size() != 3)
    {
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), "Wrong number of Params", user_tmp->first);
        return;
    }
    if(cmds[2] != PASS_OP)
    {
        message(SERVER_NAME, "464", user_tmp->second->getNickName(), "Wrong password : operator init", user_tmp->first);
        return;
    }
    user_tmp->second->setOper();
    message(SERVER_NAME, "", user_tmp->second->getNickName(), "You are now an IRC operator", user_tmp->first);
    message(SERVER_NAME, "221", user_tmp->second->getNickName(), "o", user_tmp->first);
    std::map<std::string, Channel*>::iterator it = user_tmp->second->getChan()->begin();
    while (it != user_tmp->second->getChan()->end())
    {
        it->second->getAccessUser()->find(user_tmp->first)->second += "o";
        it++;
    }
}

void    Server::ft_invite(std::vector<std::string> cmds)
{
    if (cmds.size() != 3)
    {
        message(SERVER_NAME, "461", user_tmp->second->getNickName(), "invite", user_tmp->first);
        return;
    }
    if (chan.find(cmds[2]) != chan.end() && chan.find(cmds[2])->second->getAccessUser()->find(user_tmp->first) != chan.find(cmds[2])->second->getAccessUser()->end())
    {
        if (chan.find(cmds[2])->second->getAccessUser()->find(user_tmp->first)->second.find_first_of("0o") != std::string::npos)
        {
            std::map<int, User*>::iterator it = user.begin();
            while (it != user.end())
            {
                if (it->second->getNickName() == cmds[1])
                {
                    chan.find(cmds[2])->second->getAccessUser()->find(it->first)->second += "i";
                    message(SERVER_NAME, "341", user_tmp->second->getNickName() + ' ' + cmds[1] + ' ' + cmds[2], "", user_tmp->first);
                    message("nick", "INVITE", user_tmp->second->getNickName() + ' ' + cmds[2], "", it->first);
                    break;
                }
                it++;
            }
        }
    }
}  

void    Server::ft_pong(std::vector<std::string> cmds)
{
    message(SERVER_NAME, "PONG", SERVER_NAME, cmds[1], user_tmp->first);
}