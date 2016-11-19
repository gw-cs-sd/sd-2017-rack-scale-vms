
#include "includes.h"

static
int
rsa_buffer_can_write_to_buffer(
    pRSA_Byte_Stream    pByteStream,
    size_t              szDataLen
    );

static
int
rsa_buffer_can_read_from_buffer(
    pRSA_Byte_Stream    pByteStream,
    size_t              szDataLen
    );

static
int
rsa_is_big_endian(
    void
    );

static
int
rsa_ntohll(
    PBYTE   pNetData,
    PBYTE   pHostData
    );


int
rsa_buffer_init_buffer(
    size_t  szMaxSize,
    pRSA_Byte_Stream*   ppByteStream
    )
{
    int rc = NO_ERROR;
    pRSA_Byte_Stream pByteStream = NULL;

    if (!ppByteStream ||
        !szMaxSize ||
        szMaxSize > MAX_BUFFER_LENGTH)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pByteStream = malloc(sizeof(RSA_Byte_Stream));
    BAIL_PTR_ERROR(pByteStream, rc);

    pByteStream->pBuffer = malloc(sizeof(BYTE) * szMaxSize);
    BAIL_PTR_ERROR(pByteStream->pBuffer, rc);

    pByteStream->szMaxSize = szMaxSize;
    pByteStream->szWriteCur = 0;
    pByteStream->szReadCur = 0;

    *ppByteStream = pByteStream;

cleanup:

    return rc;

error:

    rsa_buffer_cleanup_buffer(pByteStream);
    if (ppByteStream)
    {
        *ppByteStream = NULL;
    }
    goto cleanup;
}

void
rsa_buffer_cleanup_buffer(
    pRSA_Byte_Stream    pByteStream
    )
{
    if (pByteStream)
    {
        if (pByteStream->pBuffer)
        {
            SAFE_FREE(pByteStream->pBuffer);
            pByteStream->pBuffer = NULL;
        }

        SAFE_FREE(pByteStream);
        pByteStream = NULL;
    }
}

int
rsa_buffer_serialize_uint64(
    uint64_t            unData,
    pRSA_Byte_Stream    pByteStream
    )
{
    int rc = NO_ERROR;
    uint64_t unHtoNsData = 0;
    uint64_t* punCursor = NULL;

    if (!pByteStream)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_write_to_buffer(
                    pByteStream,
                    sizeof(uint64_t)))
    {
        rc = COM_ERROR;
        printf("ERROR: can't write uint16_t to buffer\n");
        BAIL_ERROR(rc);
    }

    punCursor = (uint64_t *)(pByteStream->pBuffer + pByteStream->szWriteCur);

    rc = rsa_ntohll(
            (PBYTE)&unData,
            (PBYTE)&unHtoNsData
            );
    BAIL_ERROR(rc);

    if (!memcpy(
            (void *)punCursor,
            (void *)&unHtoNsData,
            sizeof(uint64_t)))
    {
        rc = COM_ERROR;
        printf("ERROR: failed memcpy uint64_t to pByteStream->pBuffer\n");
        BAIL_ERROR(rc);
    }

    pByteStream->szWriteCur += sizeof(uint64_t);


cleanup:

    return rc;

error:

    goto cleanup;
}

int
rsa_buffer_deserialize_uint64(
    pRSA_Byte_Stream    pByteStream,
    uint64_t*           punData
    )
{
    int rc = 0;
    uint64_t* punCursor = NULL;
    uint64_t uNsToHData = 0;
    uint64_t unData = 0;

    if (!pByteStream || !punData)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_read_from_buffer(
                        pByteStream,
                        sizeof(uint64_t)))
    {
        rc = COM_ERROR;
        printf("ERROR: can't read uint64_t from buffer\n");
        BAIL_ERROR(rc);
    }

    punCursor = (uint64_t *)(pByteStream->pBuffer + pByteStream->szReadCur);

    if (!memcpy(
            (void *)&uNsToHData,
            (void *)punCursor,
            sizeof(uint64_t)))
    {
        rc = COM_ERROR;
        printf("ERROR: failed memcpy uint64_t from pByteStream->pBuffer\n");
        BAIL_ERROR(rc);
    }

    pByteStream->szReadCur += sizeof(uint64_t);

    rc = rsa_ntohll(
            (PBYTE)&uNsToHData,
            (PBYTE)&unData
            );
    BAIL_ERROR(rc);

    *punData = unData;


cleanup:

    return rc;

error:

    if (punData)
    {
        *punData = 0;
    }

    goto cleanup;
}

int
rsa_buffer_write_to_socket(
    int                 sockfd,
    pRSA_Byte_Stream    pByteStream
    )
{
    int rc = NO_ERROR;

    if (sockfd < 0 || !pByteStream)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    rc = send(
            sockfd,
            pByteStream->pBuffer,
            pByteStream->szWriteCur,
            NET_FLAGS
            );
    BAIL_ERROR_IF(rc && rc == -1);

    if (rc && rc != -1)
    {
        rc = NO_ERROR;
    }


cleanup:

    return rc;

error:

    goto cleanup;
}

int
rsa_buffer_read_from_socket(
    int sockfd,
    pRSA_Byte_Stream    pByteStream
    )
{
    int rc = NO_ERROR;
    int bytes_read = 0;

    if (sockfd < 0 || !pByteStream)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    bytes_read = read(
                    sockfd,
                    pByteStream->pBuffer,
                    200
                    );
    if (bytes_read == -1)
    {
        rc = COM_ERROR;
        printf("ERROR: could not read from socket\n");
        BAIL_ERROR(rc);
    }

    pByteStream->szWriteCur = bytes_read;


cleanup:

    return rc;

error:

    goto cleanup;
}


static
int
rsa_buffer_can_write_to_buffer(
    pRSA_Byte_Stream    pByteStream,
    size_t              szDataLen
    )
{
    return ((pByteStream->szWriteCur + szDataLen) <= pByteStream->szMaxSize);
}

static
int
rsa_buffer_can_read_from_buffer(
    pRSA_Byte_Stream    pByteStream,
    size_t              szDataLen
    )
{
    return ((pByteStream->szReadCur + szDataLen) <= pByteStream->szMaxSize);
}

static
int
rsa_is_big_endian(
    void
    )
{
    union {
        uint32_t    unData;
        char        cData[4];
    } EndianDetect = {0x01020304};


    return EndianDetect.cData[0] == 1;
}

static
int
rsa_ntohll(
    PBYTE   pIn,
    PBYTE   pOut
    )
{
    int rc = NO_ERROR;
    size_t szSize = sizeof(uint64_t);
    unsigned uInIdx = 0;
    unsigned uOutIdx = 0;

    if (!pIn || !pOut)
    {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_is_big_endian())
    {
        for (uInIdx = szSize; uOutIdx < szSize && uInIdx--; ++uOutIdx)
        {
            pOut[uOutIdx] = pIn[uInIdx];
        }
    }
    else
    {
        if (!memcpy(
                pOut,
                pIn,
                szSize))
        {
            rc = COM_ERROR;
            printf("ERROR: memcpy rsa_ntohll\n");
            BAIL_ERROR(rc);
        }
    }


cleanup:

    return rc;

error:

    goto cleanup;

}
