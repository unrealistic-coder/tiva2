/*
 * SST26VF032B.c
 *
 *  Created on: 25-Dec-2024
 *      Author: 14169
 */
#include <stdbool.h>
#include <stdint.h>
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

/* Initializer Functions */

void Clock_Init(){
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_CFG_VCO_160),80000000);
}

void Led_Init(){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
}

void SPI_Init(){
        /*
         * 360 NW uses the SSI3 Module of TM4CNCPDT controller
         * By default only SPI mode can be used in SST26VF032B SPI flash
         * SST26VF032B/SST26VF032BA -> MAX CLOCK 104 MHz / 80 MHz
         * Normal Read - 40 Mhz
         */
        Clock_Init();
        Led_Init();

        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)); //wait for enable
        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI3)); //wait for enable
        GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
        GPIOPinConfigure(GPIO_PQ1_SSI3FSS);
        GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0); //Tx
        GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1); //Rx
        GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
        GPIOPinTypeSSI(GPIO_PORTQ_BASE,GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3);
//        SSIAdvModeSet(SSI3_BASE, SSI_ADV_MODE_READ_WRITE);
        SSIConfigSetExpClk(SSI3_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, 1000000, 8);
        SSIEnable(SSI3_BASE);

        // flash precautionary cmds
        GlobalWriteProtectUnlock();
        LockBPR();
}

bool TEST_spi_init(){
    uint32_t id = ReadJEDECID();
    id &= 0x00FFFFFF;
    return (id == 0x00BF2642)? true : false;
}

static inline void send(uint32_t base, uint8_t cmd) {
    uint32_t discard;
    SSIDataPut(base, cmd);
    while (SSIBusy(base));
    SSIDataGet(base, &discard); // discard dummy byte received while sending command
}

static inline void receive(uint32_t base, uint8_t* address){
    uint32_t tempData; // Temporary variable to hold the dummy received byte
    SSIDataPut(base, DUMMY);
    while (SSIBusy(base));
    SSIDataGet(base, &tempData);
    *address = (uint8_t)tempData; // Extract the lower 8 bits
}

uint32_t ReadJEDECID(void) {
    uint8_t manufacturerID, memoryType, capacity;

    CHIP_SELECT(0);

    uint32_t temp;
    while (SSIDataGetNonBlocking(SSI3_BASE, &temp));

    // Send command and discard dummy byte
    send(SSI3_BASE, FLASH_ID);
    receive(SSI3_BASE, &manufacturerID);
    receive(SSI3_BASE, &memoryType);
    receive(SSI3_BASE, &capacity);

    CHIP_SELECT(1);

    // Combine the JEDEC ID into a single 24-bit value
    uint32_t jedecID = ((manufacturerID & 0xFF) << 16) |
                       ((memoryType & 0xFF) << 8) |
                       (capacity & 0xFF);

    return jedecID;
}


uint8_t ReadStatus()
{
    uint8_t status;
    uint32_t temp;

    CHIP_SELECT(0);

    while(SSIDataGetNonBlocking(SSI3_BASE, &temp)); // clear RX FIFO

    send(SSI3_BASE, RDSR);
    receive(SSI3_BASE, &status);

    CHIP_SELECT(1);

    return status;
}

uint8_t ReadConfig()
{
    uint8_t config;
    uint32_t temp;

    CHIP_SELECT(0);

    while(SSIDataGetNonBlocking(SSI3_BASE, &temp)); // clear RX FIFO

    send(SSI3_BASE, RDCR);
    receive(SSI3_BASE, &config);

    CHIP_SELECT(1);

    return config;
}

void wait(){
    // Wait until the flash is no longer busy
    uint32_t status = 0;
    do {
        status = ReadStatus();
    } while(status & 0x01);                  // Check the BUSY bit (bit 0)
}

bool ReadFlash(uint32_t addr, uint32_t bytes, uint8_t* buffer, bool fast){
    // read until CE is asserted high.
    CHIP_SELECT(0);
    uint32_t i = 0;
    // send read cmd frames
    SSIDataPut(SSI3_BASE, fast ? FREAD : READ);
    SSIDataPut(SSI3_BASE, (addr >> 16) & 0xFF);
    SSIDataPut(SSI3_BASE, (addr >> 8) & 0xFF);
    SSIDataPut(SSI3_BASE, addr & 0xFF);
    if(fast){
        SSIDataPut(SSI3_BASE, DUMMY); //send dummy
    }
    while(SSIBusy(SSI3_BASE)); // wait for tx

    uint32_t temp;
    while(SSIDataGetNonBlocking(SSI3_BASE, &temp));

    for(i = 0; i < bytes; i++){
        receive(SSI3_BASE, &buffer[i]);
    }
    CHIP_SELECT(1);

    /*
     *  The End-of-Transmission (EOT) interrupt indicates that
     *  the data has been transmitted completely
     *  and is only valid for Master mode devices/operations.
     *  The interrupt can also indicate that read
     *  data is ready immediately, without waiting for the
     *  receive FIFO time-out period to complete.
     */
    if(SSIIntStatus(SSI3_BASE, 0) & SSI_TXEOT){
        return true;
    }
    return false;
}

