
#include "includes.h"

static int rsa_buffer_can_write_to_buffer(pRSA_Byte_Stream pByteStream, size_t szDataLen);
static int rsa_buffer_can_read_from_buffer( pRSA_Byte_Stream pByteStream, size_t szDataLen);
static int rsa_is_big_endian(void);
static int rsa_ntohll(PBYTE pNetData, PBYTE pHostData);


int
rsa_buffer_init_buffer(size_t szMaxSize, pRSA_Byte_Stream *ppByteStream)
{
    int rc = NO_ERROR;
    pRSA_Byte_Stream pByteStream = NULL;

    if (!ppByteStream || szMaxSize < 1 || szMaxSize > RSA_MAX_BUFFER_LENGTH) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    pByteStream = malloc(sizeof(RSA_Byte_Stream));
    BAIL_IF_ERROR(!pByteStream, PTR_ERROR, rc);
    pByteStream->pBuffer = malloc(sizeof(BYTE) * szMaxSize);
    BAIL_IF_ERROR(!pByteStream->pBuffer, PTR_ERROR, rc);
    pByteStream->szMaxSize  = szMaxSize;
    pByteStream->szWriteCur = 0;
    pByteStream->szReadCur  = 0;

    *ppByteStream = pByteStream;


cleanup:

    return rc;

error:

    rsa_buffer_cleanup_buffer(pByteStream);
    if (ppByteStream) {
        *ppByteStream = NULL;
    }
    goto cleanup;
}

void
rsa_buffer_cleanup_buffer(pRSA_Byte_Stream pByteStream)
{
    if (pByteStream) {
        if (pByteStream->pBuffer) {
            SAFE_FREE(pByteStream->pBuffer);
            pByteStream->pBuffer = NULL;
        }
        SAFE_FREE(pByteStream);
        pByteStream = NULL;
    }
}

int
rsa_buffer_serialize_uint64(uint64_t unData, pRSA_Byte_Stream pByteStream)
{
    int rc = NO_ERROR;
    uint64_t unHtoNsData = 0;
    uint64_t *punCursor = NULL;

    if (!pByteStream) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_write_to_buffer(pByteStream, sizeof(uint64_t))) {
        rc = COM_ERROR;
        printf("ERROR: can't write uint16_t to buffer\n");
        BAIL_ERROR(rc);
    }

    punCursor = (uint64_t *)(pByteStream->pBuffer + pByteStream->szWriteCur);
    rc = rsa_ntohll((PBYTE)&unData, (PBYTE)&unHtoNsData);
    BAIL_ERROR(rc);
    if (!memcpy((void *)punCursor, (void *)&unHtoNsData, sizeof(uint64_t))) {
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
rsa_buffer_serialize_blob(PBYTE pData, uint64_t unSize, pRSA_Byte_Stream pByteStream)
{
    int rc = NO_ERROR;
    PBYTE pCursor = NULL;

    if (!pByteStream || !pData || unSize < 0) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_write_to_buffer(pByteStream, unSize)) {
        rc = COM_ERROR;
        printf("ERROR: can't write %" PRIu64 " size blob to buffer\n", unSize);
        BAIL_ERROR(rc);
    }

    pCursor = pByteStream->pBuffer + pByteStream->szWriteCur;
    if (!memcpy((void *)pCursor, pData, unSize)) {
        rc = COM_ERROR;
        printf("ERROR: failed memcpy %" PRIu64 " size blob to pByteStream->pBuffer\n", unSize);
        BAIL_ERROR(rc);
    }
    pByteStream->szWriteCur += unSize;


cleanup:

    return rc;

error:

    goto cleanup;
}

int
rsa_buffer_deserialize_uint64(pRSA_Byte_Stream pByteStream, uint64_t *punData)
{
    int rc = 0;
    uint64_t *punCursor = NULL;
    uint64_t uNsToHData = 0;
    uint64_t unData = 0;

    if (!pByteStream || !punData) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_read_from_buffer(pByteStream, sizeof(uint64_t))) {
        rc = COM_ERROR;
        printf("ERROR: can't read uint64_t from buffer\n");
        BAIL_ERROR(rc);
    }

    punCursor = (uint64_t *)(pByteStream->pBuffer + pByteStream->szReadCur);
    if (!memcpy((void *)&uNsToHData, (void *)punCursor, sizeof(uint64_t))) {
        rc = COM_ERROR;
        printf("ERROR: failed memcpy uint64_t from pByteStream->pBuffer\n");
        BAIL_ERROR(rc);
    }
    pByteStream->szReadCur += sizeof(uint64_t);
    rc = rsa_ntohll((PBYTE)&uNsToHData, (PBYTE)&unData);
    BAIL_ERROR(rc);

    *punData = unData;


