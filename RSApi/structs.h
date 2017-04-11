
typedef struct UT_hash_handle UT_hash_handle;

typedef struct _RSA_Byte_Stream
{
    PBYTE           pBuffer;        // Pointer to byte stream
    size_t          szWriteCur;     // Where to write next and size of data in buffer
    size_t          szReadCur;      // How much data have we read?
    size_t          szMaxSize;      // Capacity of buffer
} RSA_Byte_Stream, * pRSA_Byte_Stream;

typedef struct _RSA_Mem_Data
{
    uint64_t        unVirtAddr;     // Virtual Address of Page (key for hashtable)
    PBYTE           pPageData;      // Data From Page.  Assume sz is always 4096 (PAGESIZE)
    //UT_hash_handle  hhHT;           // Handle to make structure "hashable"
} RSA_Mem_Data, * pRSA_Mem_Data;
