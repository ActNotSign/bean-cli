#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#include <string>
#define BUFFER_SIZE 1024
using namespace std;

char sendbuf[BUFFER_SIZE];
char recvbuf[BUFFER_SIZE];
char * fix = "\r\n";

void clear ()
{
     memset(sendbuf, 0, sizeof(sendbuf));
     memset(recvbuf, 0, sizeof(recvbuf));
}

void split( char **arr, char *str, const char *del)
{
     char *s = NULL;
     s = strtok(str,del);
     while(s != NULL) {
        *arr++ = s;
        s = strtok(NULL,del);
     }
}

void delete_tube(int sock_cli, char * tube, int count)
{
    char * reserve = "reserve\r\n";
    char * use = "watch %s\r\n";
    //use tube
    char send_use[BUFFER_SIZE];
    sprintf(send_use, use, tube);
    send(sock_cli, send_use, strlen(send_use), 0);
    recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
    std::cout<< recvbuf <<std::endl;
    clear();

    char *delim = " ";
    char *p;
    char *arr[4];
    char send_delete[BUFFER_SIZE];
    for (int i = 0 ; i < count ; i++)
    {
        send(sock_cli, reserve, strlen(reserve), 0);
        recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
        std::cout<<recvbuf<<std::endl;
        //delete
        split(arr, recvbuf, delim);
        sprintf(send_delete, "delete %s\r\n", *(arr+1));
        clear();
        send(sock_cli, send_delete, strlen(send_delete), 0);
        recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
        std::cout<<recvbuf<<" "<< arr[1] <<std::endl;
        clear();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
         std::cout<<"params : bean-cli <ip> <port>"<<std::endl;
         exit(1);
    }
     int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
     struct sockaddr_in servaddr;
     memset(&servaddr, 0, sizeof(servaddr));
     servaddr.sin_family = AF_INET;
     servaddr.sin_port = htons (atoi(argv[2]));
     servaddr.sin_addr.s_addr = inet_addr(argv[1]);
     if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
     {
         std::cout<<"Connect to the server failed!"<<std::endl;
         exit(1);
     }
     clear();
     std::cout<< "***************Connected*************" <<std::endl;
     char *p_command[2];
     std::cout<<">> ";
     while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
     {

        sendbuf[strlen(sendbuf) - 1] = 0;
        //add \r\n and send command
         strcat(sendbuf, fix);
         send(sock_cli, sendbuf, strlen(sendbuf), 0);
         if(strcmp(sendbuf,"exit\n")==0)
             break;
         //echo back string
         recv(sock_cli, recvbuf, sizeof(recvbuf), 0);
         std::cout<<recvbuf<<std::endl;
         //init memory
        split(p_command, sendbuf, " ");
        if (strstr(*(p_command), "delete-tube") != NULL) {
             delete_tube(sock_cli, *(p_command+1), 1000000);
             continue;
        }
        clear();
        std::cout<<">> ";
     }
     close(sock_cli);
     return 0;
}