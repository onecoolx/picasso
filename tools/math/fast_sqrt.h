#ifndef _FAST_SQRT_H_
#define _FAST_SQRT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint16_t g_sqrt_table[1024];
extern int8_t g_elder_bit_table[256];

//Fast integer Sqrt - really fast: no cycles, divisions or multiplications
#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4035) //Disable warning "no return value"
#endif
inline uint32_t fast_sqrt(uint32_t val)
{
#if defined(_M_IX86) && defined(_MSC_VER)
    //For Ix86 family processors this assembler code is used.
    //The key command here is bsr - determination the number of the most
    //significant bit of the value. For other processors
    //(and maybe compilers) the pure C "#else" section is used.
    __asm {
        mov ebx, val
        mov edx, 11
        bsr ecx, ebx
        sub ecx, 9
        jle less_than_9_bits
        shr ecx, 1
        adc ecx, 0
        sub edx, ecx
        shl ecx, 1
        shr ebx, cl
        less_than_9_bits:
        xor eax, eax
        mov ax, g_sqrt_table[ebx*2]
        mov ecx, edx
        shr eax, cl
    }
#else

    //This code is actually pure C and portable to most
    //arcitectures including 64bit ones.
    uint32_t t = val;
    uint32_t shift = 11;
    int bit = 0;

    //The following piece of code is just an emulation of the
    //Ix86 assembler command "bsr" (see above). However on old
    //Intels (like Intel MMX 233MHz) this code is about twice
    //faster (sic!) then just one "bsr". On PIII and PIV the
    //bsr is optimized quite well.
    bit = t >> 24;
    if (bit) {
        bit = g_elder_bit_table[bit] + 24;
    } else {
        bit = (t >> 16) & 0xFF;
        if (bit) {
            bit = g_elder_bit_table[bit] + 16;
        } else {
            bit = (t >> 8) & 0xFF;
            if (bit) {
                bit = g_elder_bit_table[bit] + 8;
            } else {
                bit = g_elder_bit_table[t];
            }
        }
    }

    //This code calculates the sqrt.
    bit -= 9;
    if (bit > 0) {
        bit = (bit >> 1) + (bit & 1);
        shift -= bit;
        val >>= (bit << 1);
    }
    return g_sqrt_table[val] >> shift;
#endif
}
#if defined(_MSC_VER)
    #pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif
#endif /*_FAST_SQRT_H_*/
