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
#include "source/diskio.h"
#include "source/ff.h"

//webpage files
#include "fs/old/index.h"
#include "fs/old/styles.h"
#include "fs/old/script.h"

//#include <spi_flash.h>

////webpage files
//#include "fs/AllParameters.h"
//#include "fs/DataLog.h"
//#include "fs/EmailConfig.h"
//#include "fs/EthernetSetting.h"
//#include "fs/FlagSetting.h"
//#include "fs/MainPage.h"
//#include "fs/RTCSetting.h"
//#include "fs/ScheduleSetting.h"
//#include "fs/SerialSetting.h"
//
//#include "fs/CSS/Main.h"
//#include "fs/CSS/Tab.h"
//
//#include "fs/images/blue.h"
//
//#include "fs/Script/AllParameter.h"
//#include "fs/Script/DataLog.h"
//#include "fs/Script/Email.h"
//#include "fs/Script/EthernetSetting.h"
//#include "fs/Script/FlagSetting.h"
//#include "fs/Script/MainPage.h"
//#include "fs/Script/RTCSetting.h"
//#include "fs/Script/ScheduleSetting.h"
//#include "fs/Script/SerialSetting.h"
//#include "fs/Script/WebSocket.h"
//
//#include "fs_deflate_array/AllParameters_deflated_shtml.h"
//#include "fs_deflate_array/DataLog_deflated_shtml.h"
//#include "fs_deflate_array/EmailConfig_deflated_shtml.h"
//#include "fs_deflate_array/EthernetSetting_deflated_shtml.h"
//#include "fs_deflate_array/FlagSetting_deflated_shtml.h"
//#include "fs_deflate_array/MainPage_deflated_shtml.h"
//#include "fs_deflate_array/RTCSetting_deflated_shtml.h"
//#include "fs_deflate_array/ScheduleSetting_deflated_shtml.h"
//#include "fs_deflate_array/SerialSetting_deflated_shtml.h"
//
//#include "fs_deflate_array/CSS/Main_deflated_css.h"
//#include "fs_deflate_array/CSS/Tab_deflated_css.h"
//
//#include "fs_deflate_array/images/blue_deflated_jpg.h"
//
//#include "fs_deflate_array/Script/AllParameter_deflated_js.h"
//#include "fs_deflate_array/Script/DataLog_deflated_js.h"
//#include "fs_deflate_array/Script/Email_deflated_js.h"
//#include "fs_deflate_array/Script/EthernetSetting_deflated_js.h"
//#include "fs_deflate_array/Script/FlagSetting_deflated_js.h"
//#include "fs_deflate_array/Script/MainPage_deflated_js.h"
//#include "fs_deflate_array/Script/RTCSetting_deflated_js.h"
//#include "fs_deflate_array/Script/ScheduleSetting_deflated_js.h"
//#include "fs_deflate_array/Script/SerialSetting_deflated_js.h"
//#include "fs_deflate_array/Script/WebSocket_deflated_js.h"

#include "fs_gzip_array/AllParameters_shtml.h"
#include "fs_gzip_array/DataLog_shtml.h"
#include "fs_gzip_array/EmailConfig_shtml.h"
#include "fs_gzip_array/EthernetSetting_shtml.h"
#include "fs_gzip_array/FlagSetting_shtml.h"
#include "fs_gzip_array/MainPage_shtml.h"
#include "fs_gzip_array/RTCSetting_shtml.h"
#include "fs_gzip_array/ScheduleSetting_shtml.h"
#include "fs_gzip_array/SerialSetting_shtml.h"

#include "fs_gzip_array/CSS/Main_css.h"
#include "fs_gzip_array/CSS/Tab_css.h"

#include "fs_gzip_array/images/blue_jpg.h"

#include "fs_gzip_array/Script/AllParameter_js.h"
#include "fs_gzip_array/Script/DataLog_js.h"
#include "fs_gzip_array/Script/Email_js.h"
#include "fs_gzip_array/Script/EthernetSetting_js.h"
#include "fs_gzip_array/Script/FlagSetting_js.h"
#include "fs_gzip_array/Script/MainPage_js.h"
#include "fs_gzip_array/Script/RTCSetting_js.h"
#include "fs_gzip_array/Script/ScheduleSetting_js.h"
#include "fs_gzip_array/Script/SerialSetting_js.h"
#include "fs_gzip_array/Script/WebSocket_js.h"

