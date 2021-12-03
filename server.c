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
        fprintf(stderr, "\nSERVER:\nError while ShM adding\n");
    }

    // Разблокирование 1-ого клиента
    semop(fd_sem, &unclock_first_client, 1);
    // Ожидание 1-ого клиента
    semop(fd_sem, &wait_first_client, 1);
    printf("\nSERVER:\nWaiting message from client 1...\n");

    char answer1[2048];
    strcpy(answer1, addr);
    printf("\nSERVER:\nMessage from client 1:\n%s\n", answer1);

    int PIDs_amount = 0;
    for (int i = 0; i < strlen(answer1); i++)
    {
        if (answer1[i] == '\n')
            PIDs_amount += 1;
    }

    char PIDs[PIDs_amount][16];
    int PIDs_id = 0;
    int space_counter = 0;

    for (int i = 0; i < PIDs_amount; i++)
        strcpy(PIDs[i], "");

    for (int i = 0; i < strlen(answer1); i++)
    {
        if (answer1[i] == ' ')
        {
            space_counter++;
        }
        else if (space_counter == 2)
            strncat(PIDs[PIDs_id], &answer1[i], 1);

        if (answer1[i] == '\n')
        {
            space_counter = 0;
            PIDs_id++;
        }
    }

    char output[2048] = "";

    for (int i = 0; i < PIDs_amount; i++)
    {

        char line[2048] = "";
        FILE *fp;
        char cmd[2048] = "ps -eo time ";
        strcat(cmd, PIDs[i]);
        strcat(cmd, " | tail +2");

        if ((fp = popen(cmd, "r")) == NULL)
        {
            fprintf(stderr, "\nCLIENT 1:\nError while popen\n");
            return 3;
        }

        fgets(line, 2048, fp);
        strcat(output, line);
    }

    printf("%s", output);


    // Разблокирование 2-ого клиента
    semop(fd_sem, &unclock_second_client, 1);
    // Ожидание 2-ого клиента
    semop(fd_sem, &wait_second_client, 1);

    char answer2[2048];
    strcpy(answer2, addr);
    printf("\nSERVER:\nMessage from client 2:\n%s\n", answer2);

    shmdt(addr);
    shmctl(fd_shm, IPC_RMID, 0);
    semctl(fd_sem, 0, IPC_RMID);

    return 0;
}