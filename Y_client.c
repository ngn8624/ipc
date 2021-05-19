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

void *ReceiveMessage(void *rd)
{
    int *sock = (int *)rd;

    while (1)
    {
        char msg[255];
        int str_len = read(*(sock), msg, sizeof(char) * 255);
        if (msg[0] == '1')
        {
            char message = '2';
            write(*sock, &message, sizeof(message));
            printf("2번을 보냈습니다.\n");
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
    pthread_t p_thread[1];

    printf("Start clients...\n");
    memset(&connectSocket, 0, sizeof(connectSocket));
    connectSocket.sin_family = AF_INET;
    inet_aton("192.168.0.11", (struct in_addr *)&connectSocket.sin_addr.s_addr);
    connectSocket.sin_port = htons(PORT);

    if (connect(listenSock, (struct sockaddr *)&connectSocket, sizeof(connectSocket)) < 0)
    {
        printf("Can not connect.\n");
        exit(0);
    }
    pthread_create(&p_thread[0], NULL, ReceiveMessage, (void *)&listenSock);
    pthread_join(p_thread[0], NULL);
    close(listenSock);
    return 0;
}