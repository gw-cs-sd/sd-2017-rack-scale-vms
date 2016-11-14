/*
 * This is a simple client which tests the remote memory server.
 *
 * It creates a dummy RSA_Mem_Data struct filled with dummy data.  It then
 * serializes the data and sends it over a socket to the server requesting
 * "more ram" to which the server mallocs some data and sends it back.
 */

#include "includes.h"

int main(void)
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pRSAMemData = NULL;
    char* pszRetPage = NULL;

    pRSAMemData = malloc(sizeof(RSA_Mem_Data));
    BAIL_PTR_ERROR(pRSAMemData, rc);

    pRSAMemData->unPhysAddr = 401101824;
    pRSAMemData->unVirtAddr = 401101124;

    rc = rsa_download_more_ram(
                    pRSAMemData,
                    &pszRetPage
                    );
    BAIL_ERROR(rc);

    printf("SUCCESS: %s\n", pszRetPage);


cleanup:
    SAFE_FREE(pRSAMemData);

    return rc;

error:

    goto cleanup;
}
