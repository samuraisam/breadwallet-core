//
//  BRHash.h
//
//  Created by Aaron Voisine on 8/8/15.
//  Copyright (c) 2015 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BRHash_h
#define BRHash_h

#include <stddef.h>

#if !defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)
#error endianess is unkown
#endif

#if __BIG_ENDIAN__
#define be16(x) (x)
#define le16(x) ((((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8))
#define be32(x) (x)
#define le32(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#define be64(x) (x)
#define le64(x) ((((x) & 0x00000000000000ffULL) << 56) | (((x) & 0xff00000000000000ULL) >> 56) |\
                 (((x) & 0x000000000000ff00ULL) << 40) | (((x) & 0x00ff000000000000ULL) >> 40) |\
                 (((x) & 0x0000000000ff0000ULL) << 24) | (((x) & 0x0000ff0000000000ULL) >> 24) |\
                 (((x) & 0x00000000ff000000ULL) << 8)  | (((x) & 0x000000ff00000000ULL) >> 8))
#else
#define be16(x) ((((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8))
#define le16(x) (x)
#define be32(x) ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#define le32(x) (x)
#define be64(x) ((((x) & 0x00000000000000ffULL) << 56) | (((x) & 0xff00000000000000ULL) >> 56) |\
                 (((x) & 0x000000000000ff00ULL) << 40) | (((x) & 0x00ff000000000000ULL) >> 40) |\
                 (((x) & 0x0000000000ff0000ULL) << 24) | (((x) & 0x0000ff0000000000ULL) >> 24) |\
                 (((x) & 0x00000000ff000000ULL) << 8)  | (((x) & 0x000000ff00000000ULL) >> 8))
#define le64(x) (x)
#endif

#define RMD160_DIGEST_LENGTH (160/8)

void BRSHA1(void *md, const void *data, size_t len);

void BRSHA256(void *md, const void *data, size_t len);

void BRSHA256_2(void *md, const void *data, size_t len);

void BRSHA512(void *md, const void *data, size_t len);

// ripemd-160 hash function: http://homes.esat.kuleuven.be/~bosselae/ripemd160.html
void BRRMD160(void *md, const void *data, size_t len);

void BRHash160(void *md, const void *data, size_t len);

void BRHMAC(void *md, void (*hash)(void *, const void *, size_t), size_t hlen, const void *key, size_t klen,
            const void *data, size_t dlen);

void BRPBKDF2(void *dk, size_t dklen, void (*hash)(void *, const void *, size_t), size_t hlen,
              const void *pw, size_t pwlen, const void *salt, size_t slen, unsigned rounds);

// murmurHash3 (x86_32): https://code.google.com/p/smhasher/
uint32_t BRMurmur3_32(const void *data, size_t len, uint32_t seed);

#endif // BRHash_h
