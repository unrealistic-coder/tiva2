/*
 * fats.c
 *
 *  Created on: 30-Jan-2025
 *      Author: 14169
 */
#include <fats.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static inline void send_chunk(int sock, const char* data, size_t datalen) {
    char chunk_size[10];
    snprintf(chunk_size, sizeof(chunk_size), "%X\r\n", (unsigned int)datalen);
    send(sock, chunk_size, strlen(chunk_size),0); // Send chunk size
//    send_all(sock, data, datalen); // Send chunk data
//    size_t sent = 0;
//    while (sent < datalen) {
//        int sent_flag = send(sock, data + sent, datalen - sent, 0);
//        if (sent_flag < 0) {
//            perror("send failed");
//            return;
//        }
//        if(sent_flag == 0) {
//            perror("send failed");
//            return;
//        }
//        sent += sent_flag;
//    }
    send(sock, data, datalen, 0);
    send(sock, "\r\n", 2, 0); // Send CRLF after chunk data
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
    MKFS_PARM fs_params = {FM_FAT | FM_SFD, 1, 1, 1, 4096}; // FAT12/16/32
    BYTE work[FF_MAX_SS]; // Work buffer (aligned with sector size)

    res = f_mkfs("", &fs_params, work, sizeof(work));
    if (res == FR_OK) {
        System_printf("Disk formatted successfully.\n");
    } else {
        System_printf("Error formatting disk: %d\n", res);
    }
    System_flush();

}

unsigned int write_file(const char* filename, unsigned char* data, unsigned int len){
    FRESULT res;
    FIL file;
    UINT bw;

    res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if(res != FR_OK){
        System_printf("failed to open, %s\n",filename);
        System_flush();
        return 0;
    }
    System_printf("File opened successfully for writing.\n");

    res = f_write(&file, data, len, &bw);
    if(res != FR_OK){
        System_printf("failed to write, %s\n",filename);
        System_flush();
        return 0;
    }
    System_printf("%s written successfully\n", filename);
//    printf("Data written to file: %s\n", data);
    System_flush();
    f_close(&file);
    return bw;
}
// dynamic way
//
//unsigned int send_file_web(const char* filename, int sock){
//    FRESULT res;
//    FIL file;
//    UINT br;
//    int sum = 0;
//    System_printf("filename: %s\n",filename);
//    res = f_open(&file, filename, FA_READ);
//    if(res != FR_OK){
//        System_printf("failed to open, %s\n",filename);
//        return 0;
//    }
//    System_printf("File opened successfully for reading.\n");
//
//    // chunk size of 2048 //TODO: define macro for chunk size
//    char* hbuff = (char*)malloc(sizeof(char)*4096);
//    if(hbuff == NULL){
//        System_printf("failed to allocate heap memory\n");
//        return 0;
//    }
//    do{
//        // get chunk
//       memset(hbuff, 0, 4096);
//       res = f_read(&file, hbuff, 4096, &br); // Read a chunk
//       if (res != FR_OK) {
//           System_printf("Failed to read from file! Error: %d\n", res);
//           free(hbuff);
//           f_close(&file);
//           return 0;
//       }
//      if(br > 0){
//          send_chunk(sock, hbuff, br);
//      }
//      sum+=br;
//       // hbuff[br] = '\0'; // Null-terminate the chunk
//       // printf("%s\n", hbuff); // Print the chunk (or process it as needed)
//    } while (br > 0); // Continue until no more data is read
//    //end chunk frame
//    send(sock, "0\r\n\r\n", 5, 0);
//
//
//    System_printf("file sent to web: %s\n", filename);
//    System_printf("filesize: %d\n", sum);
//    free(hbuff);
//    f_close(&file);
//    return 1;
//}


//static way
unsigned int send_file_web(const char* filename, int sock) {
    FIL file;
    FRESULT res;
    UINT br;
    int total_sent = 0;

    res = f_open(&file, "/fs/MainPage.shtml", FA_READ);
    if (res != FR_OK) {
        System_printf("Failed to open file: %s\n", filename);
        System_flush();
//        send_404_response(sock);
        return 0;
    }

    System_printf("File opened successfully: %s\n", filename);
    System_flush();

    static char hbuff[CHUNK_SIZE];  // Use static buffer instead of malloc

    do {
        memset(hbuff, 0, CHUNK_SIZE);
        res = f_read(&file, hbuff, CHUNK_SIZE, &br);
        if (res != FR_OK) {
            System_printf("Failed to read file: %s\n", filename); System_flush();
            f_close(&file);
            return 0;
        }
        if (br > 0) {
            send_chunk(sock, hbuff, br);
            total_sent += br;
        }
    } while (br > 0);

    // Send final chunk (chunked transfer encoding requires a closing "0\r\n\r\n")
    send(sock, "0\r\n\r\n", 5, 0);

    System_printf("File sent successfully: %s (size: %d bytes)\n", filename, total_sent);
    System_flush();
    f_close(&file);
    return 1;
}

int store_folder(struct folder* root, char* parent_path) {
    FRESULT res;
    char root_path[256];
    snprintf(root_path, sizeof(root_path), "%s/%s", parent_path, root->folder_name);

    // Check if the folder already exists
    FRESULT stat_res = f_stat(root_path, NULL);
    if (stat_res != FR_OK) {
        res = f_mkdir(root_path);  // Create only if it doesn't exist
        if (res) return -1;
    }
    int i =0;
    // Store files in the current folder
    for (i = 0; i < root->file_count; i++) {
        char file_path[256];
        snprintf(file_path, sizeof(file_path), "%s/%s", root_path, root->files[i].filename);
        res = write_file(file_path, root->files[i].filedata, root->files[i].filesize);
        if (res < 1) return -2;
    }

    // Recursively store subfolders
    for (i = 0; i < root->subfolder_count; i++) {
        res = store_folder(&root->subfolders[i], root_path);
        if (res) {
            System_printf("Warning: Failed to store subfolder %s\n", root->subfolders[i].folder_name);
        }
    }
    System_flush();
    return 0;
}

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);
    System_printf("opened folder, %s\n",path);/* Open the directory */
    System_flush();
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {                                       /* It is a file. */
                System_printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
        System_flush();
    }

    return res;
}

void mount(){
    FATFS fs;
    FRESULT res;
    res = f_mount(&fs, "0:", 1); // Mount filesystem
    if (res != FR_OK) {
        System_printf("Failed to mount filesystem! Error: %d\n", res);
        System_flush();
        return;
    }
    System_printf("Mounted\n"); System_flush();
}

void unmount(){
    FRESULT res;
    res = f_mount(NULL, "0:", 0); // Unmount filesystem
    if (res != FR_OK) {
        System_printf("Failed to Unmount filesystem! Error: %d\n", res);
        System_flush();
        return;
    }
    System_printf("Un - Mounted\n"); System_flush();
}


