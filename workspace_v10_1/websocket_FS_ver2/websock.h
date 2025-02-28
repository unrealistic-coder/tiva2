/*
 * websock.h
 *
 *  Created on: 31-Jan-2025
 *      Author: 14169
 */

#ifndef WEBSOCK_H_
#define WEBSOCK_H_

#define SHA1_HASH_SIZE 20
#define WEBSOCKET_OPCODE_TEXT 0x1
#define BUFFER_SIZE 1024

int handle_websocket_handshake(int client_socket, char* buffer);
void sendPongFrame(int clientfd);
int send_websocket_frame(int client_socket, const char *payload, size_t payload_len);

#endif /* WEBSOCK_H_ */
