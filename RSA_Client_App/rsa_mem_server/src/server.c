/*
 * This is a simple server that Rx/Tx RSA_Mem_Data to/from a client requesting
 * more memory.
 */

#include "includes.h"

int
main(
    int     argc,
    char    **argv
    )
{
    int rc = NO_ERROR;
    int sockfd = 0;
    int idx = 0;
    int yes = 1;
    int ipaddr = 0;
    int clientfd = 0;
    int bytes_read = 0;
    struct addrinfo hints = { 0 };
    struct addrinfo *server = NULL;
    struct sockaddr_storage client_addr = { 0 };
    socklen_t addr_size = 0;
    pRSA_Mem_Data pRSAMemData = NULL;
    pRSA_Byte_Stream pByteStream = NULL;

    printf("RSA Mem Server INFO: Listening on port: %s\n", PORT);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; /* or SOCK_DGRAM */
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(
            NULL,
            PORT,
            &hints,
            &server
            );
    BAIL_ERROR(rc);

    sockfd = socket(
                server->ai_family,
                server->ai_socktype,
                server->ai_protocol
                );
    if (sockfd == -1)
    {
        rc = COM_ERROR;
        printf("ERROR: could not create socket\n");
        BAIL_ERROR(rc);
    }

    rc = setsockopt(
            sockfd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &yes,
            sizeof(int)
            );
    BAIL_ERROR(rc);

    rc = bind(
            sockfd,
            server->ai_addr,
            server->ai_addrlen
            );
    BAIL_ERROR(rc);

    listen(sockfd, BACKLOG);

    for (;;)
    {
        addr_size = sizeof(client_addr);

        clientfd = accept(
                        sockfd,
                        (struct sockaddr *)&client_addr,
                        &addr_size
                        );
        if (clientfd == -1)
        {
            rc = COM_ERROR;
            printf("ERROR: could not accept connection on socket\n");
            BAIL_ERROR(rc);
        }

        rc = rsa_buffer_init_buffer(
                        200,
                        &pByteStream
                        );
        BAIL_ERROR(rc);

        rc = rsa_buffer_read_from_socket(
                            clientfd,
                            pByteStream
                            );
        BAIL_ERROR(rc);

        if (DEBUG)
        {
            printf("DEBUG: client ip= ");

            for (idx = 2; idx < 6; ++idx)
            {
                ipaddr = (int)(((struct sockaddr *)&client_addr)->sa_data[idx]);
                ipaddr = (ipaddr + 256) % 256;
                printf("%d.",ipaddr);
            }
            printf("\n");
        }

        rc = rsa_deserialize_mem_data(
                        pByteStream,
                        &pRSAMemData
                        );
        BAIL_ERROR(rc);

        printf("Recevied data:\n");
        printf("\tPhysAddr: %" PRIu64 "\n", pRSAMemData->unPhysAddr);
        printf("\tVirtAddr: %" PRIu64 "\n", pRSAMemData->unVirtAddr);

        rsa_cleanup_mem_data(pRSAMemData);
        close(clientfd);
    }


cleanup:
    freeaddrinfo(server);
    close(sockfd);

    printf("\nExiting\n");
    return rc;

error:

    goto cleanup;
}
