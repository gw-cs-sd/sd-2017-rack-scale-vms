/*
 * Title: rsa_test_client.c
 * Author: Neel Shah
 * Data: 04/11/2017
 * Abstract:
 *     This is a simple client which tests the remote memory server.
 *     It creates a dummy RSA_Mem_Data struct filled with dummy data.  It then
 *     serializes the data and sends it over a socket to the memserver, which
 *     stores the data into a hash table, and sends back a response.
 */

#include "../RSApi/includes.h"

int
main(void)
{
    int rc = NO_ERROR;
    pRSA_Mem_Data pRecvMemData = NULL;
    char* dummy = "Who are you?";
    RSA_Mem_Data mdSendMemData =
        {
            .unVirtAddr = 0x04030201,
            .pPageData = dummy
        };

    printf("INFO: Sending dummy Page Data...\n");
    rc = rsa_download_more_ram(&mdSendMemData, &pRecvMemData);
    BAIL_ERROR(rc);
    printf("INFO: Received response Page Data: ");
    printf("\tPage: %" PRIu64 "\n", pRecvMemData->unVirtAddr);
    printf("\tPageData: %s\n", (char *)pRecvMemData->pPageData);

cleanup:
    rsa_cleanup_mem_data(pRecvMemData);

    printf("INFO: Exiting rsa_test_client\n");
    return rc;

error:

    goto cleanup;
}
