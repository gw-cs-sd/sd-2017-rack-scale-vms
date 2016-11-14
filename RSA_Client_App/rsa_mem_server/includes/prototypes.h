
/* rsa_buffer_utils.c */

int
rsa_buffer_init_buffer(
    size_t  szMaxSize,
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
rsa_buffer_deserialize_uint64(
    pRSA_Byte_Stream    pByteStream,
    uint64_t*           punData
    );

int
rsa_buffer_write_to_socket(
    int                 sockfd,
    pRSA_Byte_Stream    pByteStream
    );

int
rsa_buffer_read_from_socket(
    int                 sockfd,
    pRSA_Byte_Stream    pByteStream
    );

/* rsa_network_helper.c */

int
rsa_init_mem_data(
    uint64_t        unPhysAddr,
    uint64_t        unVirtAddr,
    pRSA_Mem_Data*  ppRSAMemData
    );

void
rsa_cleanup_mem_data(
    pRSA_Mem_Data   pRSAMemData
    );

int
rsa_serialize_mem_data(
    pRSA_Mem_Data       pRSAMemData,
    pRSA_Byte_Stream    pByteStream
    );

int
rsa_deserialize_mem_data(
    pRSA_Byte_Stream    pByteStream,
    pRSA_Mem_Data*      ppRSAMemData
    );

int
rsa_download_more_ram(
    pRSA_Mem_Data   pRSAMemData,
    char**          ppszRetPage
    );

