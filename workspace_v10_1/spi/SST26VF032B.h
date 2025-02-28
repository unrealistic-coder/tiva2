/*
 * SST26VF032B.h
 *
 *  Created on: 25-Dec-2024
 *      Author: 14169
 */

#ifndef SST26VF032B_H_
#define SST26VF032B_H_

#include <stdint.h>
#include <stdbool.h>

//device info
#define SECTOR_SIZE  4096
#define LAST_SECTOR  1023
#define FIRST_SECTOR 0
#define SECTOR_COUNT 1024

//Configuration Commands
#define NOP       0x00      /*No Operation*/
#define RSTEN     0x66      /*Reset Enable*/
#define RST       0x99      /*Reset Memory*/
#define RDSR      0x05      /*Read STATUS Register*/
#define WRSR      0x01      /*Write STATUS Register*/
#define RDCR      0x35      /*Read Configuration Register*/

//Read Commands
#define READ      0x03      /*Read Memory*/
#define FREAD     0x0B      /*Read Memory at Higher Speed*/

//Write Commands
#define WREN      0x06      /*Write Enable*/
#define WRDI      0x04      /*Write Disable*/

//Burst Commands
#define SB        0xC0      /*Set Burst Length*/
#define RBSPI     0xEC      /*SPI Read Burst with Wrap*/

//Burst length
#define BYTE_8    0x00      /*8 bytes*/
#define BYTE_16   0x01      /*16 bytes*/
#define BYTE_32   0x02      /*32 bytes*/
#define BYTE_64   0x03      /*64 bytes*/

//Erase Commands
#define SE        0x20      /*Erase 4 Kbytes of Memory Array - SECTOR ERASE*/
#define BE        0xD8      /*Erase 64, 32 or 8 Kbytes of Memory Array - BLOCK ERASE*/
#define CE        0xC7      /*Erase Full Array*/
#define PP        0x02      /*Page Program*/

//Suspend Commands
#define WRSU      0xB0      /*Suspends Program Or Erase*/
#define WRRE      0x30      /*Resumes Program Or Erase*/

//Protection Commands
#define RBPR      0x72      /*Read Block Protection Register*/
#define WBPR      0x42      /*Write Block Protection Register*/
#define LBPR      0x8D      /*Lock Down Block Protection Register*/
#define nVWLDR    0xE8      /*non-Volatile Write Lock Down Register*/
#define ULBPR     0x98      /*Global Block Protection Unlock*/

//Security ID Commands
#define RSID      0x88      /*Read Security ID*/
#define PSID      0xA5      /*Program User Security ID area*/
#define LSID      0x85      /*Lockout Security ID Programming*/

#define FLASH_ID  0x9F      /*Manufacturer device ID*/

#define DUMMY     0xFF      /*Dummy byte for Receive and Provide clock*/

//Status bits
#define STATUS_BUSY  0x01
#define STATUS_WEL   0x02
#define STATUS_WSE   0x04
#define STATUS_WSP   0x08
#define STATUS_WPLD  0x10
#define STATUS_SEC   0x20

//Configuration bits
#define CONFIG_IOC   0x02
#define CONFIG_BPNV  0x08
#define CONFIG_WPEN  0x80

//Macro Functions
#define CHIP_SELECT(state) GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_1, (state) ? GPIO_PIN_1 : 0)
#define LED_ON() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , GPIO_PIN_0 | GPIO_PIN_1)
#define LED_OFF() GPIOPinWrite( GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1 , 0)

uint32_t ui32SysClock;

void Clock_Init();
void Led_Init();
void SPI_Init();

//Flash IO Functions
uint32_t ReadJEDECID();
uint8_t ReadStatus();
uint8_t ReadConfig();
bool ReadFlash(uint32_t addr, uint32_t bytes, uint8_t* buffer, bool flash);
void ResetFlash();
void wait();
void ReadBurst(uint8_t len);
void LockBPR();
void ReadBPR();
void WriteEnable(bool state);
//void writeDisable();
void WriteStatus(uint8_t config);
void GlobalWriteProtectUnlock();
bool PageProgram(uint32_t addr, uint32_t size, uint8_t* data);

bool SectorErase(uint32_t addr);
void BlockErase(uint32_t addr);
void ChipErase(void);

uint8_t resume();
uint8_t suspend();

void SetBurst(uint8_t bytes);
void ReadBurst(uint8_t bytes);
bool TEST_spi_init();

#endif /* SST26VF032B_H_ */
