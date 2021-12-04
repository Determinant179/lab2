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

struct sembuf sem_unlock = {0, -5, 0};
struct sembuf sem_wait = {0, 6, 0};

int main()
{
    int sem_num;

    // Получение семафоров
    int fd_sem = -1;
    while (fd_sem == -1)
    {
        fd_sem = semget(4, 0, 0);
        sleep(1);
    }
    printf("sem -> -5\n");
    semop(fd_sem, &sem_unlock, 1);
    sem_num = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", sem_num);

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
        fprintf(stderr, "\n<CLIENT 2>\nError while shared memory adding\n");
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

    printf("sem -> \n");
    semop(fd_sem, &sem_wait, 1);
    sem_num = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", sem_num);
    shmdt(addr);

    return 0;
}