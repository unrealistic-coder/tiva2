/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

//#include "lwip/apps/httpd_opts.h" //source path
#include <enet_fs_updated.h>
#include "httpd_opts.h"
#include "lwip/def.h"
#include "lwip/apps/fs.h"
#include <string.h>
#include <stdio.h>
#include "fatfs/source/ff.h"
#include "fatfs/source/user_diskio.h"
#include "fsdata.c"
#include "lwip/mem.h"
//*****************************************************************************
//
// The following are data structures used by FatFs.
//
//*****************************************************************************
static FATFS g_sFatFs;

//*****************************************************************************
//
// The number of milliseconds that has passed since the last disk_timerproc()
// call.
//
//*****************************************************************************
static uint32_t ui32TickCounter = 0;
//*****************************************************************************
//
// Initialize the file system.
//
//*****************************************************************************
void
fs_init(void)
{
    //
    // Initialize and mount the Fat File System.
    //
    FRESULT res = f_mount(&g_sFatFs,"0:",1);
    if (res != FR_OK) {
        perror("Mount failed\n");
//        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
//    printf("Filesystem mounted successfully.\n");
}

//*****************************************************************************
//
// File System tick handler.
//
//*****************************************************************************
void
fs_tick(uint32_t ui32TickMS)
{
    //
    // Increment the tick counter.
    //
    ui32TickCounter += ui32TickMS;

    //
    // Check to see if the FAT FS tick needs to run.
    //
    if(ui32TickCounter >= 10)
    {
        ui32TickCounter = 0;
//        disk_timerproc();
    }
}
//--------------------------------CUSTOM--FS--FUNCTIONS------------------------------
int
fs_open_custom(struct fs_file *file, const char *name){
    FIL *psFatFile = NULL;
    FRESULT fresult = FR_OK;
    static char path[40];

    // Resolve path /fsg-gzip, /fsd-deflate, /fs-original
    snprintf(path, sizeof(path), "/fsg%s", name);

    // Allocate memory for file pointer
    psFatFile = mem_malloc(sizeof(FIL));
    if(psFatFile == NULL)
    {
      return ERR_MEM;
    }

    // Attempt to open the file on the Fat File System.
    fresult = f_open(psFatFile, path , FA_READ); // pcName changed from pcName+3 coz we not using /sd/ path
    //        if(fresult != FR_OK){
    //            if (fresult != FR_OK) {
    //                printf("f_open failed! Error code: %d, File: %s\n", fresult, pcName);
    //            }
    //        }
    if(fresult == FR_OK)
    {
      file->data = NULL;
      file->len = 0;
      file->index = 0;
      file->pextension = psFatFile;
      return ERR_OK;
    }

    // If we get here, we failed to find the file on the Fat File System,
    // so free up the Fat File system handle/object.
    mem_free(psFatFile);
    return ERR_VAL;
}

int
fs_read_custom(struct fs_file *psFile, char *pcBuffer, int iCount){
    int iAvailable;
    UINT uiBytesRead;
    FRESULT fresult;

    // Check to see if a Fat File was opened and process it.
    if(psFile->pextension)
    {
        // Read the data.
        fresult = f_read(psFile->pextension, pcBuffer, iCount, &uiBytesRead);
        if((fresult != FR_OK) || (uiBytesRead == 0))
        {
            return(-1);
        }
        return((int)uiBytesRead);
    }

    // Check to see if more data is available.
    if(psFile->index == psFile->len)
    {
        // There is no remaining data.  Return a -1 for EOF indication.
        return(-1);
    }

    // Determine how much data we can copy.  The minimum of the 'iCount'
    // parameter or the available data in the file system buffer.
    iAvailable = psFile->len - psFile->index;
    if(iAvailable > iCount)
    {
        iAvailable = iCount;
    }

    // Copy the data.
    memcpy(pcBuffer, psFile->data + iAvailable, iAvailable);
    psFile->index += iAvailable;

    // Return the count of data that we copied.
    return(iAvailable);
}

void
fs_close_custom(struct fs_file *file){

    // If a Fat file was opened, free its object.
    if(file->pextension)
    {
        mem_free(file->pextension);
    }

}
/*-----------------------------------------------------------------------------------*/
err_t
fs_open(struct fs_file *file, const char *name)
{
  const struct fsdata_file *f;

  if ((file == NULL) || (name == NULL)) {
    return ERR_ARG;
  }

#if LWIP_HTTPD_CUSTOM_FILES
  if (fs_open_custom(file, name)) {
    file->flags |= FS_FILE_FLAGS_CUSTOM;
    return ERR_OK;
  }
#endif /* LWIP_HTTPD_CUSTOM_FILES */
//-------------------------------------------------------------------------------------
  if(1) // from flash only
  {
      // Variable declaration
      FIL *psFatFile = NULL;
      FRESULT fresult = FR_OK;
      static char path[40];

      // Resolve path /fsg-gzip, /fsd-deflate, /fs-original
      snprintf(path, sizeof(path), "/fsg%s", name);

      // Allocate memory for file pointer
      psFatFile = mem_malloc(sizeof(FIL));
      if(psFatFile == NULL)
      {
          return ERR_MEM;
      }

      // Attempt to open the file on the Fat File System.
      fresult = f_open(psFatFile, path , FA_READ); // pcName changed from pcName+3 coz we not using /sd/ path
//        if(fresult != FR_OK){
//            if (fresult != FR_OK) {
//                printf("f_open failed! Error code: %d, File: %s\n", fresult, pcName);
//            }
//        }
      if(fresult == FR_OK)
      {
          file->data = NULL;
          file->len = 0;
          file->index = 0;
          file->pextension = psFatFile;
          return ERR_OK;
      }

      // If we get here, we failed to find the file on the Fat File System,
      // so free up the Fat File system handle/object.
      mem_free(psFatFile);
      return ERR_VAL;
  }
//-------------------------------------------------------------------------------------
  else
  {
      for (f = FS_ROOT; f != NULL; f = f->next) {
        if (!strcmp(name, (const char *)f->name)) {
          file->data = (const char *)f->data;
          file->len = f->len;
          file->index = f->len;
          file->flags = f->flags;
#if HTTPD_PRECALCULATED_CHECKSUM
          file->chksum_count = f->chksum_count;
          file->chksum = f->chksum;
#endif /* HTTPD_PRECALCULATED_CHECKSUM */
#if LWIP_HTTPD_FILE_EXTENSION
          file->pextension = NULL;
#endif /* LWIP_HTTPD_FILE_EXTENSION */
#if LWIP_HTTPD_FILE_STATE
          file->state = fs_state_init(file, name);
#endif /* #if LWIP_HTTPD_FILE_STATE */
          return ERR_OK;
        }
      }
      /* file not found */
      return ERR_VAL;
  }
}

/*-----------------------------------------------------------------------------------*/
void
fs_close(struct fs_file *file)
{
  //--------------------------------------------------------------------------------
  if(1) // flash only
  {
     if(file->pextension)
         mem_free(file->pextension);
  }
  //--------------------------------------------------------------------------------
#if LWIP_HTTPD_CUSTOM_FILES
  if ((file->flags & FS_FILE_FLAGS_CUSTOM) != 0) {
    fs_close_custom(file);
  }
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#if LWIP_HTTPD_FILE_STATE
  fs_state_free(file, file->state);
#endif /* #if LWIP_HTTPD_FILE_STATE */
  LWIP_UNUSED_ARG(file);
}
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_DYNAMIC_FILE_READ
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_read_async(struct fs_file *file, char *buffer, int count, fs_wait_cb callback_fn, void *callback_arg)
#else /* LWIP_HTTPD_FS_ASYNC_READ */
int
fs_read(struct fs_file *file, char *buffer, int count)
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
{
  int read;
  if (file->index == file->len) {
    return FS_READ_EOF;
  }
#if LWIP_HTTPD_FS_ASYNC_READ
  LWIP_UNUSED_ARG(callback_fn);
  LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
#if LWIP_HTTPD_CUSTOM_FILES
  if ((file->flags & FS_FILE_FLAGS_CUSTOM) != 0) {
#if LWIP_HTTPD_FS_ASYNC_READ
    return fs_read_async_custom(file, buffer, count, callback_fn, callback_arg);
#else /* LWIP_HTTPD_FS_ASYNC_READ */
    return fs_read_custom(file, buffer, count);
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
  }
#endif /* LWIP_HTTPD_CUSTOM_FILES */
//-------------------------------------------------------------------------------------
  UINT uiBytesRead;
  FRESULT fresult;
  if(file->pextension){
      fresult = f_read(file->pextension, buffer, count, &uiBytesRead);
      if((fresult != FR_OK) || (uiBytesRead == 0))
      {
          return(-1);
      }
      return((int)uiBytesRead);
  }
  if(file->index == file->len)
  {
      // There is no remaining data.  Return a -1 for EOF indication.
      return(-1);
  }
//-------------------------------------------------------------------------------------
  read = file->len - file->index;
  if (read > count) {
    read = count;
  }

  MEMCPY(buffer, (file->data + file->index), read);
  file->index += read;

  return (read);
}
#endif /* LWIP_HTTPD_DYNAMIC_FILE_READ */
/*-----------------------------------------------------------------------------------*/
#if LWIP_HTTPD_FS_ASYNC_READ
int
fs_is_file_ready(struct fs_file *file, fs_wait_cb callback_fn, void *callback_arg)
{
  if (file != NULL) {
#if LWIP_HTTPD_FS_ASYNC_READ
#if LWIP_HTTPD_CUSTOM_FILES
    if (!fs_canread_custom(file)) {
      if (fs_wait_read_custom(file, callback_fn, callback_arg)) {
        return 0;
      }
    }
#else /* LWIP_HTTPD_CUSTOM_FILES */
    LWIP_UNUSED_ARG(callback_fn);
    LWIP_UNUSED_ARG(callback_arg);
#endif /* LWIP_HTTPD_CUSTOM_FILES */
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
  }
  return 1;
}
#endif /* LWIP_HTTPD_FS_ASYNC_READ */
/*-----------------------------------------------------------------------------------*/
int
fs_bytes_left(struct fs_file *file)
{
//-------------------------------------------------------------------------------------
    if(file->pextension)
    {
        // Return the number of bytes left to be read from the Fat File.
        return(f_size((FIL *)file->pextension) -
               f_tell((FIL *)file->pextension));
    }
//-------------------------------------------------------------------------------------

  return file->len - file->index;
}
