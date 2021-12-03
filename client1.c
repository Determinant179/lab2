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
    if ((fp = popen("ps -eo pid,ni,time | awk \'{if ( $2 > 10 ) print }\'", 'r')) == NULL)
    {
        fprintf(stderr, "\nCLIENT 1:\nError while popen\n");
        return 3;
    }

    char output[2048];
    fgets(output, 2048, fp);

    printf("%s", output);

    return 0;
}