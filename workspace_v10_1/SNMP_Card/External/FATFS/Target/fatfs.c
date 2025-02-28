/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "fatfs.h"

int FatFS_mkdir(const char *Path)
{
    return f_mkdir(Path);
}

int FatFS_opendir(DIR *TempDirObj,const char *Path)
{
//    DIR TempDirObj;

    return f_opendir( TempDirObj, Path);
}

int FatFS_closedir(DIR *TempDirObj)
{
//    DIR TempDirObj;

    return f_closedir( TempDirObj);
}

#if 1
int FatFS_fopen (FatFS_file *File, const char *Path, uint8_t Options)
{
    return f_open(File, Path, Options);
}

int FatFS_fseek(FatFS_file *File, uint32_t Offset)
{
    return f_lseek( File, Offset);
}

int FatFS_fread(FatFS_file *File, void *Buffer, uint16_t BytesToRead)
{
    UINT ActualBytesRead = 0;
    FRESULT Status = f_read( File, Buffer, BytesToRead, &ActualBytesRead);

    if(BytesToRead != ActualBytesRead)
        Status = FR_DENIED;

    return Status;
}

int FatFS_fwrite(FatFS_file *File, const void *Buffer, uint16_t BytesToWrite)
{
    UINT ActualBytesWritten = 0;
    FRESULT Status = f_write( File, Buffer, BytesToWrite, &ActualBytesWritten);

    if(BytesToWrite != ActualBytesWritten)
        Status = FR_DENIED;

    return Status;
}

int FatFS_fclose(FatFS_file *File)
{
    return f_close(File);
}


DWORD FatFS_fgetfree(void)
{
    DWORD FreeClust = 0xFFFFFFFF;
    FRESULT frs;
    DWORD fre_clust;
    FATFS* fs = &fatfs[SPI_FLASH];
   // DWORD fre_clust;
    frs = f_getfree("1:",&fre_clust,&fs);  //Available Space = (free_clust * 64)/2 KB
    if(!frs)
    FreeClust = (fre_clust * fs->csize)>>1;
    
    osDelay(10);
    return FreeClust;
}
#endif
/* USER CODE BEGIN Application */

/* USER CODE END Application */
