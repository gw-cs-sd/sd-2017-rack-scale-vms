/*
 * Title: rsa_mem_server.c
 * Author: Neel Shah
 * Date: 04/11/2017
 * Abstract:
 *     This is a simple server that Rx/Tx RSA_Mem_Data to/from a client requesting
 *     more memory.  It then stores the received pages+data into a hash table for
 *     easy referencing.
 */
#include <signal.h>

#include "../RSApi/includes.h"

#define UNUSED(x) x __attribute__((__unused__))
#define TEST


static int death = 0;

static void
do_networking(void);

static void
signal_handler(int sig);


int
main(
    int     argc,
    char    **argv
    )
{
    do_networking();

    return 0;
}


static void
do_networking(void)
{
    int rc = NO_ERROR;
    int sockfd = 0;
    int sockopts = AI_PASSIVE;
    int idx = 0;
    int yes = 1;
    int ipaddr = 0;
    int clientfd = 0;
    int bytes_read = 0;
    int bytes_sent = 0;
    struct addrinfo *server = NULL;
    struct sockaddr_storage client_addr = { 0 };
    socklen_t addr_size = 0;
    pRSA_Byte_Stream pRecvBuffer = NULL;
    pRSA_Byte_Stream pSendBuffer = NULL;
    pRSA_Mem_Data pRecvMemData = NULL;
    UNUSED(pRSA_Mem_Data pSendMemData) = NULL;

    signal(SIGINT,  &signal_handler);
    signal(SIGTERM, &signal_handler);

    rc = rsa_net_create_socket(
                    sockopts,
                    &server,
                    &sockfd
                    );
    BAIL_ERROR(rc);

    rc = setsockopt(
            sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &yes,
            sizeof(int)
            );
    BAIL_ERROR(rc);

    rsa_net_connect_socket(
                RSA_PROC_SERVER,
                server,
                sockfd
                );
    BAIL_ERROR(rc);

    listen(sockfd, RSA_BACKLOG);

    printf("INFO: RSA Mem Server Started: Listening on: %s:%s\n", RSA_IP_ADDR, RSA_PORT);

    for (;;)
    {
        addr_size = sizeof(client_addr);

        clientfd = accept(
                        sockfd,
                        (struct sockaddr *)&client_addr,
                        &addr_size
                        );
        //RSA_LOG("could not accept connection on socket\n");
        BAIL_IF_ERROR(clientfd == -1, COM_ERROR, rc);

        rc = rsa_buffer_init_buffer(
                        PAGESIZE + sizeof(uint64_t),
                        &pRecvBuffer
                        );
        BAIL_ERROR(rc);

        rc = rsa_buffer_read_from_socket(
                            clientfd,
                            pRecvBuffer,
                            &bytes_read
                            );
        BAIL_ERROR(rc);

        if (DEBUG)
        {
            printf("NEW CONNECTION: client ip = ");

            for (idx = 2; idx < 6; ++idx)
            {
                ipaddr = (int)(((struct sockaddr *)&client_addr)->sa_data[idx]);
                ipaddr = (ipaddr + 256) % 256;
                printf("%d.", ipaddr);
            }
            printf("\n");
        }

        rc = rsa_deserialize_mem_data(
                        pRecvBuffer,
                        &pRecvMemData
                        );
        BAIL_ERROR(rc);

        printf("Recevied data:\n");
        printf("\tVirtAddr: %" PRIx64 "\n", pRecvMemData->unVirtAddr);
        printf("\tPageData Pointer: %p\n", pRecvMemData->pPageData);
        printf("\tPageData Contents as str: %s\n", pRecvMemData->pPageData);

        // TODO: Lookup recv data in ht, store it, serve it

        printf("Sending response\n");

        rc = rsa_buffer_init_buffer(
                        PAGESIZE + sizeof(uint64_t),
                        &pSendBuffer
                        );
        BAIL_ERROR(rc);

#ifdef TEST
        char* dummy = "Memory Downloaded! :)\n";
        RSA_Mem_Data mdTestMemData =
            {
                .unVirtAddr = 0x01020304,
                .pPageData  = (PBYTE)dummy
            };

        rc = rsa_serialize_mem_data(
                        &mdTestMemData,
                        pSendBuffer
                        );
#else
        rc = rsa_serialize_mem_data(
                        pSendMemData,
                        pSendBuffer
                        );
#endif
        BAIL_ERROR(rc);

        rc = rsa_buffer_write_to_socket(
                        clientfd,
                        pSendBuffer,
                        &bytes_sent
                        );
        BAIL_ERROR(rc);

        rsa_cleanup_mem_data(pRecvMemData);
        rsa_buffer_cleanup_buffer(pRecvBuffer);
        rsa_buffer_cleanup_buffer(pSendBuffer);
        pRecvBuffer     = NULL;
        pSendBuffer     = NULL;
        pRecvMemData    = NULL;
        bytes_read      = 0;
        bytes_sent      = 0;
        close(clientfd);

        if (death) break;
    }


cleanup:
    freeaddrinfo(server);
    close(sockfd);

    printf("\nRSA MemServer Exiting\n");
    exit(rc);

error:

    goto cleanup;
}

static void
signal_handler(int sig)
{
        if (sig == SIGINT || sig == SIGTERM) death = 1;
}
