/*
 * fats.h
 *
 *  Created on: 30-Jan-2025
 *      Author: 14169
 */

#ifndef FATS_H_
#define FATS_H_

//FATFS headers
#include "source/ff.h"
#include <xdc/runtime/System.h>

#define CHUNK_SIZE 2048

struct file {
    unsigned char* filename;
    int filesize;
    unsigned char* filedata;
};

struct folder {
    unsigned char* folder_name;
    int file_count;
    int subfolder_count;
    struct folder* subfolders;
    struct file* files;
};

//FATFS fs;          // Filesystem object
//FIL file;          // File object
//static FRESULT res;       // FatFs function result
//UINT bw, br;       // Bytes written/read


void format_disk(void);
unsigned int write_file(const char* filename, unsigned char* data, unsigned int len);
unsigned int send_file_web(const char* filename, int sock);
int store_folder(struct folder* root, char* parent_path);
FRESULT scan_files (char* path);
void mount();
void unmount();

#endif /* FATS_H_ */
