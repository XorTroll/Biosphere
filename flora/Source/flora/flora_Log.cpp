
#include <flora_Log.hpp>
#include <switch.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <malloc.h>
#include <cstdio>

namespace flora
{
    static int sockfd = -1;
    static int fd = -1;

    static u16 flora_fauna_port = 11762;

    bool InitializeLogging()
    {
        if(fd != -1) return true;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            socketExit();
            return false;
        }
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = ntohs(flora_fauna_port);
        server.sin_addr.s_addr = INADDR_ANY;
        bzero(server.sin_zero, 8);
        size_t slen = sizeof(struct sockaddr_in);
        int res = bind(sockfd, (struct sockaddr*)&server, slen);
        if(res == -1)
        {
            close(sockfd);
            socketExit();
            return false;
        }
        res = listen(sockfd, 5);
        if(res == -1)
        {
            close(sockfd);
            socketExit();
            return false;
        }
        struct sockaddr_in client;
        res = accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&slen);
        if(res == -1)
        {
            close(sockfd);
            socketExit();
            return false;
        }
        fd = res;
        timeval send_timeout;
        send_timeout.tv_sec = 5;
        send_timeout.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&send_timeout, sizeof(send_timeout));
        return true;
    }

    void FinalizeLogging()
    {
        close(fd);
        close(sockfd);
        socketExit();
    }

    bool SendRaw(void *val, size_t size)
    {
        int res = send(fd, val, size, 0);
        return (res != -1);
    }
}