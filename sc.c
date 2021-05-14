#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define PORT 9998
#define BUFFER_SIZE 4096
#define BUFF_SIZE 100
#define LISTEN_QUEUE_SIZE 5

typedef struct recieve_data
{
    char *message;
    int *ClientSock;
} recieve_data;

void *sendMessage(void *ClientSock)
{
    int *cs = (int *)ClientSock;
    while (1)
    {
        char message[30];
        printf("\nclient -> : ");
        scanf("%s", message);
        write(*cs, message, sizeof(message));
    }
}

void *recieve(void *rd)
{

    recieve_data *data = (recieve_data *)rd;
    char *msg = data->message;
    while (1)
    {
        int str_len = read(*(data->ClientSock), msg, sizeof(char) * 30);
        if (str_len != -1)
        {
            printf("\n <- server : %s\n", msg);
        }
    }
}

int main(int argc, char **argv)
{
    // if (argc != 2)
    // {
    //     printf("Usage: %s IPv4-address\n", argv[0]);
    //     exit(1);
    // }
    struct sockaddr_in connectSocket;
    memset(&connectSocket, 0, sizeof(connectSocket));

    connectSocket.sin_family = AF_INET;
    inet_aton("127.0.0.1", (struct in_addr *)&connectSocket.sin_addr.s_addr);
    connectSocket.sin_port = htons(PORT);

    int ClientSock = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(ClientSock, (struct sockaddr *)&connectSocket, sizeof(connectSocket)) < 0)
    {
        printf("Can not connect.\n");
        exit(0);
    }

    int readBytes, writtenBytes;
    char message[BUFF_SIZE];

    recieve_data rd;
    rd.ClientSock = &ClientSock;
    rd.message = message;

    pthread_t p_thread[2];
    int t;
    int status;

    pthread_create(&p_thread[0], NULL, sendMessage, (void *)&ClientSock);
    pthread_create(&p_thread[1], NULL, recieve, (void *)&rd);

    pthread_join(p_thread[0], (void **)&status);
    pthread_join(p_thread[1], (void **)&status);

    close(ClientSock);
    return 0;
}