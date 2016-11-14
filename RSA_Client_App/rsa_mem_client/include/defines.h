#ifndef _RSA_CLIENT_H_
#define _RSA_CLIENT_H_

#define DEBUG 1

#define MAX_BUFFER_LENGTH (~(size_t)0)

#define BACKLOG 10
#define PORT "2222"
#define IP_ADDR "192.168.1.139"
#define NET_FLAGS 0

typedef char BYTE, * PBYTE;

typedef enum
{
    FALSE = 0,
    TRUE = 1
} BOOL;

typedef enum
{
    NO_ERROR = 0,
    ARG_ERROR = -100,
    PTR_ERROR = -200,
    COM_ERROR = -300
} ERR_CODES;

#define BAIL_ERROR(err)                             \
    if (err) {                                      \
        printf("ERROR! | [%s,%d] err_code = %d\n",  \
               __FUNCTION__,                        \
               __LINE__,                            \
               err                                  \
               );                                   \
        goto error;                                 \
    }

#define BAIL_ERROR_IF(cond)                         \
    if (cond) {                                     \
        printf("ERROR! | [%s,%d] err_code = %d\n",  \
               __FUNCTION__,                        \
               __LINE__,                            \
               rc                                   \
               );                                   \
        goto error;                                 \
    }

#define BAIL_PERROR(err)                            \
    if (err) {                                      \
        printf("ERROR! | [%s,%d] err_code = %d\n",  \
               __FUNCTION__,                        \
               __LINE__,                            \
               err                                  \
               );                                   \
        goto error;                                 \
    }

#define BAIL_PTR_ERROR(p, err)  \
    if (p == NULL) {            \
        err = PTR_ERROR;        \
        BAIL_ERROR(err);        \
    }

#define SAFE_FREE(ptr)      \
    do {                    \
        if ((ptr)) {        \
            free(ptr);      \
            (ptr) = NULL;   \
        }                   \
    } while (0)

#endif /* _RSA_CLIENT_H_ */