#include "fs/tiva/404.htm.h"
#include "fs/tiva/about.htm.h"
#include "fs/tiva/block.htm.h"
#include "fs/tiva/block.jpg.h"
#include "fs/tiva/ek-tm4c1294xl.jpg.h"
#include "fs/tiva/favicon.ico.h"
#include "fs/tiva/index.htm.h"
#include "fs/tiva/javascript.js.h"
#include "fs/tiva/overview.htm.h"
#include "fs/tiva/styles.css.h"
#include "fs/tiva/ti.jpg.h"
#include "fs/tiva/tm4c.jpg.h"
//
//static volatile bool g_bIntFlag = false;
//uint32_t ui32SysClock;
//
//// DEBUGGING PURPOSE
//uint8_t dummy[3] = {0};
//uint8_t get[6] = {1,1,1,1,1,1};
uint8_t flag[6] = {2,2,2,2,2,2};
//
//
//void
//Timer0BIntHandler(void)
//{
//    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//    g_bIntFlag = true;
//}
//
//void Timer_Init(){
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
//    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT);
//    IntMasterEnable();
//    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//    IntEnable(INT_TIMER0B);
//}
//
//void delay_ns(uint16_t value){
//    // min 100 ns
//    uint16_t load_value = value*ui32SysClock;
//    TimerLoadSet(TIMER0_BASE, TIMER_B, load_value); // 100 ns delay for 80MHz clock
//    TimerEnable(TIMER0_BASE, TIMER_B);
//    while(!g_bIntFlag);
//}


void TEST_SingleByte_WriteRead(){

    unsigned char character = 'A';
    unsigned char buffer;
    PageProgram(0x3FE000, 1, &character);
    wait();
    ReadFlash(0x3FE000, 1, &buffer, false);

    if (buffer == character) {
        flag[0] = 1;
    } else {
        flag[0] = 0;
    }

}

void TEST_MultiByte_WriteRead(){
    const char* string = "VASUdevan";
    unsigned char buffers[20];
    PageProgram(0x3FF000, strlen(string), string);
    wait();
    ReadFlash(0x3FF000, strlen(string), buffers, true);

    buffers[strlen(string)] = '\0';
    if (!strcmp(string, (const char*)buffers)) {
        flag[1] = 1;
    } else {
        flag[1] = 0;
    }
}


void TEST_Sector_Write()
{

        uint32_t sector = 0;
        uint8_t count = 2;
        unsigned char data[4096] = {0};


        unsigned char buff[4096];
        memset(buff, 2, 4096);

        // Convert sector to byte address
        uint32_t sector_address = sector * 4096;
        uint32_t i = 0, j = 0;
        uint32_t page_address = 0;

        // SECTOR WRITE
        for( i = 0; i < count; i++){

            sector_address = (sector_address + (i * 4096));
            SectorErase(sector_address);
            ReadFlash(sector_address, 4096, buff, true);

            srand(2+i);
            int k = 0;
            for(k=0;k<4096;k++){
                data[k]=rand()%256;
            }

            //sector is divided into 16 pages
            for(j = 0; j < 16; j++){
                //add 256 byte to move to next page and do page program
                page_address = (sector_address + (j * 256));
                if(!PageProgram(page_address, 256, data + (j*256)))
                    flag[3] = 0; //fail
            }

            wait();

            if(!ReadFlash(sector_address, 4096, buff, true))
                flag[4] = 0; // fail
            if (memcmp(data, buff, 4096) == 0) {
                flag[2] = 1;  // Success
            } else {
                flag[2] = 0;  // Failure
            }
        }
}

