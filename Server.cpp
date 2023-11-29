#include <string>
#include "Server.hpp"
#include <signal.h>

Server::Server(std::string av1, std::string av2) : port(atoi(av1.c_str())), password(av2) 
{
    int yes = 1;
    serverEndPoint = socket(AF_INET, SOCK_STREAM, 0);
	if (serverEndPoint < 0)
	{
		std::cout << "Can't initiate socket." << std::endl;
		exit(1);
	}

	setsockopt(serverEndPoint, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    SA_IN server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(serverEndPoint, (SA*)&server_addr, sizeof(server_addr));
    listen(serverEndPoint, 128);

    FD_ZERO(&current_sockets);
    FD_SET(serverEndPoint, &current_sockets);
}

void    Server::exit_clean()
{
    if(flag != 1)
        return;
    std::map<std::string, Channel*> ::iterator off_chan = chan.begin();
    user_tmp = user.begin();
    while(off_chan != chan.end())
    {
        delete(off_chan->second);
        off_chan++;
    }
    while(user_tmp != user.end())
    {
        FD_CLR(user_tmp->first, &ready_sockets);
        FD_CLR(user_tmp->first, &current_sockets);
        delete(user_tmp->second);
        user_tmp++;
    }
    chan.clear();
    user.clear();
    exit(1);
}

void    Server::runningServer()
{
    int fd;
    if(flag == 2)
        std::cout << "Welcome \n";
    exit_clean();
    while(true)
    {
        ready_sockets = current_sockets;
        select(1024, &ready_sockets, NULL, NULL, NULL);
        exit_clean();
        for(int i = 0; i < 1024; i++)
        {
            if(FD_ISSET(i, &ready_sockets))
            {
                if(i == serverEndPoint)
                {
                    sin_size = sizeof(SA_IN);
                    SA_IN client_addr;
                    fd = accept(serverEndPoint, (struct sockaddr *) &client_addr, &sin_size);
                    FD_SET(fd, &current_sockets);
                    User *new_one = new User(fd);
                    user.insert(std::pair<int, User *>(fd, new_one));
                }
                else
                {
                    nbytes = recv(i, buffer, BUFFERSIZE, MSG_DONTWAIT);
                    if(!nbytes)
                    {

                        strcpy(buffer, "QUIT :leaving");
                        sepCmd();
                        FD_CLR(i, &current_sockets);
                    }
                    else
                    {
                        write(1, &buffer, nbytes);
                        user_tmp = user.find(i);
                        sepCmd();
                    }
                    bzero(buffer, nbytes);
                }
            }
        }
    }
}

void    Server::findCmd(std::vector<std::string> cmds)
{
    if(!cmds[0].size())
        return;
    if (cmds[0] == "CAP")
        ft_cap(cmds);
    else if (cmds[0] == "NICK")
        ft_nick(cmds);
    else if (cmds[0] == "JOIN")
        ft_join(cmds);
    else if (cmds[0] == "PART")
        ft_part(cmds);
    else if (cmds[0] == "LIST")
        ft_list(cmds);
    else if (cmds[0] == "KICK")
        ft_kick(cmds);
    else if (cmds[0] == "MOTD")
        ft_motd();
    else if (cmds[0] == "MODE")
        ft_mode(cmds);
    else if (cmds[0] == "NOTICE")
        ft_notice(cmds);
    else if (cmds[0] == "USER")
        ft_user(cmds);
    else if (cmds[0] == "PASS")
        ft_pass(cmds);
    else if (cmds[0] == "TOPIC")
        ft_topic(cmds);
    else if (cmds[0] == "PRIVMSG")
        ft_privmsg(cmds);
    else if (cmds[0] == "QUIT")
        ft_quit(cmds);
    else if (cmds[0] == "OPER")
        ft_oper(cmds);
    else if (cmds[0] == "INVITE")
        ft_invite(cmds);
    else if (cmds[0] == "PING")
        ft_pong(cmds);
    else if (cmds[0] == "NAMES")
        ft_names(cmds);
}

void     Server::sepCmd()
{
    std::vector<std::string> cmd;
    std::stringstream ss(buffer);
    std::string test;

    while(getline(ss, test, '\n'))
        (cmd).push_back(test);

    size_t i = -1;
    while (++i < cmd.size())
    {
        std::vector<std::string> sepcmd;
        std::stringstream cc(cmd[i]);
        while(cc)
        {
            std::string tmp;
            cc >> tmp;
            if (tmp[0] == ':')
                tmp.erase(tmp.find(':'), 1);
            if (tmp == "\n" || tmp == "\0")
                break;
            sepcmd.push_back(tmp);
        }
        if(sepcmd.size())
            findCmd(sepcmd);
    }
}

void    Server::message(std::string server, std::string code_rep, std::string nick, std::string msg, int fd)
{
    std::string final_msg;
    if(server == "nick")
    {
        server = user_tmp->second->getNickName() + "!" + 
            user_tmp->second->getInfo(0) + "@" + user_tmp->second->getInfo(2);  
    }
    final_msg = ":" + server + " " + code_rep + " " + nick + " :" + msg + "\r\n";
    send(fd, final_msg.c_str(), final_msg.size(), 0);
}

void    Server::messageToChannel(std::string server, std::string code_rep, std::string nick, std::string msg, std::string channel)
{
    std::map<std::string, Channel *>::iterator ch = chan.find(channel);
    std::map<int, std::string>::iterator tt = ch->second->getAccessUser()->begin();
    while(tt != ch->second->getAccessUser()->end())
    {
        message(server, code_rep, nick, msg, tt->first);
        tt++;
    }
}

std::string Server::getUser(int fd)
{
    std::map<int, User *>::iterator it = user.begin();
    std::string ret;
    ret = "No User";
    while (it != user.end())
    {
        if (it->first == fd)
        {
           ret = it->second->getNickName();
           break;
        }
        it++;
    }
    return ret;
}
