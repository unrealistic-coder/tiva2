
/*
 *    ======== tcpEchoServer.c ========
 *    Contains BSD sockets code.
 */
//TODO: C:/ti/tirtos_tivac_2_16_01_14/packages
// removed from general->variables->show sysstem variables->COM_TIRTSC_TIRTOS_REPOS
#include <source/user_diskio.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Startup.h>


#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/drivers/GPIO.h>
#include <ti/sysbios/knl/Clock.h>

/* NDK BSD support */
#include <sys/socket.h>

/* Example/Board Header file */
#include "Board.h"

// C std headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Tiva C Drivers
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "SST26VF032B.h"

//FATFS headers
#include "source/ff.h"

//webpage files
#include "fs/index.h"
#include "fs/styles.h"
#include "fs/script.h"

#define TCPPACKETSIZE 512
#define NUMTCPWORKERS 3
#define BUFFER_SIZE 1024
#define SHA1_HASH_SIZE 20
#define WEBSOCKET_OPCODE_TEXT 0x1

FATFS fs;          // Filesystem object
FIL file;          // File object
static FRESULT res;       // FatFs function result
UINT bw, br;       // Bytes written/read
char buffer[128];  // Buffer for reading/writing

static inline void send_all(int sock, const char* data, size_t datalen) {
    size_t sent = 0;
    while (sent < datalen) {
        int res = send(sock, data + sent, datalen - sent, 0);
        if (res < 0) {
            perror("send failed");
            break;
        }
        sent += res;
    }
}

static inline void send_chunk(int sock, const char* data, size_t datalen) {
    char chunk_size[10];
    snprintf(chunk_size, sizeof(chunk_size), "%X\r\n", (unsigned int)datalen);
    send_all(sock, chunk_size, strlen(chunk_size)); // Send chunk size
    send_all(sock, data, datalen); // Send chunk data
    send_all(sock, "\r\n", 2); // Send CRLF after chunk data
}

// static inline void send_chunk(int sock, const char* data) {
//     char chunk_size[4]; // max 1024 length 1KB
//     snprintf(chunk_size, sizeof(chunk_size),"%X\r\n" ,(unsigned int)strlen(data));
//     send(sock, chunk_size, strlen(chunk_size), 0); // send length of chunk with CRLF
//     send(sock, data, strlen(data), 0); //send data
//     send(sock,"\r\n",2,0);
// }

//fatfs codes
void format_disk(void) {
    FRESULT res;
    MKFS_PARM fs_params = {FM_FAT | FM_SFD, 1, 1, 128, 4096}; // FAT12/16/32
    BYTE work[FF_MAX_SS]; // Work buffer (aligned with sector size)

    res = f_mkfs("", &fs_params, work, sizeof(work));
    if (res == FR_OK) {
        System_printf("Disk formatted successfully.\n");
    } else {
        System_printf("Error formatting disk: %d\n", res);
    }

}

unsigned int write_file(const char* filename, unsigned char* data, unsigned int len){
    res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if(res != FR_OK){
        System_printf("failed to open, %s\n",filename);
        return 0;
    }
    System_printf("File opened successfully for writing.\n");

    res = f_write(&file, data, len, &bw);
    if(res != FR_OK){
        System_printf("failed to write, %s\n",filename);
        return 0;
    }
    System_printf("%s written successfully\n", filename);
//    printf("Data written to file: %s\n", data);
    f_close(&file);
    return bw;
}

unsigned int send_file_web(const char* filename, int sock, size_t filesize){

    res = f_open(&file, filename, FA_READ);
    if(res != FR_OK){
        System_printf("failed to open, %s\n",filename);
        return 0;
    }
    System_printf("File opened successfully for reading.\n");

    // chunk size of 2048 //TODO: define macro for chunk size
    char* hbuff = (char*)malloc(sizeof(char)*2048);
    if(hbuff == NULL){
        System_printf("failed to allocate heap memory\n");
        return 0;
    }
    do{
        // get chunk
       memset(hbuff, 0, 2048);
       res = f_read(&file, hbuff, 2048, &br); // Read a chunk
       if (res != FR_OK) {
           System_printf("Failed to read from file! Error: %d\n", res);
           free(hbuff);
           f_close(&file);
           return 0;
       }
      if(br > 0){
          send_chunk(sock, hbuff, br);
      }
       // hbuff[br] = '\0'; // Null-terminate the chunk
       // printf("%s\n", hbuff); // Print the chunk (or process it as needed)
    } while (br > 0); // Continue until no more data is read
    //end chunk frame
    send(sock, "0\r\n\r\n", 5, 0);

    // not done
    System_printf("file sent to web: %s\n", filename);
    free(hbuff);
    f_close(&file);
    return 1;
}

