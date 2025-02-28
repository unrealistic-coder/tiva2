#ifndef FILE_HANDLE_H_
#define FILE_HANDLE_H_

#include <stdbool.h>
#include "source/ff.h"


FATFS fs;          // Filesystem object
FIL file;          // File object
static FRESULT res;       // FatFs function result
UINT bw, br;       // Bytes written/read

// File Handling Functions
void format_disk(void);
unsigned int write_file(const char* filename, unsigned char* data, unsigned int len);
unsigned int append_file(const char* filename, unsigned char* data, unsigned int len);
unsigned int read_file(const char* filename);
bool Mount();
bool UnMount();
void store_webpages();

#endif
