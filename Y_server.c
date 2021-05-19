#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>

#define PORT 5000

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void *SendMessage(void *rd)
{
    int *sock = (int *)rd; // 중요

    while (1)
    {
        char message[255];
        scanf("%s", message);

        if (message[0] == '1')
        {
            char msg;
            msg = '1';

            write(*sock, &msg, sizeof(msg));
        }
        sleep(1);
    }
}

void *ReceiveMessage(void *rd)
{
    int *sock = (int *)rd;

    while (1)
    {
        char msg[255];
        int str_len = read(*(sock), msg, sizeof(char) * 255);
        if (msg[0] == '2')
        {
            printf("숫자 : %d 을 잘받았습니다.\n", msg[0]);
        }
        sleep(1);
    }
}

int main(int argc, char **argv)
{
    int listenSock;
    char message[100];
    struct sockaddr_in listenSocket;
    struct sockaddr_in connectSocket;
    socklen_t connectSocketsize;
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == -1)
    {
        printf("create socket error.\n");
        exit(0);
    }
    memset(&listenSocket, 0, sizeof(listenSocket));
    listenSocket.sin_family = AF_INET;
    listenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    listenSocket.sin_port = htons(PORT);
    pthread_t p_thread[2];
    if (bind(listenSock, (struct sockaddr *)&listenSocket, sizeof(listenSocket)) == -1)
    {
        printf("bind fail...\n");
        exit(0);
    }

    if (listen(listenSock, 5) == -1)
    {
        printf("Listen fail.\n");
        exit(0);
    }
    printf("Waiting for clients...\n");
    //connectSocketsize = sizeof(connectSocket);
    listenSock = accept(listenSock, (struct sockaddr *)&connectSocket, &connectSocketsize);

    if (listenSock < 0)
    {
        printf("Server: accept failed\n");
        exit(0);
    }

    pthread_create(&p_thread[0], NULL, SendMessage, (void *)&listenSock);
    pthread_create(&p_thread[1], NULL, ReceiveMessage, (void *)&listenSock);
    pthread_join(p_thread[0], NULL);
    pthread_join(p_thread[1], NULL);
    close(listenSock);
    return 0;
}