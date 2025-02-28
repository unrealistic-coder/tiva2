#ifndef ENCODERS_H_
#define ENCODERS_H_

#include <stddef.h>
/****************************** MACROS ******************************/
#define SHA1_BLOCK_SIZE 20              // SHA1 outputs a 20 byte digest (default)
#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

/**************************** DATA TYPES ****************************/
typedef unsigned char BYTES;             // 8-bit byte
typedef unsigned int  WORDS;             // 32-bit WORDS, change to "long" for 16-bit machines

/**************************** STRUCTS ****************************/
typedef struct {
    BYTES data[64];
    WORDS datalen;
    unsigned long long bitlen;
    WORDS state[5];
    WORDS k[4];
} SHA1_CTX;
/*********************** FUNCTION DECLARATIONS **********************/

/* BASE64 FUCTIONS */
size_t b64_encoded_size(size_t inlen);
char *b64_encode(const unsigned char *in, size_t len, char* out, size_t elen);

/* SHA1 FUCTIONS */
void sha1_transform(SHA1_CTX *ctx, const BYTES data[]);
void sha1_init(SHA1_CTX *ctx);
void sha1_update(SHA1_CTX *ctx, const BYTES data[], size_t len);
void sha1_final(SHA1_CTX *ctx, BYTES hash[]);

#endif
