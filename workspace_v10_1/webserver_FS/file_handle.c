// C std headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "file_handle.h"
//FATFS headers
#include "source/user_diskio.h"
#include "source/ff.h"
#include <xdc/runtime/System.h>

//webpage files
#include "fs/index.h"
#include "fs/styles.h"
#include "fs/script.h"

#define printf System_printf

void format_disk(void) {
    FRESULT res;
    MKFS_PARM fs_params = {FM_FAT | FM_SFD, 1, 1, 1, 4096}; // FAT12/16/32
    BYTE work[FF_MAX_SS]; // Work buffer (aligned with sector size)

    res = f_mkfs("", &fs_params, work, sizeof(work));
    if (res == FR_OK) {
        printf("Disk formatted successfully.\n");
    } else {
        printf("Error formatting disk: %d\n", res);
    }
}

unsigned int write_file(const char* filename, unsigned char* data, unsigned int len){
    res = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);
    if(res != FR_OK){
        printf("failed to open, %s\n",filename);
        return;
    }
    printf("File opened successfully for writing.\n");

    res = f_write(&file, data, len, &bw);
    if(res != FR_OK){
        printf("failed to write, %s\n",filename);
        return;
    }
    printf("%s written successfully\n", filename);
//    printf("Data written to file: %s\n", data);
    f_close(&file);
    return bw;
}

unsigned int append_file(const char* filename, unsigned char* data, unsigned int len){
    res = f_open(&file, filename, FA_WRITE | FA_OPEN_APPEND);
    if(res != FR_OK){
        printf("failed to open, %s\n",filename);
        return;
    }
    printf("File opened successfully for append.\n");

    res = f_write(&file, data, len, &bw);
    if(res != FR_OK){
        printf("failed to append, %s\n",filename);
        return;
    }
    printf("%s appended successfully\n", filename);
//    printf("Data written to file: %s\n", data);
    f_close(&file);
    return bw;
}

unsigned int read_file(const char* filename){
    res = f_open(&file, filename, FA_READ);
    if(res != FR_OK){
        printf("failed to open, %s\n",filename);
        return;
    }
    printf("File opened successfully for reading.\n");
    /* heap buffer read */
    DWORD file_size = f_size(&file); // Get file size
    char *heap_buffer = (char *)malloc(file_size + 1); // Allocate memory for the file
    if (heap_buffer == NULL) {
        printf("Failed to allocate heap memory.\n");
        return;
    }
    res = f_read(&file, heap_buffer, file_size, &br);
    if (res != FR_OK || br != file_size) {
        printf("Failed to read entire file! Error: %d\n", res);
        free(heap_buffer); // Free allocated memory
        f_close(&file);
        return;
    }
    heap_buffer[file_size]='\0';
    printf("Data read from file: %s\n", heap_buffer);
    free(heap_buffer);
    f_close(&file);
    return br;
}

bool Mount(){
    res = f_mount(&fs, "", 1); // Mount on logical drive "", auto mount
    if (res != FR_OK) {    
        return false;
    }
    return true;
}

bool UnMount(){
    res = f_mount(NULL, "", 0); // Unmount
    if (res != FR_OK) {
        return false;
    }
    return true;
}

void store_webpages(){
    res = f_mount(&fs, "", 1); // Mount on logical drive "", auto mount
    if (res != FR_OK) {
        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem mounted successfully.\n");

    //operations
    write_file("index.htm", INDEX, INDEX_SIZE);
    write_file("styles.css", STYLES, STYLES_SIZE);
    write_file("script.js", SCRIPT, SCRIPT_SIZE);

    res = f_mount(NULL, "", 0); // Unmount
    if (res != FR_OK) {
        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem mounted successfully.\n");

}


