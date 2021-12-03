#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <string.h>

/*
СЕРВЕР 
    Создать разделяемую область памяти (РОП) и набор семафоров(НС). 
    Ждать сообщений клиентов. При поступлении сообщения от 1-ого клиента, 
    обработать его и переслать абоненту имена «родителей» этих процессов. 
    Распечатать показания времени, а также содержимое сообщения, 
    полученное от 2-ого клиента. Удалить РОП и НС.

КЛИЕНТ 1
    Записать в разделяемую область памяти идентификаторы всех процессов, 
    приоритет которых больше 25, а также время работы этих процессов. 
    Распечатать ответ сервера.

КЛИЕНТ 2 
    Записать в разделяемую область памяти время, 
    когда какой-либо процесс последний раз выполнял операцию над 
    семафорами набора. 
*/

struct sembuf unclock_first_client = {0, 1, 0};
struct sembuf wait_first_client = {0, -2, 0};
struct sembuf unclock_second_client = {0, 3, 0};
struct sembuf wait_second_client = {0, -4, 0};

int main()
{

    // Создание РОП
    int fd_shm = shmget(3, 2048, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_shm == -1)
    {
        fprintf(stderr, "\nSERVER:\nError while ShM creation\n");
        return 1;
    }
    printf("\nSERVER:\nSnM was created\n");

    // Создание семафоров
    int fd_sem = semget(4, 1, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_sem == -1)
    {
        fprintf(stderr, "\nSERVER:\nError while semaphore set creation\n");
        return 2;
    }
    printf("\nSERVER:\nSemaphore set was created\n");

    // Добавление РОП
    char *addr = shmat(fd_shm, 0, 0);
    if (addr == (char *)-1)
    {
        fprintf(stderr, "\nSERVER:\nError while  ShM adding\n");
    }

    // Разблокирование 1-ого клиента
    semop(fd_sem, &unclock_first_client, 1);
    // Ожидание 1-ого клиента
    semop(fd_sem, &wait_first_client, 1);

    char answer1[2048];
    strcpy(answer1, addr);
    printf("\nSERVER:\n Message from client 1: \"%s\"\n", answer1);

    // Разблокирование 2-ого клиента
    semop(fd_sem, &unclock_second_client, 1);
    // Ожидание 2-ого клиента
    semop(fd_sem, &wait_second_client, 1);

    char answer2[2048];
    strcpy(answer2, addr);
    printf("\nSERVER:\n Message from client 2: \"%s\"\n", answer2);

    shmdt(addr);
    shmctl(fd_shm, IPC_RMID, 0);
    semctl(fd_sem, 0, IPC_RMID);

    return 0;
}