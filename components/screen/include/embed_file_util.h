#ifndef SARA_CHARM_SCREEN_EMBED_FILE_UTIL_H
#define SARA_CHARM_SCREEN_EMBED_FILE_UTIL_H


#include <stdint.h>
#include <stddef.h>


#define EXTERN_EMBED_FILE(name, format)                                 \
    extern const uint8_t embed_file_ ## name ## _ ## format ## _begin[] \
        asm("_binary_" #name "_" #format "_start");                     \
    extern const uint8_t embed_file_ ## name ## _ ## format ## _end[]   \
        asm("_binary_" #name "_" #format "_end")
#define GET_EMBED_FILE(name, format)                           \
{                                                              \
    .begin = embed_file_ ## name ## _ ## format ## _begin,     \
    .len = (size_t)(embed_file_ ## name ## _ ## format ## _end \
        - embed_file_ ## name ## _ ## format ## _begin)        \
}

typedef struct embed_file
{
    const uint8_t* begin;
    size_t len;
} embed_file_t;

#endif //!SARA_CHARM_SCREEN_EMBED_FILE_UTIL_H
