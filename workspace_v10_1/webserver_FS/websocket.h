#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#define BUFFER_SIZE 1024
#define SHA1_HASH_SIZE 20

int handle_websocket_handshake(int client_socket, char* buffer);
void sendPongFrame(int clientfd);
int send_websocket_frame(int client_socket, const char *payload, size_t payload_len);
unsigned int send_file_web(const char* filename, int sock);

#endif