void ReadBPR(uint8_t* bpr){

    CHIP_SELECT(0);
    uint8_t i;
    send(SSI3_BASE, RBPR);
    for(i = 0; i < 10; i++){
        receive(SSI3_BASE, &bpr[i]);
    }

    CHIP_SELECT(1);
}


void ResetFlash(){
    //reset enable
    CHIP_SELECT(0);
    send(SSI3_BASE, RSTEN);
    CHIP_SELECT(1);

    // reset
    CHIP_SELECT(0);
    send(SSI3_BASE, RST);
    CHIP_SELECT(1);
}



void ReadBurst(uint8_t len){

}

/*
    WREN - enabled for below commands

    Sector Erase, Block Erase, Chip Erase, Page Program,
    Program Security ID, Lockout Security ID, Write Block
    Protection Register, Lock-Down Block Protection Register,
    nonvolatile Write Lock Lock-Down Register,
    SPI Quad Page program, and Write STATUS Register.
*/

inline void WriteEnable(bool state){
    CHIP_SELECT(0);
    send(SSI3_BASE, (state)? WREN : WRDI);
    CHIP_SELECT(1);
}

bool PageProgram(uint32_t addr, uint32_t size, uint8_t* data){
    uint32_t i = 0;
    WriteEnable(1);
    CHIP_SELECT(0);

    SSIDataPut(SSI3_BASE, PP);
    SSIDataPut(SSI3_BASE, (addr >> 16) & 0xFF);
    SSIDataPut(SSI3_BASE, (addr >> 8) & 0xFF);
    SSIDataPut(SSI3_BASE, addr & 0xFF);

    SSIDataGet(SSI3_BASE, &i); // clear dummy byte
    size = (size>256)? 256 : size;
    for(i = 0;i < size; i++){
        SSIDataPut(SSI3_BASE, data[i]);
        while(SSIBusy(SSI3_BASE)); // wait for tx
    }

    CHIP_SELECT(1);

    wait();
    if(SSIIntStatus(SSI3_BASE, 0) & SSI_TXEOT){
        return true;
    }
    return false;
}

//void writeDisable(){
//    CHIP_SELECT(0);
//    SSIDataPut(SSI3_BASE, WRDI);
//    while(SSIBusy(SSI3_BASE)); // wait for tx
//    CHIP_SELECT(1);
//}



void WriteStatus(uint8_t config){
    //write config register
    WriteEnable(1);
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, WRSR);
    SSIDataPut(SSI3_BASE, 0xFF); //dummy byte
    SSIDataPut(SSI3_BASE, config);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);
}

void GlobalWriteProtectUnlock(){
    WriteEnable(1);
    CHIP_SELECT(0);
    send(SSI3_BASE, ULBPR);
    CHIP_SELECT(1);
}

void LockBPR(){
    WriteEnable(1);
    CHIP_SELECT(0);
    send(SSI3_BASE, LBPR);
    CHIP_SELECT(1);
}



bool SectorErase(uint32_t addr){
    WriteEnable(1);
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, SE);
    SSIDataPut(SSI3_BASE, (addr >> 16) & 0xFF);
    SSIDataPut(SSI3_BASE, (addr >> 8) & 0xFF);
    SSIDataPut(SSI3_BASE, addr & 0xFF);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);

    wait();
    // verify that trasnmit is done by checking tx fifo empty
    return((HWREG(SSI3_BASE + SSI_O_SR) & SSI_SR_TFE) ? true : false);
}

void SetBurst(uint8_t bytes){
    // factory default 8
    CHIP_SELECT(0);
    send(SSI3_BASE, SB);
    switch(bytes){
        case 8:
            send(SSI3_BASE, BYTE_8);
            break;
        case 16:
            send(SSI3_BASE, BYTE_16);
            break;
        case 32:
            send(SSI3_BASE, BYTE_32);
            break;
        case 64:
            send(SSI3_BASE, BYTE_64);
            break;
    }
    CHIP_SELECT(1);
}

/*
// for aligned read access
void ReadBurst(uint8_t bytes){

}
*/

void BlockErase(uint32_t addr){
    WriteEnable(1);
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, BE);
    SSIDataPut(SSI3_BASE, (addr >> 16) & 0xFF);
    SSIDataPut(SSI3_BASE, (addr >> 8) & 0xFF);
    SSIDataPut(SSI3_BASE, addr & 0xFF);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);

    wait();
}

void ChipErase(void){
    WriteEnable(1);
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, CE);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);

    wait();
}

uint8_t suspend(){
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, WRSU);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);

    wait();
    //erase supended
    if(ReadStatus() & STATUS_WSE)
        return 1;
    //prog suspended
    else if(ReadStatus() & STATUS_WSP)
        return 2;
    return 0; //failed
}

uint8_t resume(){
    CHIP_SELECT(0);
    SSIDataPut(SSI3_BASE, WRRE);
    while(SSIBusy(SSI3_BASE)); // wait for tx
    CHIP_SELECT(1);

    wait();
    //erase resume
    if(! (ReadStatus() & STATUS_WSE))
        return 1;
    //prog resume
    else if(! (ReadStatus() & STATUS_WSP))
        return 2;
    return 0; // failed
}

