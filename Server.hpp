#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <signal.h>

#define BUFFERSIZE 1024
#define SERVER_NAME "jeuneuse"
#define VERSION "V1"
#define USER_MODE "ao"
#define CHANNEL_MODE "mtov"
#define PASS_OP "1417"
#include <map>

#include "User.hpp"
#include "Channel.hpp"

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

extern int flag;

class Channel;

class User;

class Server
{
private:
    int                                 serverEndPoint;
    int                                 port;
    int                                 nbytes;

    char                                buffer[BUFFERSIZE];
    
    std::string                         password;
    
    std::map<std::string, Channel *>    chan; 
    std::map<int, User *>               user;
    std::map<int, User*> ::iterator     user_tmp;
    
    socklen_t                           sin_size;

    fd_set                              current_sockets;
    fd_set                              ready_sockets;
public:
    Server(std::string av1, std::string av2);
    ~Server() {}

    void        runningServer();
    void        findCmd(std::vector<std::string> cmds);
    void        sepCmd();
    
    void        ft_names(std::vector<std::string> cmds);
    void        ft_cap(std::vector<std::string> cmds);
    void        ft_nick(std::vector<std::string> cmds);
    void        ft_join_bis(std::vector<std::string> cmds);
    void        ft_join(std::vector<std::string> cmds);
    void        ft_part(std::vector<std::string> cmds);
    void        ft_list(std::vector<std::string> cmds);
    void        ft_kick(std::vector<std::string> cmds);
    void        ft_motd();
    void        ft_channel_mode(std::vector<std::string> cmds);
    void        ft_mode(std::vector<std::string> cmds);
    void        ft_notice(std::vector<std::string> cmds);
    void        ft_user(std::vector<std::string> cmds);
    void        ft_pass(std::vector<std::string> cmds);
    void        ft_topic(std::vector<std::string> cmds);
    void        ft_privmsg(std::vector<std::string>cmds);
    void        ft_quit(std::vector<std::string> msg);
    void        ft_oper(std::vector<std::string> cmds);
    void        ft_invite(std::vector<std::string> cmds);
    void        ft_pong(std::vector<std::string> cmds);

    void        message(std::string server, std::string code_rep, std::string nick, std::string msg, int fd);
    void        messageToChannel(std::string server, std::string code_rep, std::string nick, std::string msg, std::string channel);
    
    int         checkInfoUser();
    void        exit_clean();
    
    std::string getUser(int fd);
};

#endif