/*base64 encoder*/
size_t b64_encoded_size(size_t inlen)
{
    size_t ret;

    ret = inlen;
    if (inlen % 3 != 0)
        ret += 3 - (inlen % 3);
    ret /= 3;
    ret *= 4;

    return ret;
}

char *b64_encode(const unsigned char *in, size_t len, char* out, size_t elen)
{
    const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t  i;
    size_t  j;
    size_t  v;

    if (in == NULL || len == 0)
        return NULL;
    out[elen] = '\0';

    for (i=0, j=0; i<len; i+=3, j+=4) {
        v = in[i];
        v = i+1 < len ? v << 8 | in[i+1] : v << 8;
        v = i+2 < len ? v << 8 | in[i+2] : v << 8;

        out[j]   = b64chars[(v >> 18) & 0x3F];
        out[j+1] = b64chars[(v >> 12) & 0x3F];
        if (i+1 < len) {
            out[j+2] = b64chars[(v >> 6) & 0x3F];
        } else {
            out[j+2] = '=';
        }
        if (i+2 < len) {
            out[j+3] = b64chars[v & 0x3F];
        } else {
            out[j+3] = '=';
        }
    }

    return out;
}

/*sha1 header*/
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

/*********************** FUNCTION DECLARATIONS **********************/
void sha1_init(SHA1_CTX *ctx);
void sha1_update(SHA1_CTX *ctx, const BYTES data[], size_t len);
void sha1_final(SHA1_CTX *ctx, BYTES hash[]);

/****************************** MACROS ******************************/
#define ROTLEFT(a, b) ((a << b) | (a >> (32 - b)))

/*********************** FUNCTION DEFINITIONS ***********************/
void sha1_transform(SHA1_CTX *ctx, const BYTES data[])
{
    WORDS a, b, c, d, e, i, j, t, m[80];

    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) + (data[j + 1] << 16) + (data[j + 2] << 8) + (data[j + 3]);
    for ( ; i < 80; ++i) {
        m[i] = (m[i - 3] ^ m[i - 8] ^ m[i - 14] ^ m[i - 16]);
        m[i] = (m[i] << 1) | (m[i] >> 31);
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];

    for (i = 0; i < 20; ++i) {
        t = ROTLEFT(a, 5) + ((b & c) ^ (~b & d)) + e + ctx->k[0] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 40; ++i) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[1] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 60; ++i) {
        t = ROTLEFT(a, 5) + ((b & c) ^ (b & d) ^ (c & d))  + e + ctx->k[2] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }
    for ( ; i < 80; ++i) {
        t = ROTLEFT(a, 5) + (b ^ c ^ d) + e + ctx->k[3] + m[i];
        e = d;
        d = c;
        c = ROTLEFT(b, 30);
        b = a;
        a = t;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
}

void sha1_init(SHA1_CTX *ctx)
{
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xc3d2e1f0;
    ctx->k[0] = 0x5a827999;
    ctx->k[1] = 0x6ed9eba1;
    ctx->k[2] = 0x8f1bbcdc;
    ctx->k[3] = 0xca62c1d6;
}