void TEST_disk_write(){

    srand(2);
    int k = 0;
    BYTE* buff;
    BYTE *rbuff;
    for(k = 0; k < 12288; k++){
        buff[k] = rand() % 256;
    }
    disk_write(0, buff, 5, 3);

    disk_read(0, rbuff, 5, 3);

    if(memcmp(rbuff, buff, 4096) == 0){
        if(memcmp(rbuff+4096, buff+4096, 4096) == 0)
            if(memcmp(rbuff+8192, buff+8192, 4096) == 0)
                flag[5] = 1;
    } else {
        flag[5] = 0;
    }

}

FATFS fs;          // Filesystem object
FIL file;          // File object
static FRESULT res;       // FatFs function result
UINT bw, br;       // Bytes written/read
char buffer[128];  // Buffer for reading/writing

void format_disk(void) {
    FRESULT res;
    MKFS_PARM fs_params = {FM_FAT | FM_SFD, 1, 1, 1, 4096}; // FAT12/16/32
    BYTE work[FF_MAX_SS]; // Work buffer (aligned with sector size)

    res = f_mkfs("0:", &fs_params, work, sizeof(work));
    if (res == FR_OK) {
        printf("Disk formatted successfully.\n");
    } else {
        printf("Error formatting disk: %d\n", res);
    }

}

