/*
 * Dummy program to allocate pages and test userfault fd on.
 *
 * Author: Neel Shah
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>

#define NUM_PAGES 2
#define DEBUG 1
#define _MALLOC_

#define DEBUG_PRINT(_str, ...)                                  \
    do {                                                        \
        if (DEBUG) {                                            \
            fprintf(stdout,                                     \
                    "DEBUG: " _str "",                          \
                    ##__VA_ARGS__);                             \
        }                                                       \
    } while (0)

#define DEBUG_CODE(...)                                         \
    do {                                                        \
        if (DEBUG) {                                            \
            ##__VA_ARGS__                                       \
        }                                                       \
    } while (0)

#define DEBUG_PAUSE()                                           \
    do {                                                        \
        if (DEBUG) {                                            \
            fprintf(stdout,                                     \
                    "[%d] Hit [ENTER] to continue exec...\n",   \
                    __LINE__);                                  \
                char enter = 0;                                 \
                while (enter != '\r' && enter != '\n') {        \
                    enter = getchar();                          \
                }                                               \
        }                                                       \
    } while(0)

unsigned long pagesize;

void *touch_memory(void *memory)
{
    fprintf(stdout, "-- page1: %p - %s\n", memory, (char *)memory);
    fprintf(stdout, "-- page2: %p - %s\n", memory+pagesize, (char *)(memory+pagesize));

    return NULL;
}

int main (int argc, char **argv)
{
    int return_code = 0;
    pid_t pid = 0;

    pid = getpid();
    if (pid < 0) {
        fprintf(stderr, "ERROR: unable to getpid: %m\n");
        goto cleanup_error;
    }

    fprintf(stdout, "INFO: my pid = %d\n", pid);

    pagesize = sysconf(_SC_PAGESIZE);

#ifdef _MALLOC_
    void *pages2 = malloc(pagesize);
    if (pages2 == NULL) {
        return -1;
    }

    fprintf(stdout, "INFO: Page mallocd at addr: %p\n", pages2);
#endif

    void *pages = NULL;
    if ((pages = mmap(NULL, pagesize * NUM_PAGES, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
        fprintf(stderr, "ERROR: mmap failed: %m\n");
        goto cleanup_error;
    }

    if (posix_memalign(&pages, PAGESIZE, PAGESIZE * 2) != 0) {
    //pages = malloc(PAGESIZE * 2);
    //if (pages == NULL) {
        fprintf(stderr, "ERROR: posix_memalign, gonna perror\n");
        perror("ERROR: posix_memalign\n");
    }
    assert(pages);

    if (mprotect(pages, PAGESIZE * 2, PROT_READ | PROT_WRITE) != 0) {
        fprintf(stderr, "ERROR: mprotect, gonna perror\n");
        perror("ERROR: mprotect\n");
    }

    fprintf(stdout, "INFO: %d Pages allocated: %p and %p\n", NUM_PAGES, pages, pages+pagesize);

    if (!sprintf((char *)pages, "neel shah touched page 1")) {
        fprintf(stderr, "ERROR: sprintf failed %m\n");
        goto cleanup_error;
    }

    if (!sprintf((char *)(pages + pagesize), "neel shah touched page 2")) {
        fprintf(stderr, "ERROR: sprintf failed %m\n");
        goto cleanup_error;
    }

    DEBUG_PRINT("page1 contents: %s\n", (char *)pages);
    DEBUG_PRINT("page2 contents: %s\n", (char *)(pages + pagesize));

    DEBUG_PAUSE();

#ifdef _MALLOC_
    free(pages2);
#endif

    pthread_t thread = {0};
    if (pthread_create(&thread, NULL, touch_memory, pages)) {
        fprintf(stderr, "ERROR: pthread_create failed: %m\n");
        goto cleanup_error;
    }

    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "ERROR: pthread_join failed %m\n");
        goto cleanup_error;
    }

    goto cleanup;

cleanup_error:
    return_code = 1;

cleanup:
    return return_code;
}
