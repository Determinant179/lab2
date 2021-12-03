#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/*
КЛИЕНТ 2
    Записать в разделяемую область памяти время,
    когда какой-либо процесс последний раз выполнял операцию над
    семафорами набора.
*/

struct sembuf waiting = {0, -3, 0};
struct sembuf notify = {0, 4, 0};

int main()
{

    // Получение семафоров
    int fd_sem = -1;
    while (fd_sem == -1)
    {
        fd_sem = semget(4, 0, 0);
        sleep(1);
    }
    semop(fd_sem, &waiting, 1);

    // Получение РОП
    int fd_shm = -1;
    while (fd_shm == -1)
    {
        fd_shm = shmget(3, 0, 0);
        sleep(1);
    }

    // Добавление РОП
    char *addr = shmat(fd_shm, 0, 0);
    if (addr == (char *)-1)
    {
        fprintf(stderr, "shared memory adding ERROR\n");
    }

    // Ожидание сервера

    struct semid_ds semid_ds;
    int rtrn, semid;

    union semun
    {
        int val;
        struct semid_ds *buf;
        short array[25];

    } arg;

    // Инициализация указателя на структуру данных
    arg.buf = &semid_ds;

    rtrn = semctl(fd_sem, 0, GETPID);
    char output[2048];
    strcpy(output, "Last operation was performed at ");

    time(&arg.buf->sem_otime);
    strcat(output, ctime(&arg.buf->sem_otime));


    strcpy(addr, output);

    semop(fd_sem, &notify, 1);
    shmdt(addr);

    return 0;
}