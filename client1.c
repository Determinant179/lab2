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

// struct sembuf waiting = {0, -1, 0};
// struct sembuf notify = {0, 2, 0};

struct sembuf
    sem_unlock[] = {{0, -1, 0}, {0, -3, 0}},
    sem_wait[] = {{0, 2, 0}, {0, 4, 0}};

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

    // printf("%s", output);

    // Получение семафоров
    int fd_sem = -1;
    while (fd_sem == -1)
    {
        fd_sem = semget(4, 0, 0);
        sleep(1);
    }

    printf("sem -> -1\n");
    semop(fd_sem, &sem_unlock[0], 1);

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
        fprintf(stderr, "\nC<LIENT 1>\nError while ShM adding\n");
    }

    // Ожидание сервера

    strcpy(addr, output);

    printf("sem -> 2\n");
    semop(fd_sem, &sem_wait[0], 1);
    shmdt(addr);

    // --------------------------------------------------------

    // Получение ответа от сервера

    printf("\n<CLIENT 1>\nWaiting message from server...\n");

    printf("sem -> -3\n");
    semop(fd_sem, &sem_unlock[1], 1);

    // Добавление РОП
    char *addr2 = shmat(fd_shm, 0, 0);
    if (addr2 == (char *)-1)
    {
        fprintf(stderr, "\n<CLIENT 1>\nError while ShM adding\n");
    }

    char answer[2048];
    strcpy(answer, addr2);
    printf("\n<CLIENT 1>\nMessage from server:\n%s\n", answer);

    shmdt(addr2);

    printf("sem -> 4\n");
    semop(fd_sem, &sem_wait[1], 1);

    shmctl(fd_shm, IPC_RMID, 0);
    semctl(fd_sem, 0, IPC_RMID);

    return 0;
}