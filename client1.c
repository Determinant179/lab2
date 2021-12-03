#include <stdio.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
КЛИЕНТ 1
    Записать в разделяемую область памяти идентификаторы всех процессов,
    приоритет которых больше 25, а также время работы этих процессов.
    Распечатать ответ сервера.
*/

struct sembuf waiting = {0, -1, 0};
struct sembuf notify = {0, 2, 0};

int main()
{

    FILE *fp;
    if ((fp = popen("ps -eo pid,ni,time | awk '{if ( $2 > 10 ) print }' | tail +2", "r")) == NULL)
    {
        fprintf(stderr, "\nCLIENT 1:\nError while popen\n");
        return 3;
    }

    char output[2048];
    char line[2048];

    // Избавдяемся от мусора в начале строки (\0)
    strcpy(output, "");

    int i = 0;
    while (1)
    {
        if (fgets(line, 2048, fp) == NULL)
            break;
        i++;
        strcat(output, line);
    }

    printf("%s", output);

    // Получение семафоров
    int fd_sem = -1;
    while (fd_sem == -1)
    {
        fd_sem = semget(4, 0, 0);
        sleep(1);
    }

    semop(fd_sem, &waiting, 1);

    printf("\nCLIENT 1:\nWaiting message from server...\n");

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
        fprintf(stderr, "\nCLIENT 1:\nError while ShM adding\n");
    }

    // Ожидание сервера

    strcpy(addr, output);

    semop(fd_sem, &notify, 1);
    shmdt(addr);

    return 0;
}