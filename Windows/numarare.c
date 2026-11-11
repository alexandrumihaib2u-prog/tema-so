/*
 * Tema SO - Comunicare intre doua procese prin obiect mapat in memorie + semafor
 * Varianta Windows (CreateFileMapping / CreateSemaphore)
 *
 * Doua procese numara impreuna de la 1 la 1000 in memoria partajata:
 *   - rezerva semaforul
 *   - citesc memoria
 *   - dau cu banul (random(2)); cat timp cade 2, scriu in memorie urmatorul numar
 *   - elibereaza semaforul
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUME_MAPARE  "Local\\SO_Tema_Memorie"
#define NUME_SEMAFOR "Local\\SO_Tema_Semafor"
#define LIMITA       1000

typedef struct {
    int numar;            /* ultimul numar scris in memorie */
    int procese_pornite;  /* folosit ca sa pornim numaratoarea abia cand sunt 2 procese */
} DatePartajate;

static int random2(void) { return rand() % 2 + 1; }   /* 1 sau 2 */

int main(void)
{
    DWORD pid = GetCurrentProcessId();
    srand((unsigned)time(NULL) ^ (pid << 16));

    /* Semafor binar cu nume: valoare initiala 1, maxim 1.
       Primul proces il creeaza, al doilea il deschide pe acelasi. */
    HANDLE sem = CreateSemaphoreA(NULL, 1, 1, NUME_SEMAFOR);
    if (sem == NULL) {
        printf("CreateSemaphore a esuat (%lu)\n", GetLastError());
        return 1;
    }

    /* Obiect mapat in memorie, sustinut de fisierul de paginare.
       Memoria noua este initializata automat cu 0. */
    HANDLE mapare = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                       0, sizeof(DatePartajate), NUME_MAPARE);
    if (mapare == NULL) {
        printf("CreateFileMapping a esuat (%lu)\n", GetLastError());
        CloseHandle(sem);
        return 1;
    }

    DatePartajate *date = (DatePartajate *)MapViewOfFile(mapare, FILE_MAP_ALL_ACCESS,
                                                         0, 0, sizeof(DatePartajate));
    if (date == NULL) {
        printf("MapViewOfFile a esuat (%lu)\n", GetLastError());
        CloseHandle(mapare);
        CloseHandle(sem);
        return 1;
    }

    /* Ma inregistrez in memoria partajata */
    WaitForSingleObject(sem, INFINITE);
    date->procese_pornite++;
    ReleaseSemaphore(sem, 1, NULL);

    printf("[PID %lu] pornit, astept al doilea proces...\n", pid);
    for (;;) {
        WaitForSingleObject(sem, INFINITE);
        int n = date->procese_pornite;
        ReleaseSemaphore(sem, 1, NULL);
        if (n >= 2) break;
        Sleep(50);
    }
    printf("[PID %lu] incep numaratoarea\n", pid);

    int scrise = 0;
    for (;;) {
        WaitForSingleObject(sem, INFINITE);          /* rezerv semaforul */

        int n = date->numar;                          /* citesc memoria */
        if (n >= LIMITA) {
            ReleaseSemaphore(sem, 1, NULL);
            break;
        }

        while (n < LIMITA && random2() == 2) {        /* dau cu banul */
            n++;
            date->numar = n;                          /* scriu urmatorul numar */
            scrise++;
            printf("[PID %lu] %d\n", pid, n);
        }

        ReleaseSemaphore(sem, 1, NULL);               /* eliberez semaforul */
        Sleep(1);                                     /* las si celalalt proces sa intre */
    }

    printf("[PID %lu] terminat. Am scris %d numere.\n", pid, scrise);

    UnmapViewOfFile(date);
    CloseHandle(mapare);
    CloseHandle(sem);
    return 0;
}
