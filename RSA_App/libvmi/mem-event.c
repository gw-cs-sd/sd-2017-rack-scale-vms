#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>

#include <libvmi/libvmi.h>
#include <libvmi/events.h>

#define DEBUG 0

#define DEBUG_PAUSE()                                                       \
    do {                                                                    \
        if (DEBUG) {                                                        \
            printf("[%d] Hit [ENTER] to continue exec...\n", __LINE__);     \
                char enter = 0;                                             \
                while (enter != '\r' && enter != '\n') {                    \
                    enter = getchar();                                      \
                }                                                           \
        }                                                                   \
    } while(0)

static int interrupted = 0;
static int mem_cb_count = 0;

static
void
print_mem_event(
    vmi_event_t*    event
    );

static
void
close_handler(
    int     sig
    );


vmi_event_t mem_event;

event_response_t
mem_event_cb(
    vmi_instance_t  vmi,
    vmi_event_t     *event
    );

event_response_t
step_cb(
    vmi_instance_t  vmi,
    vmi_event_t     *event
    );

int
main(
    int     argc,
    char*   argv[]
    )
{
    status_t status = VMI_SUCCESS;
    vmi_instance_t vmi = NULL;
    struct sigaction act;
    char* vm_name = NULL;
    addr_t phys_addr = 0ULL;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: mem-event <name of VM> <physical addr to monitor>\n");
        exit(1);
    }

    vm_name = argv[1];
    phys_addr = strtoull(argv[2], NULL, 10);

    fprintf(stdout, "Monitoring phys addr %lu on \"%s\"\n", phys_addr, vm_name);
    fprintf(stdout, "Monitoring phys addr %lx on \"%s\"\n", phys_addr, vm_name);

    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP,   &act, NULL);
    sigaction(SIGTERM,  &act, NULL);
    sigaction(SIGINT,   &act, NULL);
    sigaction(SIGALRM,  &act, NULL);
    sigaction(SIGKILL,  &act, NULL);

    status = vmi_init(
                &vmi,
                (VMI_XEN | VMI_INIT_PARTIAL | VMI_INIT_EVENTS),
                //(VMI_XEN | VMI_INIT_COMPLETE | VMI_INIT_EVENTS),
                vm_name
                );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to init LibVMI! :(\n");
        return 1;
    }
    else
    {
        fprintf(stdout, "LibVMI init success! :)\n");
    }

    status = vmi_pause_vm(vmi);
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to pause VM...DIE!\n");
        goto cleanup;
    }

    DEBUG_PAUSE();

    fprintf(stdout,
            "Preparing memory event to monitor PA 0x%lx, page 0x%lx\n",
            phys_addr,
            (phys_addr >> 12)
            );

    memset(&mem_event, 0, sizeof(vmi_event_t));

    SETUP_MEM_EVENT(&mem_event,
                    phys_addr >> 12,
                    VMI_MEMACCESS_RW,
                    mem_event_cb,
                    0
                    );

    DEBUG_PAUSE();

    status = vmi_register_event(
                    vmi,
                    &mem_event
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to register mem event...DIE!\n");
        goto cleanup;
    }

    DEBUG_PAUSE();

    status = vmi_resume_vm(vmi);
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to resume VM...DIE!\n");
        goto cleanup;
    }

    DEBUG_PAUSE();

    while (!interrupted)
    {
        status = vmi_events_listen(vmi, 500);
        if (status != VMI_SUCCESS)
        {
            fprintf(stdout, "Error waiting for events, DIE...!\n");
            interrupted = -1;
        }
    }

cleanup:
    fprintf(stdout, "Finished mem-event test\n");

    vmi_destroy(vmi);

    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Exit with status VMI_FAILURE\n");
    }
    else
    {
        fprintf(stdout, "Exit with status VMI_SUCCESS\n");
    }

    return status;
}

event_response_t
mem_event_cb(
    vmi_instance_t  vmi,
    vmi_event_t     *event
    )
{
    status_t status = VMI_SUCCESS;

    mem_cb_count++;

    print_mem_event(event);

    if (event->mem_event.out_access & VMI_MEMACCESS_R) {
        fprintf(stdout, "\tOUT_ACCESS = read\n");
    }
    else if (event->mem_event.out_access & VMI_MEMACCESS_W) {
        fprintf(stdout, "\tOUT_ACCESS = write\n");
    }

    status = vmi_clear_event(
                    vmi,
                    event,
                    NULL
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to clear mem event in cb...DIE!\n");
        return 1;
    }

    status = vmi_step_event(
                    vmi,
                    event,
                    event->vcpu_id,
                    1,
                    NULL
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to step event...DIE!\n");
        return 1;
    }

    fprintf(stdout, "Mem CB Count = %d\n", mem_cb_count);

    return 0;
}


static
void
print_mem_event(
    vmi_event_t*    event
    )
{
    fprintf(stdout,
            "PAGE %" PRIx64 " ACCESS: %c%c%c for GFN %" PRIx64 " (offset %06" PRIx64 ") gla %016" PRIx64 " (vcpu %u)\n",
                    event->mem_event.gfn >> 12,
            (event->mem_event.out_access & VMI_MEMACCESS_R) ? 'r' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_W) ? 'w' : '-',
            (event->mem_event.out_access & VMI_MEMACCESS_X) ? 'x' : '-',
            event->mem_event.gfn,
            event->mem_event.offset,
            event->mem_event.gla,
            event->vcpu_id
            );
}

static
void
close_handler(
    int sig
    )
{
    interrupted = sig;
}
