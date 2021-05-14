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

typedef struct
{
    int *connectSock;
    char sc;
} _senddata;

typedef struct
{
    int *connectSock;
    char *message;
} Recieve_Data;

typedef struct recieve_data
{
    char *message;
    int *ClientSock;
} recieve_data;

void *C_sendMessage(void *senddata)
{
    _senddata *data = (_senddata *)senddata;
    while (1)
    {
        char message[30];
        printf("\nclient -> : ");
        scanf("%s", message);
        write(*data->connectSock, message, sizeof(message));
    }
}

void *C_recieve(void *rd)
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

void *S_sendMessage(void *senddata)
{
    printf("test");
    _senddata *data = (_senddata *)senddata; // 중요

    while (1)
    {
        char message[30];
        printf("\nServer -> : ");
        scanf("%s", message);

        if (message[0] == '1')
        {
            printf("sendmessage return\n");
            int *a;

            return a;
        }
        write(*data->connectSock, message, sizeof(message));
    }
}

void *S_recieve(void *rd)
{
    Recieve_Data *data = (Recieve_Data *)rd;
    while (1)
    {
        int str_len = read(*(data->connectSock), data->message, sizeof(char) * 30);
        if (str_len != -1)
        {
            printf("\n <- client : %s\n", data->message);
        }
    }
}

int main(int argc, char **argv)
{

    int listenSock;
    int connectSock;
    char message[BUFF_SIZE];
    struct sockaddr_in listenSocket;
    struct sockaddr_in connectSocket;
    socklen_t connectSocketsize;
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == -1)
    {
        printf("create socket error.\n");
        return 1;
    }
    memset(&listenSocket, 0, sizeof(listenSocket));
    listenSocket.sin_family = AF_INET;
    listenSocket.sin_addr.s_addr = htonl(INADDR_ANY);
    listenSocket.sin_port = htons(PORT);
    // ssize_t receivedBytes;
    // char readBuff[BUFFER_SIZE];
    // char sendBuff[BUFFER_SIZE];
    Recieve_Data rd;
    _senddata senddata;
    if (bind(listenSock, (struct sockaddr *)&listenSocket, sizeof(listenSocket)) == -1)
    {
        printf("Start clients...\n");

        memset(&connectSocket, 0, sizeof(connectSocket));

        connectSocket.sin_family = AF_INET;
        inet_aton("127.0.0.1", (struct in_addr *)&connectSocket.sin_addr.s_addr);
        connectSocket.sin_port = htons(PORT);
        //int ClientSock = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(listenSock, (struct sockaddr *)&connectSocket, sizeof(connectSocket)) < 0)
        {
            printf("Can not connect.\n");
            exit(0);
        }
        recieve_data rd2;
        rd2.ClientSock = &listenSock;
        rd2.message = message;
        senddata.sc = 'c';
        senddata.connectSock = &listenSock;
        pthread_t p_thread[2];
        int t;
        int status;

        pthread_create(&p_thread[0], NULL, C_sendMessage, (void *)&senddata);
        pthread_create(&p_thread[1], NULL, C_recieve, (void *)&rd2);

        pthread_join(p_thread[0], (void **)&status);
        pthread_join(p_thread[1], (void **)&status);

        close(listenSock);
    }
    else
    {

        if (listen(listenSock, LISTEN_QUEUE_SIZE) == -1)
        {
            printf("Listen fail.\n");
            exit(0);
        }

        printf("Waiting for clients...\n");

        connectSocketsize = sizeof(connectSocket);

        connectSock = accept(listenSock, (struct sockaddr *)&connectSocket, &connectSocketsize);

        if (connectSock < 0)
        {
            printf("Server: accept failed\n");
            exit(0);
        }
        rd.connectSock = &connectSock;
        senddata.sc = 's';
        senddata.connectSock = &connectSock;
        pthread_t p_thread[2];

        int status;

        pthread_create(&p_thread[0], NULL, S_sendMessage, (void *)&senddata);
        pthread_create(&p_thread[1], NULL, S_recieve, (void *)&rd);

        pthread_join(p_thread[0], (void **)&status);
        pthread_join(p_thread[1], (void **)&status);

        close(connectSock);
    }

    // close(listenFD);

    // ssize_t receivedBytes;

    // // read할 값이 있다면 계속 읽어들인다.
    // while((receivedBytes = read(connectFD, readBuff, BUFF_SIZE)) > 0)
    // {

    //     printf("%lu bytes read\n", receivedBytes);
    //     readBuff[receivedBytes] = '\0';
    //     fputs(readBuff, stdout);
    //     fflush(stdout);

    //     sprintf(sendBuff,"Server %s",readBuff);
    //     write(connectFD, sendBuff, strlen(sendBuff));

    // }

    // // 클라이언트가 종료되면 해당 자식 프로세스의 커넥팅 소켓도 종료
    // close(connectFD);
    // exit(0);

    // }

    // // 부모 서버라면 커넥팅 소켓을 닫아준다.
    // else
    //     close(connectFD);
    // close(listenFD);
    return 0;
}