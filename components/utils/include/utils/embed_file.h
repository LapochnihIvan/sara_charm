#ifndef SARA_CHARM_UTILS_EMBED_FILE_H
#define SARA_CHARM_UTILS_EMBED_FILE_H


#include <stdint.h>
#include <stddef.h>


#define EXTERN_EMBED_FILE(name, format)                                 \
    extern const uint8_t embed_file_ ## name ## _ ## format ## _begin[] \
        asm("_binary_" #name "_" #format "_start");                     \
    extern const uint8_t embed_file_ ## name ## _ ## format ## _end[]   \
        asm("_binary_" #name "_" #format "_end")

#define EXTERN_EMBED_ARCHIVED_FILE(name, format, archive_format) \
    EXTERN_EMBED_FILE(name, format ## _ ## archive_format)

#define GET_EMBED_FILE(name, format)                           \
{                                                              \
    .data = embed_file_ ## name ## _ ## format ## _begin,      \
    .len = (size_t)(embed_file_ ## name ## _ ## format ## _end \
        - embed_file_ ## name ## _ ## format ## _begin)        \
}

#define GET_EMBED_ARCHIVED_FILE(name, format, archive_format) \
    GET_EMBED_FILE(name, format ## _ ## archive_format)

typedef struct embed_file
{
    const uint8_t* data;
    size_t len;
} embed_file_t;


#endif //!SARA_CHARM_UTILS_EMBED_FILE_H
