
#include "includes.h"

int
rsa_init_mem_data(
    uint64_t            unVirtAddr,
    PBYTE               pData,
    pRSA_Mem_Data*      ppMemData
    )
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pMemData = NULL;

    if (!ppMemData || !pData || unVirtAddr < 0)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pMemData = malloc(sizeof(RSA_Mem_Data));
    BAIL_IF_ERROR(!pMemData, PTR_ERROR, rc);
    pMemData->pPageData = malloc(PAGESIZE);
    BAIL_IF_ERROR(!pMemData->pPageData, PTR_ERROR, rc);

    pMemData->unVirtAddr = unVirtAddr;
    if (!memcpy(pMemData->pPageData,
                pData,
                PAGESIZE))
    {
        rc = COM_ERROR;
        RSA_LOG("[RSApi ERROR] failed memcpy on pMemData->pPageData\n");
        BAIL_ERROR(rc);
    }

    *ppMemData = pMemData;


cleanup:

    return rc;

error:

    rsa_cleanup_mem_data(pMemData);
    if (ppMemData)
    {
        *ppMemData = NULL;
    }

    goto cleanup;
}

void
rsa_cleanup_mem_data(
    pRSA_Mem_Data       pMemData
    )
{
    if (pMemData)
    {
        if (pMemData->pPageData)
        {
            SAFE_FREE(pMemData->pPageData);
            pMemData->pPageData = NULL;
        }

        SAFE_FREE(pMemData);
        pMemData = NULL;
    }
}

int
rsa_download_more_ram(
    pRSA_Mem_Data       pMemData,
    pRSA_Mem_Data*      ppRetData
    )
{
    int rc = NO_ERROR;
    struct addrinfo* pServerInfo = NULL;
    int iSockFd = 0;
    pRSA_Byte_Stream pSendStream = NULL;
    pRSA_Byte_Stream pRecvStream = NULL;
    int temp;
    pRSA_Mem_Data pRetData = NULL;

    BAIL_IF_ERROR((!ppRetData || !pMemData), ARG_ERROR, rc);

    rc = rsa_net_create_socket(
                RSA_NO_FLAGS,
                &pServerInfo,
                &iSockFd
                );
    BAIL_ERROR(rc);

    rc = rsa_net_connect_socket(
                RSA_PROC_CLIENT,
                pServerInfo,
                iSockFd
                );
    BAIL_ERROR(rc);

    rc = rsa_buffer_init_buffer(
                    PAGESIZE + sizeof(uint64_t),
                    &pSendStream
                    );
    BAIL_ERROR(rc);

    rc = rsa_buffer_init_buffer(
                    PAGESIZE + sizeof(uint64_t),
                    &pRecvStream
                    );
    BAIL_ERROR(rc);

    rc = rsa_serialize_mem_data(
                    pMemData,
                    pSendStream
                    );
    BAIL_ERROR(rc);

    rc = rsa_buffer_write_to_socket(
                        iSockFd,
                        pSendStream,
                        &temp
                        );
    BAIL_IF_ERROR(pSendStream->szWriteCur != temp, COM_ERROR, rc);

    rc = rsa_buffer_read_from_socket(
                        iSockFd,
                        pRecvStream,
                        &temp
                        );
    BAIL_ERROR(rc);

    rc = rsa_deserialize_mem_data(
                    pRecvStream,
                    &pRetData
                    );
    BAIL_ERROR(rc);

    *ppRetData = pRetData;


cleanup:
    freeaddrinfo(pServerInfo);
    close(iSockFd);
    rsa_buffer_cleanup_buffer(pSendStream);
    rsa_buffer_cleanup_buffer(pRecvStream);

    return 0;

error:

    rsa_cleanup_mem_data(pRetData);
    if (ppRetData)
    {
        *ppRetData = NULL;
    }

    goto cleanup;
}

int
rsa_serialize_mem_data(
    pRSA_Mem_Data       pMemData,
    pRSA_Byte_Stream    pByteStream
    )
{
    int rc = NO_ERROR;

    if (!pMemData || !pByteStream)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    rc = rsa_buffer_serialize_uint64(
                        pMemData->unVirtAddr,
                        pByteStream
                        );
    BAIL_ERROR(rc);

    rc = rsa_buffer_serialize_blob(
                    pMemData->pPageData,
                    PAGESIZE,
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
    pRSA_Mem_Data*      ppMemData
    )
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pMemData = NULL;

    if (!pByteStream || !ppMemData)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pMemData = malloc(sizeof(RSA_Mem_Data));
    BAIL_IF_ERROR(!pMemData, PTR_ERROR, rc);

    rc = rsa_buffer_deserialize_uint64(
                        pByteStream,
                        &pMemData->unVirtAddr
                        );
    BAIL_ERROR(rc);

    pMemData->pPageData = malloc(PAGESIZE);
    BAIL_IF_ERROR(!pMemData->pPageData, PTR_ERROR, rc);

    rc = rsa_buffer_deserialize_blob(
                        pByteStream,
                        PAGESIZE,
                        &pMemData->pPageData
                        );
    BAIL_ERROR(rc);

    *ppMemData = pMemData;


cleanup:

    return rc;

error:

    rsa_cleanup_mem_data(pMemData);
    if (ppMemData)
    {
        *ppMemData = NULL;
    }


    goto cleanup;
}
