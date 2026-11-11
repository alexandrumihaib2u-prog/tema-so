/*
 * Tema SO - Comunicare intre doua procese prin obiect mapat in memorie + semafor
 * Varianta Linux (POSIX shm_open + mmap / sem_open)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define NUME_MAPARE  "/so_tema_memorie"
#define NUME_SEMAFOR "/so_tema_semafor"
#define LIMITA       1000

typedef struct {
    int numar;
    int procese_pornite;
} DatePartajate;

static int random2(void) { return rand() % 2 + 1; }   /* 1 sau 2 */

int main(void)
{
    pid_t pid = getpid();
    srand((unsigned)time(NULL) ^ ((unsigned)pid << 16));

    sem_t *sem = sem_open(NUME_SEMAFOR, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) { perror("sem_open"); return 1; }

    int fd = shm_open(NUME_MAPARE, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(DatePartajate)) == -1) { perror("ftruncate"); return 1; }

    DatePartajate *date = mmap(NULL, sizeof(DatePartajate), PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
    if (date == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    sem_wait(sem);
    date->procese_pornite++;
    sem_post(sem);

    printf("[PID %d] pornit, astept al doilea proces...\n", pid);
    fflush(stdout);
    for (;;) {
        sem_wait(sem);
        int n = date->procese_pornite;
        sem_post(sem);
        if (n >= 2) break;
        usleep(50000);
    }

    int scrise = 0;
    for (;;) {
        sem_wait(sem);                                /* rezerv semaforul */

        int n = date->numar;                          /* citesc memoria */
        if (n >= LIMITA) { sem_post(sem); break; }

        while (n < LIMITA && random2() == 2) {        /* dau cu banul */
            n++;
            date->numar = n;                          /* scriu urmatorul numar */
            scrise++;
            printf("[PID %d] %d\n", pid, n);
            fflush(stdout);
        }

        sem_post(sem);                                /* eliberez semaforul */
        usleep(1000);
    }

    printf("[PID %d] terminat. Am scris %d numere.\n", pid, scrise);

    /* Ultimul proces care iese sterge memoria si semaforul din sistem */
    sem_wait(sem);
    int ramase = --date->procese_pornite;
    sem_post(sem);
    munmap(date, sizeof(DatePartajate));
    sem_close(sem);
    if (ramase == 0) {
        shm_unlink(NUME_MAPARE);
        sem_unlink(NUME_SEMAFOR);
    }
    return 0;
}
