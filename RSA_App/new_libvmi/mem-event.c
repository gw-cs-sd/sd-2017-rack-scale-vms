#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <inttypes.h>
#include <sys/mman.h>
#include <string.h>


#include <libvmi/libvmi.h>
#include <libvmi/events.h>

static int interrupted = 0;
static int mem_cb_count = 0;

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
    unsigned long phys_addr = 0ULL;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: mem-event <name of VM> <physical addr to monitor>\n");
        exit(1);
    }

    vm_name = argv[1];
    phys_addr = strtoull(argv[2], NULL, 16);

    fprintf(stdout, "Monitoring phys addr %lu on \"%s\"\n", phys_addr, vm_name);
    fprintf(stdout, "Monitoring phys addr %lx on \"%s\"\n", phys_addr, vm_name);

    act.sa_handler = close_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGHUP,   &act, NULL);
    sigaction(SIGTERM,  &act, NULL);
    sigaction(SIGINT,   &act, NULL);
    sigaction(SIGALRM,  &act, NULL);

    status = vmi_init(
                &vmi,
                (VMI_XEN | VMI_INIT_PARTIAL | VMI_INIT_EVENTS),
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
        return 1;
    }

    memset(&mem_event, 0, sizeof(vmi_event_t));

    SETUP_MEM_EVENT(&mem_event,
                    phys_addr,
                    //VMI_MEMACCESS_RWX,
                    VMI_MEMACCESS_RW,
                    mem_event_cb,
                    0
                    );

    status = vmi_register_event(
                    vmi,
                    &mem_event
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to register mem event...DIE!\n");
        return 1;
    }

    status = vmi_resume_vm(vmi);
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to resume VM...DIE!\n");
        return 1;
    }

    while (!interrupted)
    {
        status = vmi_events_listen(vmi, 500);
        if (status != VMI_SUCCESS)
        {
            fprintf(stdout, "Error waiting for events, DIE...!\n");
            interrupted = -1;
        }
    }

    fprintf(stdout, "Finished mem-event test\n");

    vmi_destroy(vmi);

    return 0;
}

event_response_t
mem_event_cb(
    vmi_instance_t  vmi,
    vmi_event_t     *event
    )
{
    status_t status = VMI_SUCCESS;

    mem_cb_count++;

    if (event->mem_event.out_access & VMI_MEMACCESS_R)
    {
        fprintf(
            stdout,
            "Memory event: VCPU=%u\tIN_PA=%" PRIx64 "\tOUT_GLA=%" PRIx64 "\tOUT_GFN=%" PRIx64 "\n",
            event->vcpu_id,
            event->mem_event.physical_address,
            event->mem_event.gla,
            event->mem_event.gfn
            );

        fprintf(
            stdout,
            "\tOUT_ACCESS = read\n"
            );
    }
    else
    {
        fprintf(
            stdout,
            "Memory event: VCPU=%u\tIN_PA=%" PRIx64 "\tOUT_GLA=%" PRIx64 "\tOUT_GFN=%" PRIx64 "\n",
            event->vcpu_id,
            event->mem_event.physical_address,
            event->mem_event.gla,
            event->mem_event.gfn
            );

        fprintf(
            stdout,
            "\tOUT_ACCESS != read\n"
            );
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
                    step_cb
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to step event...DIE!\n");
        return 1;
    }

    fprintf(stdout, "Mem CB Count = %d\n", mem_cb_count);

    return 0;
}

event_response_t
step_cb(
    vmi_instance_t  vmi,
    vmi_event_t     *event
    )
{
    status_t status = VMI_SUCCESS;

    fprintf(stdout, "Re-registering mem event\n");

    status = vmi_register_event(
                    vmi,
                    event
                    );
    if (status == VMI_FAILURE)
    {
        fprintf(stdout, "Failed to re-register mem event...DIE!\n");
        return 1;
    }

    return 0;
}


static
void
close_handler(
    int sig
    )
{
    interrupted = sig;
}
