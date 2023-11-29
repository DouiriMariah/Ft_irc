#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "User.hpp"

class User;

class Channel
{
    private:
        std::map<int, std::string>  access_user;
        std::string                 name_chan;
        std::string                 topic;
        std::string                 mode;
    public:
        Channel(std::string name): name_chan(name), topic(""), mode("") {}
        ~Channel() {}

        std::map<int, std::string> *getAccessUser() {return(&access_user);}

        std::string getName() {return(name_chan);}
        std::string getTopic() {return(topic);}
        std::string getMode() {return(mode);}
        
        void    joiner(int name, std::string rights) {access_user.insert(std::pair<int, std::string>(name, rights));}
        void    setTopic(std::string topico) {topic = topico;}
        void    setMode(std::string new_mode) {mode = new_mode;}
        void    setAccessUser(int fd, std::string new_acces) {getAccessUser()->find(fd)->second = new_acces;}
};

#endif