cleanup:

    return rc;

error:

    if (punData) {
        *punData = 0;
    }

    goto cleanup;
}

int
rsa_buffer_deserialize_blob(pRSA_Byte_Stream pByteStream, uint64_t unSize, PBYTE *ppData)
{
    int rc = 0;
    PBYTE pCursor = NULL;
    PBYTE pData = NULL;

    if (!pByteStream || !ppData || unSize < 0) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_buffer_can_read_from_buffer(pByteStream, unSize)) {
        rc = COM_ERROR;
        printf("ERROR: can't read %" PRIu64 " sized blob from buffer\n", unSize);
        BAIL_ERROR(rc);
    }

    pData = (PBYTE)malloc(unSize);
    BAIL_IF_ERROR(!pData, PTR_ERROR, rc);
    pCursor = pByteStream->pBuffer + pByteStream->szReadCur;
    if (!memcpy((void *)pData, (void *)pCursor, unSize)) {
        rc = COM_ERROR;
        printf("ERROR: failed memcpy %" PRIu64 " sized blob from pByteStream->pBuffer\n", unSize);
        BAIL_ERROR(rc);
    }
    pByteStream->szReadCur += unSize;

    *ppData = pData;


cleanup:

    return rc;

error:

    if (ppData) {
        *ppData = 0;
    }

    goto cleanup;
}

int
rsa_buffer_write_to_socket(int sockfd, pRSA_Byte_Stream pByteStream, int *piBytesSent)
{
    int rc = NO_ERROR;
    int iTotalSent = 0;
    int iBytesLeft = 0;
    int iSent = 0;

    if (sockfd < 0 || !pByteStream || !piBytesSent) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    iBytesLeft = pByteStream->szWriteCur;
    while (iTotalSent < pByteStream->szWriteCur) {
        iSent = send(sockfd, pByteStream->pBuffer + iTotalSent, iBytesLeft, RSA_NO_FLAGS);
        if (iSent == -1) {
            break;
        }
        iTotalSent += iSent;
        iBytesLeft -= iSent;
    }
    BAIL_IF_ERROR(iSent == -1, COM_ERROR, rc);


cleanup:

    *piBytesSent = iTotalSent;
    return rc;

error:

    goto cleanup;
}

int
rsa_buffer_read_from_socket(int sockfd, pRSA_Byte_Stream pByteStream, int *piBytesRead)
{
    int rc = NO_ERROR;
    int iBytesRead = 0;

    if (sockfd < 0 || !pByteStream || !piBytesRead) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    iBytesRead = recv(sockfd, pByteStream->pBuffer, pByteStream->szMaxSize, RSA_NO_FLAGS);
    BAIL_IF_ERROR(iBytesRead == -1, COM_ERROR, rc);
    pByteStream->szWriteCur = iBytesRead;


cleanup:

    *piBytesRead = iBytesRead;
    return rc;

error:

    goto cleanup;
}


static int
rsa_buffer_can_write_to_buffer(pRSA_Byte_Stream pByteStream, size_t szDataLen)
{
    return ((pByteStream->szWriteCur + szDataLen) <= pByteStream->szMaxSize);
}

static int
rsa_buffer_can_read_from_buffer(pRSA_Byte_Stream pByteStream, size_t szDataLen)
{
    return ((pByteStream->szReadCur + szDataLen) <= pByteStream->szMaxSize);
}

static int
rsa_is_big_endian(void)
{
    union {
        uint32_t    unData;
        char        cData[4];
    } EndianDetect = {0x01020304};

    return EndianDetect.cData[0] == 1;
}

static int
rsa_ntohll(PBYTE pIn, PBYTE pOut)
{
    int rc = NO_ERROR;
    size_t szSize = sizeof(uint64_t);
    unsigned uInIdx = 0;
    unsigned uOutIdx = 0;

    if (!pIn || !pOut) {
        rc = ARG_ERROR;
        BAIL_ERROR(rc);
    }

    if (!rsa_is_big_endian()) {
        for (uInIdx = szSize; uOutIdx < szSize && uInIdx--; ++uOutIdx) {
            pOut[uOutIdx] = pIn[uInIdx];
        }
    } else {
        if (!memcpy(pOut, pIn, szSize)) {
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
