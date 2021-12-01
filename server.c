#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>
/*
 * Вариант №20
 * Сервер.
 *  Создать разделяемую область памяти (РОП) и набор семафоров (НС).
 *      От поступивших в РОП от клиентов сообщений, сформировать информацию о количестве всех файлов,
 *          содержащихся в подкаталогах текущего каталога и вывести её на печать,
 *          а также вывести значение идентификатора.
 *  Удалить РОП и НС.
 */
#define SHM_KEY 3
#define SEM_KEY 4

struct sembuf unclock_first_client = {0, 1, 0};
struct sembuf wait_first_client = {0, -2, 0};
struct sembuf unclock_second_client = {0, 3, 0};
struct sembuf wait_second_client = {0, -4, 0};

int main()
{

    // Создать разделяемую область памяти (РОП)
    int fd_shm = shmget(SHM_KEY, 2048, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_shm == -1)
    {
        fprintf(stderr, "server - error - shared memory creation\n");
        return 1;
    }
    printf("server - create rop - ok\n");

    // Создать набор семафоров (НС)
    int fd_sem = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_sem == -1)
    {
        fprintf(stderr, "server - error - sem creation\n");
        return 2;
    }
    printf("server - create ns - ok\n");


    shmctl(fd_shm, IPC_RMID, 0);
    semctl(fd_sem, 0, IPC_RMID);

    return 0;
}