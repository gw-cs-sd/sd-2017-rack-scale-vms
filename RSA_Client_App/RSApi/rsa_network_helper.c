
#include "includes.h"

int
rsa_net_create_socket(
    int                 iSockFlags,
    struct addrinfo**   ppServerInfo,
    int*                pSockFd
    )
{
    int rc  = NO_ERROR;
    int iSockFd = 0;
    struct addrinfo* pServerInfo = NULL;
    struct addrinfo aiHints =
        {
            .ai_family      = AF_INET,
            .ai_socktype    = SOCK_STREAM,
            .ai_flags       = iSockFlags
        };

    BAIL_IF_ERROR(!pSockFd, ARG_ERROR, rc);

    rc = getaddrinfo(
            RSA_IP_ADDR,
            RSA_PORT,
            &aiHints,
            &pServerInfo
            );
    BAIL_ERROR(rc);

    iSockFd = socket(
                pServerInfo->ai_family,
                pServerInfo->ai_socktype,
                pServerInfo->ai_protocol
                );
    if (iSockFd == -1)
    {
        rc = COM_ERROR;
        printf("[RSApi ERROR] couldn't create socket\n");
        BAIL_ERROR(rc);
    }

    *ppServerInfo   = pServerInfo;
    *pSockFd        = iSockFd;


cleanup:

    return rc;

error:

    freeaddrinfo(pServerInfo);
    if (ppServerInfo)
    {
        *ppServerInfo = NULL;
    }

    if (pSockFd)
    {
        *pSockFd = 0;
    }

    goto cleanup;
}

int
rsa_net_connect_socket(
    RSA_Proc_t          iProcType,
    struct addrinfo*    pServerInfo,
    int                 iSockFd
    )
{
    int rc = NO_ERROR;

    BAIL_IF_ERROR(
            (!pServerInfo || iSockFd < 1),
            ARG_ERROR,
            rc
            );

    switch(iProcType) {
        case RSA_PROC_SERVER:
            rc = bind(
                    iSockFd,
                    pServerInfo->ai_addr,
                    pServerInfo->ai_addrlen
                    );
            break;
        case RSA_PROC_CLIENT:
            rc = connect(
                    iSockFd,
                    pServerInfo->ai_addr,
                    pServerInfo->ai_addrlen
                    );
            break;
        default:
            rc = COM_ERROR;
    }

    BAIL_ERROR(rc);


cleanup:

    return rc;

error:

    goto cleanup;
}
