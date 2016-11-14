
#include "includes.h"

int
rsa_init_mem_data(
    uint64_t        unPhysAddr,
    uint64_t        unVirtAddr,
    pRSA_Mem_Data*  ppRSAMemData
    )
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pRSAMemData = NULL;

    if (!ppRSAMemData)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pRSAMemData = malloc(sizeof(RSA_Mem_Data));
    BAIL_PTR_ERROR(pRSAMemData, rc);

    pRSAMemData->unPhysAddr = unPhysAddr;
    pRSAMemData->unVirtAddr = unVirtAddr;

    *ppRSAMemData = pRSAMemData;


cleanup:

    return rc;

error:

    rsa_cleanup_mem_data(pRSAMemData);
    if (ppRSAMemData)
    {
        *ppRSAMemData = NULL;
    }

    goto cleanup;
}

void
rsa_cleanup_mem_data(
    pRSA_Mem_Data   pRSAMemData
    )
{
    if (pRSAMemData)
    {
        SAFE_FREE(pRSAMemData);

        pRSAMemData = NULL;
    }
}

int
rsa_download_more_ram(
    pRSA_Mem_Data   pRSAMemData,
    char**          ppszRetPage
    )
{
        int rc = NO_ERROR;
        int sockfd = 0;
        int o = 0;
        struct addrinfo hints = { 0 };
        struct addrinfo *server = NULL;
        pRSA_Byte_Stream pByteStream = NULL;
        char* pszRetPage = "Neel";

        if (!ppszRetPage || !pRSAMemData)
        {
            rc = ARG_ERROR;
            BAIL_ERROR(rc);
        }

        if (DEBUG) printf("DEBUG Server IP: %s\tServer Port: %s\n", IP_ADDR, PORT);

        if (!memset(
                &hints,
                0,
                sizeof(hints)))
        {
            rc = COM_ERROR;
            printf("ERROR: memset did not work\n");
            BAIL_ERROR(rc);
        }

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        rc = getaddrinfo(
                IP_ADDR,
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
            printf("ERROR: couldn't create socket\n");
            BAIL_ERROR(rc);
        }

        rc = connect(
                sockfd,
                server->ai_addr,
                server->ai_addrlen
                );
        BAIL_ERROR(rc);

        rc = rsa_buffer_init_buffer(
                        200,
                        &pByteStream
                        );
        BAIL_ERROR(rc);

        rc = rsa_serialize_mem_data(
                        pRSAMemData,
                        pByteStream
                        );
        BAIL_ERROR(rc);

        rc = rsa_buffer_write_to_socket(
                            sockfd,
                            pByteStream
                            );
        BAIL_ERROR(rc);

        *ppszRetPage = pszRetPage;


cleanup:
    freeaddrinfo(server);
    close(sockfd);
    rsa_buffer_cleanup_buffer(pByteStream);

    printf("\nExiting\n");
    return 0;

error:

    SAFE_FREE(pszRetPage);
    if (ppszRetPage)
    {
        *ppszRetPage = NULL;
    }

    goto cleanup;
}

int
rsa_serialize_mem_data(
    pRSA_Mem_Data       pRSAMemData,
    pRSA_Byte_Stream    pByteStream
    )
{
    int rc = NO_ERROR;

    if (!pRSAMemData || !pByteStream)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    rc = rsa_buffer_serialize_uint64(
                        pRSAMemData->unPhysAddr,
                        pByteStream
                        );
    BAIL_ERROR(rc);

    rc = rsa_buffer_serialize_uint64(
                        pRSAMemData->unVirtAddr,
                        pByteStream
                        );
    BAIL_ERROR(rc);


cleanup:

    return rc;

error:

    goto cleanup;
}

int
rsa_deserialize_mem_data(
    pRSA_Byte_Stream    pByteStream,
    pRSA_Mem_Data*      ppRSAMemData
    )
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pRSAMemData = NULL;

    if (!pByteStream || !ppRSAMemData)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pRSAMemData = malloc(sizeof(RSA_Mem_Data));
    BAIL_PTR_ERROR(pRSAMemData, rc);

    rc = rsa_buffer_deserialize_uint64(
                        pByteStream,
                        &pRSAMemData->unPhysAddr
                        );
    BAIL_ERROR(rc);

    rc = rsa_buffer_deserialize_uint64(
                        pByteStream,
                        &pRSAMemData->unVirtAddr
                        );
    BAIL_ERROR(rc);

    *ppRSAMemData = pRSAMemData;


cleanup:

    return rc;

error:

    rsa_cleanup_mem_data(pRSAMemData);
    if (ppRSAMemData)
    {
        *ppRSAMemData = NULL;
    }


    goto cleanup;
}