void sha1_update(SHA1_CTX *ctx, const BYTES data[], size_t len)
{
    size_t i;

    for (i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha1_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha1_final(SHA1_CTX *ctx, BYTES hash[])
{
    WORDS i;

    i = ctx->datalen;

    // Pad whatever data is left in the buffer.
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else {
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha1_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha1_transform(ctx, ctx->data);

    // Since this implementation uses little endian byte ordering and MD uses big endian,
    // reverse all the bytes when copying the final state to the output hash.
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
    }
}

int handle_websocket_handshake(int client_socket, char* buffer) {
    // char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    const char *websocket_key_header = "Sec-WebSocket-Key:";
    char *key_start;
    char sec_websocket_key[128] = {0};
    unsigned char sha1_hash[SHA1_HASH_SIZE];
//    char base64_encoded_hash[128];
    const char *websocket_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // buffer[bytes_received] = '\0';
    key_start = strstr(buffer, websocket_key_header);
    System_printf("DEBUG : %s\n", key_start);
    if (key_start == NULL) {
        System_printf("Invalid WebSocket handshake request.\n");
//        close(client_socket);
        return -1;
    }

    // Extract the Sec-WebSocket-Key
    sscanf(key_start + strlen(websocket_key_header), "%s", sec_websocket_key);

    // Concatenate the key with the GUID
    char concatenated_key[256];
    snprintf(concatenated_key, sizeof(concatenated_key), "%s%s", sec_websocket_key, websocket_guid);

    // Generate SHA-1 hash of the concatenated key
    SHA1_CTX ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, (unsigned char*)concatenated_key, strlen(concatenated_key));
    sha1_final(&ctx, sha1_hash);



    // Perform Base64 encoding
    size_t  olen;
    size_t ilen = 20;
    olen = b64_encoded_size(ilen);
    char cipher[olen+1];

    b64_encode(sha1_hash, ilen,cipher,olen);

    // Build WebSocket handshake response
    snprintf(response, sizeof(response),
             "HTTP/1.1 101 Switching Protocols\r\n"
             "Upgrade: websocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Accept: %s\r\n\r\n",
             cipher);

    // Send the handshake response
    send(client_socket, response, strlen(response), 0);
    System_printf("WebSocket handshake completed. Response:\n%s\n", response);
    return 1;
}

void sendPongFrame(int clientfd) {
    unsigned char frame[2] = {0x8A, 0x00};  // Pong frame with no payload (just the header)
    send(clientfd, frame, 2, 0);
    System_printf("\nSent Pong Frame\n");
    System_flush();
}




// Function to send WebSocket frame with payload from server to client (no masking)
int send_websocket_frame(int client_socket, const char *payload, size_t payload_len) {
    uint8_t frame[10 + payload_len];  // Frame will include header and payload
    size_t frame_len = 2; // Initial length for WebSocket frame (basic header)

    // First byte: FIN(1), RSV1(1), RSV2(1), RSV3(1), Opcode(4)
    frame[0] = 0x81; // FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0x1 (text frame)

    // Second byte: Mask(1), Payload length (7 bits max)
    frame[1] = (payload_len < 126) ? payload_len : 126;

    if (payload_len >= 126) {
        // Handle Payload Length Extended (126 or 65536 bytes)
        if (payload_len < 65536) {
            frame[1] = 126;
            frame[2] = (payload_len >> 8) & 0xFF;
            frame[3] = payload_len & 0xFF;
            frame_len = 4; // Increase length for extended payload length
        } else {
            frame[1] = 127;
            frame[2] = (payload_len >> 56) & 0xFF;
            frame[3] = (payload_len >> 48) & 0xFF;
            frame[4] = (payload_len >> 40) & 0xFF;
            frame[5] = (payload_len >> 32) & 0xFF;
            frame[6] = (payload_len >> 24) & 0xFF;
            frame[7] = (payload_len >> 16) & 0xFF;
            frame[8] = (payload_len >> 8) & 0xFF;
            frame[9] = payload_len & 0xFF;
            frame_len = 10; // Increase length for very large payload
        }
    }

    // No masking here, since the server does not mask the payload
    frame[1] &= ~0x80;  // Unset the Mask bit (Mask=0)

    // Add the payload data after the frame header
    memcpy(frame + frame_len, payload, payload_len);

    // Send the frame over the socket to the client
    ssize_t bytes_sent = send(client_socket, frame, frame_len + payload_len, 0);
    if (bytes_sent < 0) {
//        perror("Failed to send WebSocket frame");
        return -1;
    }

    return 0; // Success
}

// websock
//void create_websocket();
//void webWorker(UArg arg0, UArg arg1) {
//    fdOpenSession((void *)Task_self());
//    int  clientfd = (int)arg0;
//    System_printf("webWorker: start clientfd = 0x%x\n", clientfd);
//    System_flush();
//    int  bytesRcvd;
////    int  bytesSent;
//    char buffer[TCPPACKETSIZE];
//    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
//        buffer[bytesRcvd] = '\0';
//        unsigned char ping_frame[] = {0x89, 0x00};  // Ping frame with no payload
//        send(clientfd, ping_frame, sizeof(ping_frame), 0);
//
//    }
//    System_printf("webWorker: stop clientfd = 0x%x\n", clientfd);
//    fdCloseSession((void *)Task_self());
//}

Void tcpWorker(UArg arg0, UArg arg1)
{
    fdOpenSession((void *)Task_self());
    int  clientfd = (int)arg0;
    int  bytesRcvd;
//    int  bytesSent;
    char buffer[TCPPACKETSIZE];

    System_printf("tcpWorker: start clientfd = 0x%x\n", clientfd);
    System_flush();

    while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
        buffer[bytesRcvd] = '\0';  // Ensure null termination
//        System_printf("Request:%s\n",buffer);
        res = f_mount(&fs, "", 1); // Mount filesystem
        if (res != FR_OK) {
            System_printf("Failed to mount filesystem! Error: %d\n", res);
            System_flush();
            return;
        }

        if (bytesRcvd >= 5) {  // Check only if valid request received
            if (strncmp(buffer, "GET / ", 6) == 0) {
                if(strstr(buffer, "Upgrade: websocket") != NULL && strstr(buffer, "Connection: Upgrade") != NULL) {
                    // websock reposnse
                    if(handle_websocket_handshake(clientfd, buffer)) {
                        // websocket communication
                        // create_websocket(); // to swicth to another port
                        System_printf("Websocket started\n"); System_flush();
                        while ((bytesRcvd = recv(clientfd, buffer, TCPPACKETSIZE, 0)) > 0) {
//                            sendPongFrame(clientfd);
//                            buffer[bytesRcvd] = '\0';

                            // TODO: handle websocket frames form client
//                            uint8_t opcode = buffer[0] & 0x0F;  // Extract opcode
                            uint8_t masked = buffer[1] & 0x80;  // Check if frame is masked
//                            uint8_t fin = buffer[0] & 0x80; // check if frames is fragmented
                            uint64_t payload_length = buffer[1] & 0x7F;  // Extract payload length
                            uint8_t offset = 2;

                            // extract payload length
                            if (payload_length == 126) {
                                // next 2 byte length
                                payload_length = (buffer[offset] << 8) | buffer[offset + 1];
                                offset += 2;
                            } else if (payload_length == 127) {
                                // next 8 byte length
                                payload_length = 0;
                                uint8_t i = 0;
                                for (i = 0; i < 8; i++) {
                                    payload_length = (payload_length << 8) | buffer[offset + i];
                                }
                                offset += 8;
                            }

                            // if masked extract masking key
                            uint8_t masking_key[4];
                            if (masked) {
                                memcpy(masking_key, &buffer[offset], 4);
                                offset += 4;
                            }

                            // Decode the payload
                            uint64_t i = 0;
                            char payload[payload_length + 1];
                            for (i = 0; i < payload_length; i++) {
                                payload[i] = masked ? (buffer[offset + i] ^ masking_key[i % 4]) : buffer[offset + i];
                            }
                            payload[payload_length] = '\0';
                            memset(buffer, 0, sizeof(buffer));



                            // check commands
                            if(!strcmp(payload,"vasu")) {
                                send_websocket_frame(clientfd, payload, payload_length);
                            }
                            else {
                                send_websocket_frame(clientfd,"Invalid", strlen("Invalid"));
                            }

                            System_printf(payload); System_flush();


                            //TODO: hanlde fragmemted frames if neccessary
                        }
                        System_printf("\nWebsocket closed\n"); System_flush();

                    } else {
                        const char bad_request[] =
                                "HTTP/1.1 400 Bad Request\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 15\r\n\r\n"
                                "400 Bad Request";

                        send(clientfd, bad_request, strlen(bad_request), 0);
                    }
                } else {
                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n");
                send(clientfd, response, strlen(response), 0);
                // send chunks
                // send_chunk(clientfd, "<!DOCTYPE html>"
                //            "<html lang=\"en\">"
                //            "<head>"
                //            "<meta charset=\"UTF-8\">"
                //            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
                //            "<title>Webserver</title>");
                // send_chunk(clientfd,"<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\"/>"
                //                     "<script src=\"javascript.js\"></script>"
                //                     "</head>"
                //                     "<body>"
                //                     "<div class=\"container\">"
                //                     "<div class=\"input-container\">"
                //                     "<input type=\"text\" id=\"inputBox\" placeholder=\"Enter Command\" onkeydown=\"checkEnter(event)\">"
                //                     );
                // send_chunk(clientfd,"<button id=\"sendButton\" onclick=\"sendMessage()\">Send</button>"
                //                     "<button id=\"connectButton\" onClick=\"initWebSockUpgrade()\">Connect</button>"
                //                     "</div>"
                //                     "<div class=\"output-box\" id=\"outputBox\"></div>"
                //                     "</div>"
                //                     "</body>"
                //                     "</html>");
                // send(clientfd, "0\r\n\r\n", 5, 0);
               send_file_web("index.htm", clientfd, INDEX_SIZE);

                }
            }
            else if (strncmp(buffer, "GET /styles.css", 15) == 0) {

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                                         "HTTP/1.1 200 OK\r\n"
                                         "Content-Type: text/css\r\n"
                                         "Transfer-Encoding: chunked\r\n"
                                         "\r\n");
                send(clientfd, response, strlen(response), 0);

//                 send_chunk(clientfd, "body{font-family:'Courier New',monospace;background-color:black;margin:0;padding:20px;"
//                 "display:flex;justify-content:center;align-items:center;height:100vh;color:white;}"
//                 ".container{background-color:black;padding:30px;border-radius:8px;box-shadow:0 0px 15px rgba(255,255,255,0.2);"
//                 );
//                 send_chunk(clientfd, "width:700px;text-align:center;display:flex;flex-direction:column;justify-content:center;"
//                            "align-items:center;gap:20px;border:1px solid white;}"
//                            ".input-container{display:flex;align-items:center;gap:10px;}"
//                            );
//                 send_chunk(clientfd, "#inputBox{width:500px;padding:10px;font-size:16px;border-radius:4px;background-color:black;"
//                            "color:white;border:1px solid white;}"
//                            "#sendButton{padding:10px 20px;font-size:16px;background-color:black;color:white;border:1px solid white;"
//                            );
//                 send_chunk(clientfd, "border-radius:4px;cursor:pointer;}"
//                            "#sendButton:hover{background-color:white;color:black;border:1px black;}"
//                            "#connectButton{padding:10px 20px;font-size:16px;background-color:black;color:white;border:1px solid white;"
//                            );
//                 send_chunk(clientfd, "border-radius:4px;cursor:pointer;}"
//                            "#connectButton:hover{background-color:white;color:black;border:1px black;}"
//                            ".output-box{width:100%;height:300px;background-color:black;color:white;border:1px solid white;padding:10px;"
//                            );
//                 send_chunk(clientfd, "border-radius:6px;margin-top:5px;display:flex;flex-direction:column;align-items:flex-start;"
//                            "justify-content:flex-start;overflow-y:auto;font-size:16px;white-space:pre-wrap;"
//                            "font-family:'Courier New',monospace;}");
                // send(clientfd, "0\r\n\r\n", 5, 0);
               send_file_web("styles.css", clientfd, STYLES_SIZE);
            }
            else if (strncmp(buffer, "GET /javascript.js", 18) == 0) {

                char response[BUFFER_SIZE];
                snprintf(response, sizeof(response),
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: application/javascript\r\n"
                         "Transfer-Encoding: chunked\r\n"
                         "\r\n");
                send(clientfd, response, strlen(response), 0);
                // send chunks
                // send_chunk(clientfd, "const cmds=[\"zeus\",\"poseidon\",\"achilles\",\"thor\"];"
                //            "function check_cmd(message){"
                //            "if(!cmds.includes(message)){"
                //            "document.getElementById('inputBox').placeholder=\"Invalid command!\";"
                //            "}else{"
                //            "document.getElementById('inputBox').placeholder=\"Enter Command\";"
                //            "}}"
                //            "function checkEnter(event){"
                //            "if(event.key==='Enter'){"
                //            "sendMessage();"
                //            "}}"
                //            "function displayOutput(message){"
                //            "var outputBox=document.getElementById('outputBox');");
                // send_chunk(clientfd, "outputBox.textContent=message;"
                //            "}"
                //            "let socket;"
                //            "function initWebSockUpgrade(){"
                //            "const serverUrl=\"ws://10.130.214.85:8765\";"//TODO: PORT MODIFY HERE
                //            "socket=new WebSocket(serverUrl);"
                //            "socket.onopen=function(){"
                //            "const outputBox=document.getElementById(\"outputBox\");"
                //            "const timestamp=new Date().toLocaleTimeString();"
                //            "outputBox.innerHTML+=`<div>${timestamp}: WebSocket connection established successfully!</div>`;"
                //            "outputBox.scrollTop=outputBox.scrollHeight;"
                //            "};");
                // send_chunk(clientfd,  "socket.onmessage=function(event){"
                //            "const outputBox=document.getElementById(\"outputBox\");"
                //            "const timestamp=new Date().toLocaleTimeString();"
                //            "outputBox.innerHTML+=`<div>${timestamp}: Server: ${event.data}</div>`;"
                //            "outputBox.scrollTop=outputBox.scrollHeight;"
                //            "};"
                //            "socket.onclose=function(event){"
                //            "const outputBox=document.getElementById(\"outputBox\");"
                //            "const timestamp=new Date().toLocaleTimeString();"
                //            "outputBox.innerHTML+=`<div>${timestamp}: WebSocket connection closed.</div>`;"
                //            "outputBox.scrollTop=outputBox.scrollHeight;"
                //            "};");
                // send_chunk(clientfd, "socket.onerror=function(error){"
                //            "const outputBox=document.getElementById(\"outputBox\");"
                //            "const timestamp=new Date().toLocaleTimeString();"
                //            "outputBox.innerHTML+=`<div>${timestamp}: WebSocket error: ${error.message}</div>`;"
                //            "outputBox.scrollTop=outputBox.scrollHeight;"
                //            "};"
                //            "}"
                //            "function sendMessage(){"
                //            "const inputBox=document.getElementById('inputBox');"
                //            "const outputBox=document.getElementById('outputBox');"
                //            "const message=inputBox.value;");
                // send_chunk(clientfd, "check_cmd(message);"
                //            "if(message){"
                //            "const timestamp=new Date().toLocaleTimeString();"
                //            "if(socket&&socket.readyState===WebSocket.OPEN){"
                //            "socket.send(message);"
                //            "outputBox.innerHTML+=`<div>${timestamp}: Sent: ${message}</div>`;"
                //            "}else{"
                //            "outputBox.innerHTML+=`<div>${timestamp}: Error: WebSocket is not connected!</div>`;"
                //            "}"
                //            "outputBox.scrollTop=outputBox.scrollHeight;"
                //            "inputBox.value='';"
                //            "}}");
                // send(clientfd, "0\r\n\r\n", 5 ,0);
               send_file_web("script.js", clientfd, SCRIPT_SIZE);



            }
            else {
                // 404 Response for unmatched requests
                const char not_found[] =
                    "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "\r\n"
                    "404 Not Found";

                send(clientfd, not_found, strlen(not_found), 0);
            }
        }
        res = f_mount(NULL, "", 0); // Unmount filesystem
        if (res != FR_OK) {
            System_printf("Failed to Unmount filesystem! Error: %d\n", res);
            System_flush();
            return;
        }
//        bytesSent = send(clientfd, buffer, bytesRcvd, 0);
    }
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);
    System_flush();

    close(clientfd);
    fdCloseSession((void *)Task_self());
}

