/*
 * encoder.h
 *
 *  Created on: 30-Jan-2025
 *      Author: 14169
 */

#ifndef ENCODER_H_
#define ENCODER_H_
/*************************** HEADER FILES ***************************/
#include <stddef.h>
/****************************** MACROS ******************************/
#define SHA1_BLOCK_SIZE 20              // SHA1 outputs a 20 byte digest

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTES;             // 8-bit byte
typedef unsigned int  WORDS;             // 32-bit word, change to "long" for 16-bit machines

typedef struct {
    BYTES data[64];
    WORDS datalen;
    unsigned long long bitlen;
    WORDS state[5];
    WORDS k[4];
} SHA1_CTX;

/****************************** MACROS ******************************/
#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

size_t b64_encoded_size(size_t inlen);
char *b64_encode(const unsigned char *in, size_t len, char* out, size_t elen);
void sha1_init(SHA1_CTX *ctx);
void sha1_update(SHA1_CTX *ctx, const BYTES data[], size_t len);
void sha1_final(SHA1_CTX *ctx, BYTES hash[]);

#endif /* ENCODER_H_ */
