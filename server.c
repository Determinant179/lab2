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


struct sembuf
    sem_unlock[] = {{0, 1, 0}, {0, 3, 0}, {0, 5, 0}},
    sem_wait[] = {{0, -2, 0}, {0, -4, 0}, {0, -6, 0}};

int main()
{

    int rtrn;

    // Создание РОП
    int fd_shm = shmget(3, 2048, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_shm == -1)
    {
        fprintf(stderr, "\n<SERVER>\nError while ShM creation\n");
        return 1;
    }
    printf("\n<SERVER>\nSnM was created\n");

    // Создание семафоров
    int fd_sem = semget(4, 1, IPC_CREAT | IPC_EXCL | 0664);
    if (fd_sem == -1)
    {
        fprintf(stderr, "\n<SERVER>\nError while semaphore set creation\n");
        return 2;
    }
    printf("\n<SERVER>\nSemaphore set was created\n");

    // Добавление РОП
    char *addr = shmat(fd_shm, 0, 0);
    if (addr == (char *)-1)
    {
        fprintf(stderr, "\n<SERVER>\nError while ShM adding\n");
    }

    // Разблокирование 1-ого клиента
    printf("sem -> 1\n");
    semop(fd_sem, &sem_unlock[0], 1);
    rtrn = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", rtrn);

    // Ожидание 1-ого клиента
    printf("sem -> -2\n");
    semop(fd_sem, &sem_wait[0], 1);
    rtrn = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", rtrn);

    printf("\n<SERVER>\nWaiting message from client 1...\n");

    char answer1[2048];
    strcpy(answer1, addr);
    printf("\n<SERVER>\nMessage from client 1:\n%s\n", answer1);

    int PIDs_amount = 0;
    for (int i = 0; i < strlen(answer1); i++)
    {
        if (answer1[i] == '\n')
            PIDs_amount += 1;
    }

    char PIDs[PIDs_amount][16];
    int PIDs_id = 0;
    int isPIDFound = 0;

    for (int i = 0; i < PIDs_amount; i++)
        strcpy(PIDs[i], "");

    for (int i = 0; i < strlen(answer1) - 1; i++)
    {
        if (answer1[i] != ' ' && isPIDFound == 0)
        {
            strncat(PIDs[PIDs_id], &answer1[i], 1);
            if (answer1[i + 1] == ' ')
                isPIDFound = 1;
        }

        if (answer1[i] == '\n')
        {
            isPIDFound = 0;
            PIDs_id++;
        }
    }

    char output_line[2048] = "";
    char output[2048] = "";

    for (int i = 0; i < PIDs_amount; i++)
    {

        char line[2048] = "";

        FILE *fp;
        char cmd[2048] = "ps -eo ppid ";
        strcat(cmd, PIDs[i]);
        strcat(cmd, " | tail +2");

        if ((fp = popen(cmd, "r")) == NULL)
        {
            fprintf(stderr, "\nCLIENT 1:\nError while popen\n");
            return 3;
        }

        fgets(line, 2048, fp);
        line[strlen(line) - 1] = '\0';

        FILE *fp2;
        char cmd2[2048] = "ps -eo comm ";
        strcat(cmd2, line);
        strcat(cmd2, " | tail +2");

        if ((fp2 = popen(cmd2, "r")) == NULL)
        {
            fprintf(stderr, "\nCLIENT 1:\nError while popen\n");
            return 3;
        }

        fgets(output_line, 2048, fp2);
        strcat(output, output_line);
    }

    // ------------------------------------------------------------

    strcpy(addr, output);
    // shmdt(addr);
    printf("sem -> 3\n");
    semop(fd_sem, &sem_unlock[1], 1);
    rtrn = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", rtrn);

    printf("sem -> -4\n");
    semop(fd_sem, &sem_wait[1], 1);
    rtrn = semctl(fd_sem, 0, GETVAL, 0);
    printf("\nЗначение семафора = %d\n", rtrn);

    // ------------------------------------------------------------

    // Добавление РОП
    // char *addr2 = shmat(fd_shm, 0, 0);
    // if (addr2 == (char *)-1)
    // {
    //     fprintf(stderr, "\n<CLIENT 1>\nError while ShM adding\n");
    // }

    // Разблокирование 2-ого клиента
    printf("sem -> 5\n");
    semop(fd_sem, &sem_unlock[3], 1);

    // Ожидание 2-ого клиента
    printf("sem -> -6\n");
    semop(fd_sem, &sem_wait[2], 1);

    char answer2[2048];
    strcpy(answer2, addr);
    printf("\n<SERVER>\nMessage from client 2:\n%s\n", answer2);

    shmdt(addr);
    shmctl(fd_shm, IPC_RMID, 0);
    semctl(fd_sem, 0, IPC_RMID);

    return 0;
}