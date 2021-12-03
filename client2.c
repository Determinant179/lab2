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

struct sembuf waiting = {0, -5, 0};
struct sembuf notify = {0, 6, 0};

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
        fprintf(stderr, "\nCLIENT 2:\nError while shared memory adding\n");
    }

    struct semid_ds semid_ds;
    struct semid_ds *buf;

    int rtrn;

    // Инициализация указателя на структуру данных
    buf = &semid_ds;

    rtrn = semctl(fd_sem, 0, GETPID);
    char output[2048];
    strcpy(output, "Last operation was performed at ");

    time(&buf->sem_otime);
    strcat(output, ctime(&buf->sem_otime));

    strcpy(addr, output);

    semop(fd_sem, &notify, 1);
    shmdt(addr);

    return 0;
}