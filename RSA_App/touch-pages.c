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
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <sys/syscall.h>

#include "../RSApi/includes.h"

#define UNUSED(x) x __attribute__((__unused__))
#define NUM_PAGES 2
#define DEBUG 1
#define pagesize 4096

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

int userfaultfd(int flags)
{
	return syscall(SYS_userfaultfd, flags);
}

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
    int fd = 0;

    char data[pagesize] = "pthread touched page, so handler put this there";

    pid = getpid();
    if (pid < 0) {
        fprintf(stderr, "ERROR: unable to getpid: %m\n");
        goto cleanup_error;
    }

    fprintf(stdout, "INFO: my pid = %d\n", pid);

    if ((fd = userfaultfd(O_NONBLOCK)) == -1) {
        fprintf(stderr, "userfaultfd failed: %m\n");
    }

    struct uffdio_api api =
        {
            .api = UFFD_API
        };
    if (ioctl(fd, UFFDIO_API, &api)) {
        fprintf(stderr, "ioctl failed: %m\n");
    }

    if (api.api != UFFD_API) {
		fprintf(stderr, "++ unexepcted UFFD api version.\n");
		goto cleanup_error;
	}

    void *pages = NULL;
    //if ((pages = mmap(NULL, pagesize * NUM_PAGES, PROT_READ | PROT_WRITE,
    //          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
    //    fprintf(stderr, "ERROR: mmap failed: %m\n");
    //    goto cleanup_error;
    //}

    if (posix_memalign(&pages, pagesize, pagesize * 2) != 0) {
        fprintf(stderr, "ERROR: posix_memalign, gonna perror\n");
        perror("ERROR: posix_memalign\n");
    }
    assert(pages);

    if (mprotect(pages, pagesize * 2, PROT_READ | PROT_WRITE) != 0) {
        fprintf(stderr, "ERROR: mprotect, gonna perror\n");
        perror("ERROR: mprotect\n");
    }

    fprintf(stdout, "INFO: %d Pages allocated: %p and %p\n", NUM_PAGES, pages, pages+pagesize);

    struct uffdio_register reg =
        {
            .mode = UFFDIO_REGISTER_MODE_MISSING,
            .range =
                {
                    .start = (long) pages,
                    .len = pagesize * 2
                }
	    };
	if (ioctl(fd, UFFDIO_REGISTER,  &reg)) {
		fprintf(stderr, "++ ioctl(fd, UFFDIO_REGISTER, ...) failed: %m\n");
		goto cleanup_error;
	}
	if (reg.ioctls != UFFD_API_RANGE_IOCTLS) {
		fprintf(stderr, "++ unexpected UFFD ioctls.\n");
		goto cleanup_error;
	}

    DEBUG_PAUSE();

    pthread_t thread = {0};
    if (pthread_create(&thread, NULL, touch_memory, pages)) {
        fprintf(stderr, "ERROR: pthread_create failed: %m\n");
        goto cleanup_error;
    }

    /**** Handler ****/

    struct pollfd evt =
        {
            .fd = fd,
            .events = POLLIN
        };

	while (poll(&evt, 1, 10) > 0) {
		/* unexpected poll events */
		if (evt.revents & POLLERR) {
			fprintf(stderr, "++ POLLERR\n");
			goto cleanup_error;
		} else if (evt.revents & POLLHUP) {
			fprintf(stderr, "++ POLLHUP\n");
			goto cleanup_error;
		}
		struct uffd_msg fault_msg = {0};
		if (read(fd, &fault_msg, sizeof(fault_msg)) != sizeof(fault_msg)) {
			fprintf(stderr, "++ read failed: %m\n");
			goto cleanup_error;
		}
		char *place = (char *)fault_msg.arg.pagefault.address;
		if (fault_msg.event != UFFD_EVENT_PAGEFAULT
		    || (place != pages && place != pages + pagesize)) {
			fprintf(stderr, "unexpected pagefault?.\n");
			goto cleanup_error;
		}

        fprintf(stderr, "Someone touched a page!!\n");
        fprintf(stderr, "Download some memory and put it there!\n");
        RSA_Mem_Data fault =
            {
                .unVirtAddr = (long)place,
                .pPageData = data
            };
        pRSA_Mem_Data recvd = NULL;

        rsa_download_more_ram(
                        &fault,
                        &recvd
                        );
        fprintf(stderr, "Downloaded memory, copying data to page\n");

		struct uffdio_copy copy =
            {
                .dst = (long) place,
                .src = (long) recvd->pPageData,
                .len = pagesize
            };
		if (ioctl(fd, UFFDIO_COPY, &copy)) {
			fprintf(stderr, "++ ioctl(fd, UFFDIO_COPY, ...) failed: %m\n");
			goto cleanup_error;
		}
    }

    if (pthread_join(thread, NULL)) {
        fprintf(stderr, "ERROR: pthread_join failed %m\n");
        goto cleanup_error;
    }

    goto cleanup;

cleanup_error:
    return_code = 1;

cleanup:
    if (fd) close(fd);
    return return_code;
}
