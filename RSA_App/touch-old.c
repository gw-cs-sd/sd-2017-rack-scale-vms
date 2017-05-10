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
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <math.h>
#include <strings.h>
#include <asm/unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include </usr/src/linux/include/uapi/linux/userfaultfd.h>
//#include <linux/userfaultfd.h>
#include <poll.h>
#include <sys/syscall.h>

#define UNUSED(x) x __attribute__((__unused__))
#define DEBUG 1
#define PAGESIZE 4096
#define NUM_PAGES 2

#define DEBUG_PRINT(_str, ...)                                  \
    do {                                                        \
        if (DEBUG) {                                            \
            printf("DEBUG: " _str "",                           \
                   ##__VA_ARGS__);                              \
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
            printf("[%d] Hit [ENTER] to continue exec...\n",    \
                   __LINE__);                                   \
                char enter = 0;                                 \
                while (enter != '\r' && enter != '\n') {        \
                    enter = getchar();                          \
                }                                               \
        }                                                       \
    } while(0)

typedef long long addr_t;

const addr_t magic_num1 = 0xDEADBEEF;
const addr_t magic_num2 = 0xBAADA555;
char magic_buf1[PAGESIZE];
char magic_buf2[PAGESIZE];

struct page_attr {
    int has_been_brought_in_p;
    int has_been_written_when_wp_p;
};

struct uffd_params {
    int uffd;
    addr_t *addr_space;
    size_t size;
    size_t n_pages;
    struct page_attr *page_attrs;
};


void floatsleep(float seconds)
{
  struct timespec req;

  req.tv_sec = floor(seconds);
  req.tv_nsec = (float)(seconds - req.tv_sec) * 1000000000.0;
  nanosleep(&req, NULL);
}

void *uffd_handler(void *args)
{
    struct uffd_params *params = args;
    int uffd = params->uffd;

    printf(" -- INFO: uffd_thread got uffd = %d\n", uffd);

    for (;;) {
        int pollres = 0;
        struct uffd_msg msg;
        struct pollfd pollfd[1];
        pollfd[0].fd = params->uffd;
        pollfd[0].events = POLLIN;

        pollres = poll(pollfd, 1, -1);
        switch(pollres) {
            case -1:
                perror(" -- ERROR: failed to poll userfaultfd");
                continue;
                break;
            case 0:
                continue;
                break;
            case 1:
                break;
            default:
                fprintf(stderr, " -- ERROR: polling failed with res=%d: %m\n", pollres);
                exit(2);
        }
        if (pollfd[0].revents & POLLERR) {
            fprintf(stderr, " -- ERROR: POLLERR recvd when polling userfaultfd: %m\n");
            exit(1);
        }
        if (!(pollfd[0].revents & POLLIN)) {
            continue;
        }

        int ret = 0;
        ret = read(uffd, &msg, sizeof(msg));
        if (ret == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            perror(" -- ERROR: failed to read from userfaultfd");
        }
        if (ret != sizeof(msg)) {
            fprintf(stderr, " -- ERROR: failed to read enough data: %m\n");
            exit(1);
        }

        if (msg.event & UFFD_EVENT_PAGEFAULT) {
            printf(" -- =====> PAGEFAULT on %p | flags=0x%llx write?=0x%llx wp?=0x%llu\n",
                    (void *)msg.arg.pagefault.address,
                    msg.arg.pagefault.flags,
                    msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WRITE,
                    msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WP);
        }

        addr_t addr = msg.arg.pagefault.address;
        addr_t page_start = addr - (addr % PAGESIZE);
        addr_t page = (page_start - (addr_t)params->addr_space) / PAGESIZE;
        if (page > params->n_pages) {
            printf(" -- ERROR: uffd handled page out of bounds: %m\n");
            exit(1);
        } else {
            printf(" -- INFO: uffd handler recvd page %lld\n", page);
            printf(" -- INFO: uffd handler recvd page with contents: %s\n", (char *)addr);
        }

        if (msg.arg.pagefault.flags & UFFD_PAGEFAULT_FLAG_WP) {
            // send write unlock
            struct uffdio_writeprotect wp =
                {
                    .range =
                        {
                            .start = (addr_t)params->addr_space,
                            .len = (addr_t)params->size,
                        },
                    .mode = 0,
                };

            printf(" -- INFO: uffd handler toggling UFFDIO_WRITEPROTECT\n");
            fflush(stdout);

            if (ioctl(uffd, UFFDIO_WRITEPROTECT, &wp)) {
                perror(" -- ERROR: uffd handler failed to ioctl UFFDIO_WRITEPROTECT");
            }

            params->page_attrs[page].has_been_written_when_wp_p = 1;

            //struct uffdio_copy cp =
            //{
            //    .src = (addr_t)magic_buf1,
            //    .dst = (addr_t)addr,
            //    .len = (addr_t)params->size,
            //    .mode = 0,
            //};

            //printf(" -- INFO: uffd handler sending UFFDIO_COPY event\n");
            //fflush(stdout);

            //if (ioctl(uffd, UFFDIO_COPY, &cp)) {
            //    printf(" -- ERROR: uffd handler failed to send UFFDIO_COPY event: %m\n");
            //}

            continue;
        }

        struct uffdio_copy cp =
            {
                .src = (addr_t)magic_buf1,
                .dst = (addr_t)addr,
                .len = (addr_t)params->size,
                .mode = 0,
            };

        printf(" -- INFO: uffd handler sending UFFDIO_COPY event\n");
        fflush(stdout);
        if (ioctl(uffd, UFFDIO_COPY, &cp)) {

            printf(" -- ERROR: uffd handler failed to send UFFDIO_COPY event: %m\n");
        }
    }

    return NULL;
}

//void *touch_memory(void *memory)
//{
//    fprintf(stdout, "-- page1: %p - %s\n", memory, (char *)memory);
//    fprintf(stdout, "-- page2: %p - %s\n", memory+PAGESIZE, (char *)(memory+PAGESIZE));
//
//    return NULL;
//}

int main (int argc, char **argv)
{
    int i = 0;
    int ret = 0;
    pid_t pid = 0;
    int uffd = 0;
    int uffd_flags = 0;
    int expected = 0;
    struct page_attr *page_attrs = NULL;
    addr_t *addr_space = NULL;
    pthread_t uffd_thread = {0};

    pid = getpid();
    if (pid < 0) {
        printf("ERROR: unable to getpid: %m\n");
        exit(1);
    } else {
        printf("INFO: my pid = %d\n", pid);
        fflush(stdout);
    }

    /* Set Up Magic Buffers */
    for (i = 0; i < (PAGESIZE/sizeof(addr_t)); ++i) {
        if (!memcpy((void *)(magic_buf1 + i*sizeof(addr_t)), (void *)&magic_num1, sizeof(addr_t))) {
            printf("ERROR: uffd handler strncpy magic num to buf failed: %m\n");
            exit(1);
        }
        if (!memcpy((void *)(magic_buf2 + i*sizeof(addr_t)), (void *)&magic_num2, sizeof(addr_t))) {
            printf("ERROR: uffd handler strncpy magic num to buf failed: %m\n");
            exit(1);
        }
    }

    printf("INFO: userfaultfd syscall #: %d\n", __NR_userfaultfd);
    fflush(stdout);
    uffd = syscall(__NR_userfaultfd, O_CLOEXEC|O_NONBLOCK);
    if (uffd == -1) {
        printf("ERROR: userfaultfd syscall: %m\n");
        exit(1);
    }

    uffd_flags = fcntl(uffd, F_GETFD, NULL);
    printf("INFO: userfaultfd flags: 0x%llX, fd is %d\n", (addr_t)uffd_flags, uffd);
    fflush(stdout);

    struct uffdio_api api =
        {
            .api        = UFFD_API,
            .features   = 0,
        };

    if (ioctl(uffd, UFFDIO_API, &api)) {
        printf("ERROR: ioctl failed: %m\n");
        exit(1);
    }

    if (api.api != UFFD_API) {
        printf("ERROR: unexepcted UFFD api version: %m\n");
        exit(1);
    } else {
        printf("INFO: uffd features: 0x%llx\n", api.features);
        fflush(stdout);
    }

    addr_space = (addr_t *)mmap(NULL,
                                PAGESIZE * NUM_PAGES,
                                PROT_READ|PROT_WRITE,
                                MAP_PRIVATE|MAP_ANON,
                                -1,
                                0);
    ret = posix_memalign((void **)addr_space, PAGESIZE, PAGESIZE * NUM_PAGES);
    if (ret) {
        printf("ERROR: posix_memalign\n");
        exit(2);
    } else {
        printf("INFO: %d Pages allocated: %p and %p\n",
                NUM_PAGES,
                addr_space,
                addr_space + PAGESIZE/sizeof(addr_t));
        fflush(stdout);
    }

    if (!sprintf((char *)addr_space, "neel shah touched page 1")) {
        printf("ERROR: sprintf failed %m\n");
        exit(1);
    }
    if (!sprintf((char *)(addr_space + PAGESIZE/sizeof(addr_t)), "neel shah touched page 2")) {
        printf("ERROR: sprintf failed %m\n");
        exit(1);
    }

    DEBUG_PRINT("page1 contents: %s\n", (char *)addr_space);
    DEBUG_PRINT("page2 contents: %s\n", (char *)(addr_space + PAGESIZE/sizeof(addr_t)));

    struct uffdio_register reg =
        {
            .mode = UFFDIO_REGISTER_MODE_MISSING|UFFDIO_REGISTER_MODE_WP,
            .range =
                {
                    .start = (unsigned long)addr_space,
                    .len = PAGESIZE * NUM_PAGES,
                }
        };

    if (ioctl(uffd, UFFDIO_REGISTER,  &reg)) {
        printf("ERROR: registering uffd: %m\n");
        exit(1);
    } else {
        printf("INFO: userfaultfd ioctls: 0x%llx\n", reg.ioctls);
    }

    expected = UFFD_API_RANGE_IOCTLS;
    if ((reg.ioctls & expected) != expected) {
        printf("ERROR: unexpected UFFD ioctls: %m\n");
        exit(1);
    }

    page_attrs = malloc(NUM_PAGES * sizeof(struct page_attr));
    if (page_attrs == NULL) {
        printf("ERROR: mallocing page_attrs: %m\n");
        exit(1);
    }
    bzero(page_attrs, NUM_PAGES * sizeof(struct page_attr));

    struct uffd_params params =
        {
            .uffd = uffd,
            .addr_space = addr_space,
            .size = PAGESIZE * NUM_PAGES,
            .n_pages = NUM_PAGES,
            .page_attrs = page_attrs,
        };

    if (pthread_create(&uffd_thread, NULL, uffd_handler, (void *)&params)) {
        printf("ERROR: pthread_create failed: %m\n");
        exit(1);
    }

    printf("INFO: about to test read on page 1:\n"), fflush(stdout);
    printf("PAGE 1's first word: %s\n", (char *)addr_space);

    printf("INFO: about to test read on page 2:\n"), fflush(stdout);
    printf("PAGE 2's first word: %s\n", (char *)(addr_space + PAGESIZE/sizeof(addr_t)));

    printf("INFO: about to test write on page 2:\n"), fflush(stdout);
    *(addr_space + PAGESIZE/sizeof(addr_t)) = 0x02;

//    DEBUG_PAUSE();

    struct uffdio_writeprotect wp =
        {
            .range.start = (unsigned long)addr_space,
            .range.len = PAGESIZE * NUM_PAGES,
            .mode = UFFDIO_WRITEPROTECT_MODE_WP,
        };

    if (ioctl(uffd, UFFDIO_WRITEPROTECT, &wp)) {
        printf("ERROR: ioctl UFFDIO_WRITEPROTECT: %m\n");
        exit(1);
    }

    printf("INFO: main writing into prot_write page 1\n");
    fflush(stdout);
    *addr_space = 0x03;
    printf("INFO: main survived write attempt!\n");
    fflush(stdout);

    //printf("INFO: main writing into prot_write page 2\n");
    //fflush(stdout);
    //*(addr_space + PAGESIZE/sizeof(addr_t)) = 0x04;
    //fflush(stdout);
    //printf("INFO: main survived write attempt!\n");

    if (ioctl(uffd, UFFDIO_UNREGISTER, &reg.range)) {
        printf("ERROR: ioctl UFFDIO_UNREGISTER failed: %m\n");
        exit(1);
    }

    printf("PAGE_1:\n"
           "\t0x%llx \n"
           "\t...\n"
           "\t0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx \n"
           "\t0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx \n"
           "\t0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx \n",
           *addr_space,
           *(addr_space + PAGESIZE/sizeof(long long) - 2*sizeof(addr_t)),
           *(addr_space + 2*sizeof(long long)),
           *(addr_space + 3*sizeof(long long)),
           *(addr_space + 4*sizeof(long long)),
           *(addr_space + 5*sizeof(long long)),
           *(addr_space + 6*sizeof(long long)),
           *(addr_space + 7*sizeof(long long)),
           *(addr_space + 8*sizeof(long long)),
           *(addr_space + 9*sizeof(long long)),
           *(addr_space + 10*sizeof(long long)),
           *(addr_space + 11*sizeof(long long)),
           *(addr_space + 12*sizeof(long long)),
           *(addr_space + 13*sizeof(long long)),
           *(addr_space + 14*sizeof(long long)),
           *(addr_space + 15*sizeof(long long)),
           *(addr_space + 16*sizeof(long long)),
           *(addr_space + 17*sizeof(long long)),
           *(addr_space + 18*sizeof(long long)),
           *(addr_space + 19*sizeof(long long)),
           *(addr_space + 20*sizeof(long long)),
           *(addr_space + 21*sizeof(long long)),
           *(addr_space + 22*sizeof(long long)),
           *(addr_space + 23*sizeof(long long)));

    //if (pthread_join(uffd_thread, NULL)) {
    //    printf("ERROR: pthread_join failed: %m\n");
    //    exit(1);
    //}

    free(page_attrs);

    return 0;
}