void fatfs_example(void) {
    // Step 1: Mount the filesystem
    res = f_mount(&fs, "", 1); // Mount on logical drive "", auto mount
    if (res != FR_OK) {
        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem mounted successfully.\n");

    // Step 2: Create and write to a file
    res = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("Failed to open file! Error: %d\n", res);
        return;
    }
    printf("File opened successfully for writing.\n");

    const char *data = "Hello, FatFs!"
            "Author of this code: Vasudevan";

    res = f_write(&file, data, strlen(data), &bw);
    if (res != FR_OK || bw != strlen(data)) {
        printf("Failed to write to file! Error: %d\n", res);
        f_close(&file);
        return;
    }
    printf("Data written to file: %s\n", data);

    f_close(&file);

    //append
    res = f_open(&file, "test.txt", FA_WRITE | FA_OPEN_APPEND);
    if (res != FR_OK) {
        printf("Failed to open file! Error: %d\n", res);
        return;
    }
    printf("File opened successfully for writing.\n");

    const char *data2 =
            "Welcome to SPI FLASH memory.";

    res = f_write(&file, data2, strlen(data2), &bw);
    if (res != FR_OK || bw != strlen(data2)) {
        printf("Failed to append to file! Error: %d\n", res);
        f_close(&file);
        return;
    }
    printf("Data append to file: %s\n", data2);

    f_close(&file);

    // Step 3: Read the file
    res = f_open(&file, "test.txt", FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file for reading! Error: %d\n", res);
        return;
    }
    printf("File opened successfully for reading.\n");

    memset(buffer, 0, sizeof(buffer));
    res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
    if (res != FR_OK) {
        printf("Failed to read from file! Error: %d\n", res);
        f_close(&file);
        return;
    }
    printf("Data read from file: %s\n", buffer);
    f_close(&file);

    // Step 4: Unmount the filesystem
    res = f_mount(NULL, "", 0); // Unmount
    if (res != FR_OK) {
        printf("Failed to unmount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem unmounted successfully.\n");
}

void fatfs_http(void) {
    // Step 1: Mount the filesystem
    res = f_mount(&fs, "", 1); // Mount on logical drive "", auto mount
    if (res != FR_OK) {
        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem mounted successfully.\n");

    // Step 2: Create and write to a file
    res = f_open(&file, "index.htm", FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("Failed to open file! Error: %d\n", res);
        return;
    }
    printf("File opened successfully for writing.\n");

    const char *data =
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<title>Output Box Example</title>"
        "<link rel=\"stylesheet\" type=\"text/css\" href=\"styles.css\"/>"
        "<script src=\"javascript.js\"></script>"
        "</head>"
        "<body>"
        "<div class=\"container\">"
        "<div class=\"input-container\">"
        "<input type=\"text\" id=\"inputBox\" placeholder=\"Enter Command\" onkeydown=\"checkEnter(event)\">"
        "<button id=\"sendButton\" onclick=\"sendMessage()\">Send</button>"
        "<button id=\"connectButton\" onClick=\"initWebSockUpgrade()\">Connect</button>"
        "</div>"
        "<div class=\"output-box\" id=\"outputBox\">"
        "</div>"
        "</div>"
        "</body>"
        "</html>";


    res = f_write(&file, data, strlen(data), &bw);
    if (res != FR_OK || bw != strlen(data)) {
        printf("Failed to write to file! Error: %d\n", res);
        f_close(&file);
        return;
    }
    printf("Data written to file: %s\n", data);
    f_close(&file);

    // Step 3: Read the file
    res = f_open(&file, "index.htm", FA_READ);
    if (res != FR_OK) {
        printf("Failed to open file for reading! Error: %d\n", res);
        return;
    }
    printf("File opened successfully for reading.\n");

/* Chunked read */
//    memset(buffer, 0, sizeof(buffer));
//       do {
//           res = f_read(&file, buffer, sizeof(buffer) - 1, &br); // Read a chunk
//           if (res != FR_OK) {
//               printf("Failed to read from file! Error: %d\n", res);
//               f_close(&file);
//               return;
//           }
//
//           buffer[br] = '\0'; // Null-terminate the chunk
//           printf("%s\n", buffer); // Print the chunk (or process it as needed)
//       } while (br > 0); // Continue until no more data is read

/* chunked heap read*/
//       DWORD file_size = f_size(&file); // Get file size
       char *hbuffer = (char *)malloc(2048); // Allocate memory for the file
       if (hbuffer == NULL) {
           printf("Failed to allocate heap memory.\n");
           return;
       }
       do {
           res = f_read(&file, hbuffer, 128, &br); // Read a chunk
           if (res != FR_OK) {
               printf("Failed to read from file! Error: %d\n", res);
               free(hbuffer);
               f_close(&file);
               return;
           }

//           hbuffer[br] = '\0'; // Null-terminate the chunk
           printf("%s\n", hbuffer); // Print the chunk (or process it as needed)
       } while (br > 0); // Continue until no more data is read
       free(hbuffer);
/* heap buffer read */
//    DWORD file_size = f_size(&file); // Get file size
//    char *heap_buffer = (char *)malloc(file_size + 1); // Allocate memory for the file
//    if (heap_buffer == NULL) {
//        printf("Failed to allocate heap memory.\n");
//        return;
//    }
//    res = f_read(&file, heap_buffer, file_size, &br);
//    if (res != FR_OK || br != file_size) {
//        printf("Failed to read entire file! Error: %d\n", res);
//        free(heap_buffer); // Free allocated memory
//        f_close(&file);
//        return;
//    }

/* NORMAL CODE */
//    memset(buffer, 0, sizeof(buffer));
//    res = f_read(&file, buffer, sizeof(buffer) - 1, &br);
//    if (res != FR_OK) {
//        printf("Failed to read from file! Error: %d\n", res);
//        f_close(&file);
//        return;
//    }
//    printf("Data read from file: %s\n", buffer);

    f_close(&file);

    // Step 4: Unmount the filesystem
    res = f_mount(NULL, "", 0); // Unmount
    if (res != FR_OK) {
        printf("Failed to unmount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem unmounted successfully.\n");
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

static inline void send_chunk(int sock, const char* data, size_t datalen) {
    char chunk_size[10];
    snprintf(chunk_size, sizeof(chunk_size), "%X\r\n", (unsigned int)datalen);
//    send(sock, chunk_size, strlen(chunk_size),0); // Send chunk size
//    send_all(sock, data, datalen); // Send chunk data
    size_t sent = 0;
    while (sent < datalen) {
        int sent_flag = send(sock, data + sent, datalen - sent, 0);
        if (sent_flag < 0) {
            perror("send failed");
            return;
        }
        if(sent_flag == 0) {
            perror("send failed");
            return;
        }
        sent += sent_flag;
    }
//    send(sock, data, datalen, 0);
    send(sock, "\r\n", 2, 0); // Send CRLF after chunk data
}

unsigned int send_file_web(const char* filename){
    int sum = 0;
    res = f_open(&file, filename, FA_READ);
    if(res != FR_OK){
        printf("failed to open, %s\n",filename);
        return 0;
    }
    printf("File opened successfully for reading.\n");

    // chunk size of 2048 //TODO: define macro for chunk size
    char* hbuff = (char*)malloc(sizeof(char)*2048);
    if(hbuff == NULL){
        printf("failed to allocate heap memory\n");
        return 0;
    }
    do{
        // get chunk
       memset(hbuff, 0, 2048);
       res = f_read(&file, hbuff, 2048, &br); // Read a chunk
       if (res != FR_OK) {
           printf("Failed to read from file! Error: %d\n", res);
           free(hbuff);
           f_close(&file);
           return 0;
       }
       if(br>0){
//           send_chunk(hbuff, br);
//           printf(hbuff);
       }
       sum+=br;
       // hbuff[br] = '\0'; // Null-terminate the chunk
       // printf("%s\n", hbuff); // Print the chunk (or process it as needed)
    } while (br > 0); // Continue until no more data is read
    //end chunk frame
//    send(sock, "0\r\n\r\n", 5, 0);

    // not done
    printf("file sent to web: %s\n", filename);
    printf("filesize = %d\n",sum);
    free(hbuff);
    f_close(&file);
    return 1;
}

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

int store_folder(struct folder* root, char* parent_path) {
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
            printf("Warning: Failed to store subfolder %s\n", root->subfolders[i].folder_name);
        }
    }

    return 0;
}

// files in fs
struct file fs_files[] = {
    {.filename = (unsigned char*)"AllParameters.shtml", .filesize = AllParameters_shtml_SIZE, .filedata = AllParameters_shtml},
    {.filename = (unsigned char*)"DataLog.shtml", .filesize = DataLog_shtml_SIZE, .filedata = DataLog_shtml},
    {.filename = (unsigned char*)"EmailConfig.shtml", .filesize = EmailConfig_shtml_SIZE, .filedata = EmailConfig_shtml},
    {.filename = (unsigned char*)"EthernetSetting.shtml", .filesize = EthernetSetting_shtml_SIZE, .filedata = EthernetSetting_shtml},
    {.filename = (unsigned char*)"FlagSetting.shtml", .filesize = FlagSetting_shtml_SIZE, .filedata = FlagSetting_shtml},
    {.filename = (unsigned char*)"MainPage.shtml", .filesize = MainPage_shtml_SIZE, .filedata = MainPage_shtml},
    {.filename = (unsigned char*)"RTCSetting.shtml", .filesize = RTCSetting_shtml_SIZE, .filedata = RTCSetting_shtml},
    {.filename = (unsigned char*)"ScheduleSetting.shtml", .filesize = ScheduleSetting_shtml_SIZE, .filedata = ScheduleSetting_shtml},
    {.filename = (unsigned char*)"SerialSetting.shtml", .filesize = SerialSetting_shtml_SIZE, .filedata = SerialSetting_shtml}
};

struct file css_files[] = {
    {.filename = (unsigned char*)"Main.css", .filesize = Main_css_SIZE, .filedata = Main_css},
    {.filename = (unsigned char*)"Tab.css", .filesize = Tab_css_SIZE, .filedata = Tab_css}
};

struct file image_files[] = {
    {.filename = (unsigned char*)"blue.jpg", .filesize = blue_jpg_SIZE, .filedata = blue_jpg}
};

struct file script_files[] = {
    {.filename = (unsigned char*)"AllParameter.js", .filesize = AllParameter_js_SIZE, .filedata = AllParameter_js},
    {.filename = (unsigned char*)"DataLog.js", .filesize = DataLog_js_SIZE, .filedata = DataLog_js},
    {.filename = (unsigned char*)"Email.js", .filesize = Email_js_SIZE, .filedata = Email_js},
    {.filename = (unsigned char*)"EthernetSetting.js", .filesize = EthernetSetting_js_SIZE, .filedata = EthernetSetting_js},
    {.filename = (unsigned char*)"FlagSetting.js", .filesize = FlagSetting_js_SIZE, .filedata = FlagSetting_js},
    {.filename = (unsigned char*)"MainPage.js", .filesize = MainPage_js_SIZE, .filedata = MainPage_js},
    {.filename = (unsigned char*)"RTCSetting.js", .filesize = RTCsetting_js_SIZE, .filedata = RTCsetting_js},
    {.filename = (unsigned char*)"ScheduleSetting.js", .filesize = ScheduleSetting_js_SIZE, .filedata = ScheduleSetting_js},
    {.filename = (unsigned char*)"SerialSetting.js", .filesize = SerialSetting_js_SIZE, .filedata = SerialSetting_js},
    {.filename = (unsigned char*)"WebSocket.js", .filesize = WebSocket_js_SIZE, .filedata = WebSocket_js}
};

struct file tiva[] = {
    {.filename = (unsigned char*)"404.html", .filesize = html_404_SIZE, .filedata = html_404},
    {.filename = (unsigned char*)"about.htm", .filesize = about_SIZE, .filedata = about},
    {.filename = (unsigned char*)"block.htm", .filesize = block_SIZE, .filedata = block},
    {.filename = (unsigned char*)"block.jpg", .filesize = block_jpg_SIZE, .filedata = block_jpg},
    {.filename = (unsigned char*)"ek-tm4c1294xl.jpg", .filesize = ek_tm4c1294xl_SIZE, .filedata = ek_tm4c1294xl},
    {.filename = (unsigned char*)"favicon.ico", .filesize = favicon_SIZE, .filedata = favicon},
    {.filename = (unsigned char*)"index.htm", .filesize = index_SIZE, .filedata = index},
    {.filename = (unsigned char*)"javascript.js", .filesize = javascript_SIZE, .filedata = javascript},
    {.filename = (unsigned char*)"overview.htm", .filesize = overview_SIZE, .filedata = overview},
    {.filename = (unsigned char*)"styles.css", .filesize = styles_SIZE, .filedata = styles},
    {.filename = (unsigned char*)"ti.jpg", .filesize = ti_SIZE, .filedata = ti},
    {.filename = (unsigned char*)"tm4c.jpg", .filesize = tm4c_SIZE, .filedata = tm4c}
};

// Define subfolders
struct folder subfolders[] = {
    {
        .folder_name = (unsigned char*)"CSS",
        .file_count = 2,
        .files = css_files,
        .subfolder_count = 0,
        .subfolders = NULL
    },
    {
        .folder_name = (unsigned char*)"images",
        .file_count = 1,
        .files = image_files,
        .subfolder_count = 0,
        .subfolders = NULL
    },
    {
        .folder_name = (unsigned char*)"Script",
        .file_count = 10,
        .files = script_files,
        .subfolder_count = 0,
        .subfolders = NULL
    }
};

// Define the root folder
struct folder root_folder = {
    .folder_name = (unsigned char*)"fsg",
    .file_count = 9,
    .files = fs_files,
    .subfolder_count = 3,
    .subfolders = subfolders
};

struct folder example = {
     .folder_name = (unsigned char*)"tiva",
     .file_count = 12,
     .files = tiva,
     .subfolder_count = 0,
     .subfolders = NULL
};

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);
    printf("opened folder, %s\n",path);/* Open the directory */
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
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

char* get_requested_path(char* request) {
    // GET /fs/index.html HTTP/1.1
    // Host: 192.168.1.100
    // User-Agent: Mozilla/5.0
    // Accept: text/html

    char* start = strstr(request, "GET ");
    if (!start) return NULL;  // Not a GET request

    start += 4;  // Move past "GET "
    char* end = strstr(start, " ");
    if (!end) return NULL;  // Invalid request format

    *end = '\0';  // Terminate string at the first space after the path
    return start;  // Return the requested path (e.g., "/fs/index.html")
}

typedef struct
{
  const char *name;
  uint8_t shtml;
} default_filename;

const default_filename g_psDefaultFilenames[] = {
//  {"/MainPage.shtml", true },
  {"/index.htm", false },
  {"/index.shtml", true },
  {"/index.ssi", true },
  {"/index.shtm", true },
  {"/index.html", false }
};

int main(void)
{

//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)); //wait for enable
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
//    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI3)); //wait for enable
//    GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
//    GPIOPinConfigure(GPIO_PQ1_SSI3FSS);
//    GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0); //Tx
//    GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1); //Rx
////    GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
//    GPIOPinTypeSSI(GPIO_PORTQ_BASE,GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_1);
////
////    // NOTE: To protect BPR register and unlock write protect
////    GlobalWriteProtectUnlock();
////    LockBPR();
//    uint32_t ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_CFG_VCO_160),80000000);
//    SPIFlashInit(SSI3_BASE, ui32SysClock, 1000000);
//    uint8_t pui8ManufacturerID;
//    uint16_t pui16DeviceID;
//    SPIFlashReadID(SSI3_BASE, &pui8ManufacturerID, &pui16DeviceID);
//    printf("manufacturer ID: 0x%2X\nDevice ID: 0x%2X\n, ",pui8ManufacturerID,pui16DeviceID);
////    uint8_t status = ReadStatus();
//    uint8_t config = ReadConfig();
//    uint8_t bpr[10];
//    ReadBPR(bpr);
//    TEST_SingleByte_WriteRead();
//    TEST_MultiByte_WriteRead();
//    TEST_Sector_Write();
//    SectorErase(0x3FC000);
//    uint32_t temp;
//    while(SSIDataGetNonBlocking(SSI3_BASE, &temp));
//
//    unsigned char word2[10];
//    ReadFlash(0x3FC000, 9, word2, true);

//    TEST_disk_write();
//    format_disk();
//    fatfs_example();
//    fatfs_http();


//    ////////////////////////WEBSOCKET////////////////////////////////////////////////////////
////    format_disk();
    res = f_mount(&fs, "0:", 1); // Mount on logical drive "", auto mount
    if (res != FR_OK) {
        printf("Failed to mount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem mounted successfully.\n");
////    res = f_mount(&fs, "", 1);  // Try to mount the filesystem
////    if(res != FR_OK)
////        printf("Failed to mount\n");
////    if (res == FR_NO_FILESYSTEM) {
////        // No filesystem detected, so create one
////        format_disk();
////
////        // Mount again after formatting
////        res = f_mount(&fs, "", 1);
////        if (res != FR_OK) {
////            printf("Failed to mount filesystem after formatting, error: %d\n", res);
////            return -1;
////        }
////        printf("Mounted\n");
////    }
//
////    //operations
////    write_file("index.htm", INDEX, INDEX_SIZE);
////    write_file("styles.css", STYLES, STYLES_SIZE);
////    write_file("script.js", SCRIPT, SCRIPT_SIZE);
////    const char* pcName = g_psDefaultFilenames[0].name;
////    read_file(pcName);
////    read_file("/tiva/404.htm");
//////    read_file("script.js");
////
////    send_file_web("index.htm", INDEX_SIZE);
////    send_file_web("styles.css", STYLES_SIZE);
////    send_file_web("script.js", SCRIPT_SIZE);
////    // Store the folder structure
////    delete_folder("\tiva");
//    if (store_folder(&root_folder, "") != 0) {
//        printf("Failed to store folder structure.\n");
//    } else {
//        printf("Folder structure stored successfully.\n");
//    }
////    char* path="/MainPage.shtml";
////    char root[100];
////    snprintf(root, sizeof(root), "/fs%s", path);
////    printf("Opening file: %s\n", root);
////
////    if(!send_file_web(root)){
////        printf("file to web send failed");
////    }
////    printf("File opened successfully for reading.\n");
////    send_file_web(root);
    char buff[100];
    strcpy(buff, "/");
    res = scan_files(buff);
    if (res != FR_OK) {
        printf("Failed to scan dir");
//        read_file(buff);
    }
    res = f_mount(NULL, "0:", 0); // Unmount
    if (res != FR_OK) {
        printf("Failed to unmount filesystem! Error: %d\n", res);
        return;
    }
    printf("Filesystem unmounted successfully.\n");
    ////////////////////////////////////////////////////////////////////////////////////////

    while (1);

}
