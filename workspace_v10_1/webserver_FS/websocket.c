#include "encoders.h"
#include "websocket.h"
#include "file_handle.h"
#include "encoders.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <xdc/runtime/System.h>


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

static inline void send_chunk(int sock, const char* data) {
    char chunk_size[4]; // max 2048
    snprintf(chunk_size, sizeof(chunk_size),"%X\r\n" ,(unsigned int)strlen(data));
    send(sock, chunk_size, strlen(chunk_size), 0); // send length of chunk with CRLF
    send(sock, data, strlen(data), 0); //send data
    send(sock,"\r\n",2,0);
}

unsigned int send_file_web(const char* filename, int sock){
    res = f_open(&file, filename, FA_READ);
    if(res != FR_OK){
        System_printf("failed to open, %s\n",filename);
        return;
    }
    System_printf("File opened successfully for reading.\n");

    // chunk size of 2048 //TODO: define macro for chunk size
    char* hbuff = (char*)malloc(sizeof(char)*2048);
    if(hbuff == NULL){
        System_printf("failed to allocate heap memory\n");
        return;
    }
    do{
        // get chunk
       res = f_read(&file, hbuff, 2048, &br); // Read a chunk
       if (res != FR_OK) {
           System_printf("Failed to read from file! Error: %d\n", res);
           free(hbuff);
           f_close(&file);
           return;
       }
       send_chunk(sock, hbuff);
       // hbuff[br] = '\0'; // Null-terminate the chunk
       // printf("%s\n", hbuff); // Print the chunk (or process it as needed)
    } while (br > 0); // Continue until no more data is read
    //end chunk frame
    send(sock, "0\r\n\r\n", 5, 0);
    free(hbuff);

    // not done
    printf("file sent to web: %s\n", filename);
    free(hbuff);
    f_close(&file);
    return br;
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
