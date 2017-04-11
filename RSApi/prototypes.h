
/* rsa_buffer_utils.c */

int
rsa_buffer_init_buffer(
    size_t              szMaxSize,
    pRSA_Byte_Stream*   ppBuffer
    );

void
rsa_buffer_cleanup_buffer(
    pRSA_Byte_Stream    pBuffer
    );

int
rsa_buffer_serialize_uint64(
    uint64_t            data,
    pRSA_Byte_Stream    pByteStream
    );

int
rsa_buffer_serialize_blob(
    PBYTE               pData,
    uint64_t            unSize,
    pRSA_Byte_Stream    pByteStream
    );

int
rsa_buffer_deserialize_uint64(
    pRSA_Byte_Stream    pByteStream,
    uint64_t*           punData
    );

int
rsa_buffer_deserialize_blob(
    pRSA_Byte_Stream    pByteStream,
    uint64_t            unSize,
    PBYTE*              ppData
    );

int
rsa_buffer_write_to_socket(
    int                 sockfd,
    pRSA_Byte_Stream    pByteStream,
    int*                piBytesSent
    );

int
rsa_buffer_read_from_socket(
    int                 sockfd,
    pRSA_Byte_Stream    pByteStream,
    int*                piBytesRead
    );

/* rsa_network_helper.c */

int
rsa_net_create_socket(
    int                 iSockFlags,
    struct addrinfo**   ppServerInfo,
    int*                pSockFd
    );

int
rsa_net_connect_socket(
    RSA_Proc_t          iProcType,
    struct addrinfo*    pServerInfo,
    int                 iSockFd
    );

/* rsa_api.c */

int
rsa_init_mem_data(
    uint64_t            unVirtAddr,
    PBYTE               pData,
    pRSA_Mem_Data*      ppMemData
    );

void
rsa_cleanup_mem_data(
    pRSA_Mem_Data       pMemData
    );

int
rsa_serialize_mem_data(
    pRSA_Mem_Data       pMemData,
    pRSA_Byte_Stream    pByteStream
    );

int
rsa_deserialize_mem_data(
    pRSA_Byte_Stream    pByteStream,
    pRSA_Mem_Data*      ppMemData
    );

int
rsa_download_more_ram(
    pRSA_Mem_Data       pMemData,
    pRSA_Mem_Data*      ppRetData
    );

int
rsa_upload_some_ram(
    pRSA_Mem_Data       pMemData
    );

