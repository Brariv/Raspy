/*
  fork_tree.c
  Demuestra creacion de procesos con fork y permite identificar claramente
  padre e hijos (PID, PPID, generacion y orden).

  Compilar:
    gcc -O2 -Wall -Wextra -o fork_tree fork_tree.c

  Ejecutar (ejemplo):
    ./fork_tree 2 3
    (2 niveles, 3 hijos por proceso)

  Nota:
  Cada proceso imprime:
    PID  PPID  gen  idx  parent_pid
  donde:
    gen = nivel (0 es el proceso inicial)
    idx = numero de hijo (1..hijos) dentro de su padre
    parent_pid = PID del padre original en ese fork
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static int parse_int(const char *s, int fallback) {
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (s == NULL || *s == '\0' || end == s || *end != '\0') return fallback;
    if (v < 0 || v > 1000) return fallback;
    return (int)v;
}

static void print_proc_line(int gen, int idx, pid_t parent_pid) {
    printf("PID=%ld  PPID=%ld  gen=%d  idx=%d  parent_pid=%ld\n",
           (long)getpid(), (long)getppid(), gen, idx, (long)parent_pid);
    fflush(stdout);
}

static void spawn_children(int gen, int max_gen, int children_per_proc) {
    if (gen >= max_gen) return;

    for (int i = 1; i <= children_per_proc; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            /* si falla un fork, esperamos lo que ya se creo y salimos */
            for (;;) {
                int st;
                pid_t w = wait(&st);
                if (w <= 0) break;
            }
            exit(1);
        }

        if (pid == 0) {
            /* hijo */
            pid_t my_parent = getppid();
            print_proc_line(gen + 1, i, my_parent);

            /* expandimos el arbol */
            spawn_children(gen + 1, max_gen, children_per_proc);

            /* para que el orden se vea mas estable en pantalla */
            usleep(20000);

            exit(0);
        }

        /* padre continua para crear mas hijos */
    }

    /* padre espera a TODOS sus hijos directos */
    for (int j = 0; j < children_per_proc; j++) {
        int status;
        (void)wait(&status);
    }
}

int main(int argc, char **argv) {
    int levels = 2;            /* profundidad del arbol */
    int children = 2;          /* hijos por proceso */

    if (argc >= 2) levels = parse_int(argv[1], levels);
    if (argc >= 3) children = parse_int(argv[2], children);

    if (levels < 0) levels = 0;
    if (children < 0) children = 0;

    printf("Creando arbol de procesos con fork\n");
    printf("levels=%d  children_per_proc=%d\n\n", levels, children);

    /* proceso raiz */
    print_proc_line(0, 0, 0);

    spawn_children(0, levels, children);

    printf("\nProceso raiz termina: PID=%ld\n", (long)getpid());
    return 0;
}