Void tcpHandler(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    int                server;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    int                optval;
    int                optlen = sizeof(optval);
    socklen_t          addrlen = sizeof(clientAddr);
    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;

    /*
     * Create a TCP stream socket.
     */

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) {
        System_printf("Error: socket not created.\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Setup the local IP address and port to bind to.
     */
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(arg0);

    /*
     * Associates and reserves a port for use by the socket
     */
    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        System_printf("Error: bind failed.\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Instructs TCP protocol to listen for connections with up to NUMTCPWORKERS active
     * participants that can wait for a connection.
     */
    status = listen(server, NUMTCPWORKERS);
    if (status == -1) {
        System_printf("Error: listen failed.\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Setup socket option as to enable keeping connections alive
     */
    optval = 1;
    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        System_printf("Error: setsockopt failed\n");
        System_flush();
        goto shutdown;
    }

    /*
     * Accept a connection from a remote host.
     */
    while ((clientfd =
            accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1) {

        System_printf("tcpHandler: Creating thread clientfd = %d\n", clientfd);
        System_flush();

        /* Init the Error_Block */
        Error_init(&eb);

        /*
         * Initialize the defaults and set the parameters.
         * Create a new task for each connection to process the receiving data
         * from the remote host.
         */
        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 4096;
        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);
        if (taskHandle == NULL) {
            System_printf("Error: Failed to create new Task\n");
            System_flush();
            close(clientfd);
        }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);
    }

    System_printf("Error: accept failed.\n");
    System_flush();

shutdown:
    if (server > 0) {
        close(server);
    }
}
/*
 * Startup function
 */
void flash_init(void)
{

    format_disk();
    System_printf("Storing the webpages in SPI Flash\n");
    System_flush();

    res = f_mount(&fs, "", 1); // Mount filesystem
    if (res != FR_OK) {
        System_printf("Failed to mount filesystem! Error: %d\n", res);
        System_flush();
        return;
    }

    write_file("index.htm", INDEX, INDEX_SIZE);
    write_file("styles.css", STYLES, STYLES_SIZE);
    write_file("script.js", SCRIPT, SCRIPT_SIZE);

    res = f_mount(NULL, "", 0); // Unmount filesystem
    if (res != FR_OK) {
        System_printf("Failed to Unmount filesystem! Error: %d\n", res);
        System_flush();
        return;
    }
    System_printf("Webpages stored successfully in SPI Flash\n");
    System_flush();
}

#define DELAY_MS(i)      (Task_sleep(((i) * 1000) / Clock_tickPeriod))

void led_blink(void){
    while(1){
        LED_ON_MACRO();
        DELAY_MS(1000);
        LED_OFF_MACRO();
        DELAY_MS(1000);
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();
    Led_Init();


    System_printf("Starting the TCP Echo Server example\nSystem provider is set to "
                  "SysMin. Halt the target to view any SysMin contents in"
                  " ROV.\n");
    // SysMin will only print to the console when you call flush or exit
    System_flush();
//     Create a task for one-time initialization
    Task_Handle taskHandle;
    Task_Params taskParams;
    Task_Params_init(&taskParams);
    taskParams.stackSize = 512; // Adjust stack size as needed
    taskParams.priority = 1;    // Set an appropriate priority
    taskHandle = Task_create((Task_FuncPtr)led_blink, &taskParams, NULL);
    if (taskHandle == NULL) {
        System_printf("Error: Failed to create new Task\n");
        System_flush();
    }

    //     Create a task for one-time initialization
    Task_Handle taskHandle1;
    Task_Params taskParams1;
    Task_Params_init(&taskParams1);
    taskParams1.stackSize = 8192; // Adjust stack size as needed
    taskParams1.priority = 1;    // Set an appropriate priority
    taskHandle1 = Task_create((Task_FuncPtr)flash_init, &taskParams1, NULL);
    if (taskHandle1 == NULL) {
        System_printf("Error: Failed to create new Task\n");
        System_flush();
    }

    BIOS_start();

    return (0);
}
