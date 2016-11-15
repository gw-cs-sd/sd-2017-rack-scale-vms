
typedef struct _RSA_Byte_Stream
{
    PBYTE   pBuffer;
    size_t  szWriteCur;     // Where to write next and size of buffer
    size_t  szReadCur;      // How much have we read?
    size_t  szMaxSize;
} RSA_Byte_Stream, * pRSA_Byte_Stream;

typedef struct _RSA_Mem_Data
{
    /*
     * TODO:
     *
     * const char*     pszDomain;
     *
     * const char* pszDomain is a field that will be used in the future.
     * Once libbdvmi is modified to pass the driver to an event handler,
     * and once the domain name is given to the event handler, this field
     * will be needed to send this data to the RSA_App companion server.
     */

    uint64_t        unPhysAddr;
    uint64_t        unVirtAddr;

    /*
     * TODO:
     *
     * void*           pPageData;
     *
     * void* pPageData is a field that contains data sent from rsa_app to the
     * mem server or data sent from the mem server to the rsa_app.
     */
} RSA_Mem_Data, * pRSA_Mem_Data;
