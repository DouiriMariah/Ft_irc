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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <vector>
#include <iostream>

#include "Server.hpp"

#include <signal.h>
#include <sys/wait.h>


int flag;

void cleanup(int sig) {
    std::cout << "Cleanup function called with signal " << sig << std::endl;
    flag = 1;
    // exit(0);
}

int main(int ac, char **av)
{
    if(ac != 3)
        return(std::cout << "Error: 2 arguments are required\n", 1);
    std::string test(av[1]);
    if (test.find_first_not_of("0123456789") !=  std::string::npos || atoi(av[1]) <= 0)
        return (std::cout << "Error: first argument should be a valid port number\n", 1);
    std::string pass(av[2]);
    if (pass.size() < 4)
        return (std::cout << "Error: the password must be composed of at least 4 characters\n", 1);

    Server serv(av[1], av[2]);

    flag = 2;
    signal(SIGINT, cleanup);
    serv.runningServer();
    return(0);
}