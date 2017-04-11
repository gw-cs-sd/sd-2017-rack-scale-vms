#ifndef _RSA_SERVER_H_
#define _RSA_SERVER_H_

#define DEBUG 1

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#ifndef RSA_MAX_BUFFER_LENGTH
#define RSA_MAX_BUFFER_LENGTH (~(size_t)0)
#endif

#ifndef RSA_PORT
#define RSA_PORT "2222"
#endif

#ifndef RSA_IP_ADDR
#define RSA_IP_ADDR "192.168.1.139"
#endif

#ifndef RSA_NO_FLAGS
#define RSA_NO_FLAGS 0
#endif

#ifndef RSA_BACKLOG
#define RSA_BACKLOG 10
#endif

typedef enum
{
    NO_ERROR = 0,
    ARG_ERROR = -100,
    PTR_ERROR = -200,
    COM_ERROR = -300
} ERR_CODES;

typedef char BYTE, * PBYTE;

typedef enum
{
    RSA_PROC_SERVER = 0,
    RSA_PROC_CLIENT
} RSA_Proc_t;

#define RSA_LOG(msg, ...)                               \
    do {                                                \
        if ((msg)) {                                    \
            fprintf(stdout,                             \
                    "[RSApi INFO]: " #msg "\n",         \
                    ##__VA_ARGS__                       \
                    );                                  \
        }                                               \
    } while (0)

#define BAIL_ERROR(err)                                 \
    do {                                                \
        if ((err)) {                                    \
            printf("[RSApi ERROR: %s,%d] "              \
                   "RSA_Err = %d, ERRNO = %m\n",        \
                   __FUNCTION__,                        \
                   __LINE__,                            \
                   (err)                                \
                   );                                   \
            goto error;                                 \
        }                                               \
    } while (0)

#define BAIL_IF_ERROR(cond, type, err)                  \
    do {                                                \
        if ((cond)) {                                   \
            (err) = type;                               \
            BAIL_ERROR((err));                          \
        }                                               \
    } while (0)

#define SAFE_FREE(ptr)                                  \
    do {                                                \
        if ((ptr)) {                                    \
            free((ptr));                                \
            (ptr) = NULL;                               \
        }                                               \
    } while (0)

#endif /* _RSA_SERVER_H_ */
