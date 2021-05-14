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

#define PORT 9999

//int *semaphoreStoL;
int semid;
int *shared_memory;

void s_wait()
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1)
    {
        printf("semop() fail\n");
        exit(0);
    }
}

void s_quit()
{
    struct sembuf buf;
    buf.sem_num = 0;
    buf.sem_op = 1;
    buf.sem_flg = SEM_UNDO;

    if (semop(semid, &buf, 1) == -1)
    {
        printf("semop() fail\n");
        exit(0);
    }
}

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void sem_init()
{
    union semun su;

    semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0660);

    if (semid == -1)
    {
        printf("semet() fail \n");
        exit(0);
    }

    su.val = 1;
    if (semctl(semid, 0, SETVAL, su) == -1)
    {
        printf("semctl fail\n");
        exit(0);
    }
    // if (semctl(semid, 0, IPC_RMID, su) == -1)
    // {
    //     printf("semctl() fail\n");
    //     exit(0);
    // }
}

void shm_init()
{
    int shmid;
    shmid = shmget((key_t)3866, sizeof(30), 0666 | IPC_CREAT);
    if (shmid == -1)
    {
        perror("shmget failed : ");
        exit(0);
    }
    // 공유메모리를 사용하기 위해 프로세스메모리에 붙인다.
    shared_memory = shmat(shmid, (void *)0, 0);

    if (shared_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(0);
    }

    // if (shmctl(shmid, IPC_RMID, NULL) == -1)
    // {
    //     printf("shmctl failed\n");
    // }
    // shmid = shmget((key_t)3835, sizeof(30), 0666 | IPC_CREAT);
    // if (shmid == -1)
    // {
    //     perror("shmget failed : ");
    //     exit(0);
    // }
    // // 공유메모리를 사용하기 위해 프로세스메모리에 붙인다.
    // semid = shmat(shmid, (void *)0, 0);

    // if (semid == (void *)-1)
    // {
    //     perror("shmat failed : ");
    //     exit(0);
    // }
}

void *addshm()
{
    for (int i = 0; i < 100000; i++)
    {
        s_wait();
        (*shared_memory)++;
        s_quit();
    }
}

void *SendMessage(void *rd)
{
    int *sock = (int *)rd; // 중요

    while (1)
    {
        char message[255];
        scanf("%s", message);
        //shared_memory = 0;

        if (message[0] == '1')
        {
            sem_init();
            typedef struct
            {
                char msg;
                int semaphore;
            } _stSema;
            _stSema st;
            st.msg = '1';
            //semaphoreStoL = &semid;
            st.semaphore = semid;

            printf("send Semaphore : %d sizeof(_stSema) : %lud\n", st.semaphore, sizeof(_stSema));
            write(*sock, &st, sizeof(st));
        }
        if (message[0] == '2')
        {
            //*shared_memory = 0;
            pthread_t p_thread;
            pthread_create(&p_thread, NULL, addshm, NULL);
            write(*sock, &message[0], sizeof(message));

            pthread_join(p_thread, NULL);
        }
        sleep(1);
        printf("Send : %d\n", *shared_memory);
    }
}

void *ReceiveMessage(void *rd)
{
    int *sock = (int *)rd;

    while (1)
    {
        char msg[255];
        int str_len = read(*(sock), msg, sizeof(char) * 255);
        if (msg[0] == '1')
        {
            memcpy(&semid, &msg[4], sizeof(semid));
            //semid = *semaphoreStoL;
            printf("Receive Semaphore : %d\n", semid);
        }
        if (msg[0] == '2')
        {
            for (int i = 0; i < 100000; i++)
            {
                s_wait();
                (*shared_memory)++;
                s_quit();
            }
        }
        sleep(1);
        printf("Receive : %d\n", *shared_memory);
    }
}

int main(int argc, char **argv)
{
    shm_init();
    //sem_init();
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
        printf("C %d\n", semid);
        printf("C %d\n", *shared_memory);
        printf("C %p\n", shared_memory);
        printf("Start clients...\n");
        memset(&connectSocket, 0, sizeof(connectSocket));
        connectSocket.sin_family = AF_INET;
        inet_aton("127.0.0.1", (struct in_addr *)&connectSocket.sin_addr.s_addr);
        connectSocket.sin_port = htons(PORT);
        if (connect(listenSock, (struct sockaddr *)&connectSocket, sizeof(connectSocket)) < 0)
        {
            printf("Can not connect.\n");
            exit(0);
        }
    }
    else
    {
        // sem_init();
        printf("S %d\n", semid);
        printf("S %d\n", *shared_memory);
        printf("S %p\n", shared_memory);
        if (listen(listenSock, 5) == -1)
        {
            printf("Listen fail.\n");
            exit(0);
        }
        printf("Waiting for clients...\n");
        connectSocketsize = sizeof(connectSocket);
        listenSock = accept(listenSock, (struct sockaddr *)&connectSocket, &connectSocketsize);
        // char msg[5];
        // msg[0] = 2;
        // memcpy(&msg[1], semid, 4);
        // write(listenSock, semid, 5);

        if (listenSock < 0)
        {
            printf("Server: accept failed\n");
            exit(0);
        }
    }
    pthread_create(&p_thread[0], NULL, SendMessage, (void *)&listenSock);
    pthread_create(&p_thread[1], NULL, ReceiveMessage, (void *)&listenSock);
    pthread_join(p_thread[0], NULL);
    pthread_join(p_thread[1], NULL);
    close(listenSock);
    return 0;
}