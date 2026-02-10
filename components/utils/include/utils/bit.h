#ifndef SARA_CHARM_UTILITY_BIT_H
#define SARA_CHARM_UTILITY_BIT_H


#include <stdint.h>
#include <limits.h>


#define BITS_IN_BYTE (CHAR_BIT)

inline uint16_t swap_bytes(uint16_t num);

inline uint16_t swap_bytes(const uint16_t num)
{
    return ((num & 0xFF) << BITS_IN_BYTE) | (num >> BITS_IN_BYTE);
}


#endif //!SARA_CHARM_UTILITY_BIT_H
