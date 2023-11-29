#ifndef USER_HPP
#define USER_HPP

#include "Server.hpp"
#include "Channel.hpp"

class Channel;

class User
{
    private:
        int                             fd;
        int                             pass;
        bool                            oper;
        std::string                     nickname;
        std::vector<std::string>        info_user;
        std::map<std::string, Channel*> chan;
    public:
        User(int fd):  fd(fd), pass(-1), oper(false), nickname("") {}
        ~User() {}

        int                             getFd() const {return fd;}
        int                             getPass()const {return(pass);}
        
        bool                            getOper() {return(oper);}
        
        void                            setOper() {oper = true;}
        void                            setPass(int value) {pass = value;}
        void                            setNickName(std::string nick) {nickname = nick;}
        
        std::string                     getNickName()const {return nickname;}
        std::string                     getInfo(size_t index)const {return(info_user[index]);}
        
        std::map<std::string, Channel*> *getChan() {return(&chan);}
        std::vector<std::string>        *getInfoUser(){return (&info_user);}
};

#endif