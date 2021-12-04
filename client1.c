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

struct sembuf
    sem_unlock[] = {{0, -1, 0}, {0, -3, 0}},
    sem_wait[] = {{0, 2, 0}, {0, 4, 0}};

int main()
{

    int sem_val;

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

    // Получение семафоров
    int fd_sem = -1;
    while (fd_sem == -1)
    {
        fd_sem = semget(4, 0, 0);
        sleep(1);
    }

    semop(fd_sem, &sem_unlock[0], 1);
    sem_val = semctl(fd_sem, 0, GETVAL, 0);
    printf("\n<CLIENT 1>\nSem val = %d {Lock client 1, wait server...}\n", sem_val);

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
        fprintf(stderr, "\nC<LIENT 1>\nError while shared mwmory adding\n");
    }

    // Ожидание сервера

    printf("\n<CLIENT 1>\nCopy message to shared memory\n");
    strcpy(addr, output);

    semop(fd_sem, &sem_wait[0], 1);
    sem_val = semctl(fd_sem, 0, GETVAL, 0);
    printf("\n<CLIENT 1>\nSem val = %d {Unlock server}\n", sem_val);

    // --------------------------------------------------------
    // Получение ответа от сервера

    semop(fd_sem, &sem_unlock[1], 1);
    sem_val = semctl(fd_sem, 0, GETVAL, 0);
    printf("\n<CLIENT 1>\nSem val = %d {Lock client 1, wait server...}\n", sem_val);

    char answer[2048];
    strcpy(answer, addr);
    printf("\n<CLIENT 1>\nMessage from server:\n%s\n", answer);

    shmdt(addr);

    semop(fd_sem, &sem_wait[1], 1);
    sem_val = semctl(fd_sem, 0, GETVAL, 0);
    printf("\n<CLIENT 1>\nSem val = %d {Unlock server}\n", sem_val);

    // shmctl(fd_shm, IPC_RMID, 0);
    // semctl(fd_sem, 0, IPC_RMID);

    return 0;
}