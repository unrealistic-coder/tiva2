//// C std headers
//#include <stdbool.h>
//#include <stdint.h>
//#include <string.h>
//
//// Tiva C Drivers
//#include "inc/hw_memmap.h"
//#include "driverlib/gpio.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/ssi.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
//#include "driverlib/interrupt.h"
//#include "inc/hw_ssi.h"
//#include "inc/hw_ints.h"
//#include "inc/hw_types.h"
//#include "inc/hw_gpio.h"
//#include "SST26VF032B.h"
//
////static volatile bool g_bIntFlag = false;
////
////
////uint32_t ui32SysClock;
////
////// DEBUGGING PURPOSE
////uint8_t dummy[3] = {0};
////uint8_t get[6] = {1,1,1,1,1,1};
//uint8_t flag[6] = {2,2,2,2,2,2};
//
//
////void
////Timer0BIntHandler(void)
////{
////    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
////    g_bIntFlag = true;
////
////}
////
//////void
//////SSI3IntHandler(void){
//////    SSIIntClear(SSI3_BASE, SSI_TXEOT);
//////    g_eotInt = true;
//////}
////
////void Timer_Init(){
////    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
////    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT);
////    IntMasterEnable();
////    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
////    IntEnable(INT_TIMER0B);
////}
////
////void delay_ns(uint16_t value){
////    // min 100 ns
////    uint16_t load_value = value*ui32SysClock;
////    TimerLoadSet(TIMER0_BASE, TIMER_B, load_value); // 100 ns delay for 80MHz clock
////    TimerEnable(TIMER0_BASE, TIMER_B);
////    while(!g_bIntFlag);
////}
////
/////* Initializer Functions */
////
////void Clock_Init(){
////    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_CFG_VCO_160),80000000);
////}
////
////void Led_Init(){
////    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
////    GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, GPIO_PIN_0 | GPIO_PIN_1);
////}
////
////void SPI_Init(){
////        /*
////         * 360 NW uses the SSI3 Module of TM4CNCPDT controller
////         * By default only SPI mode can be used in SST26VF032B SPI flash
////         * SST26VF032B/SST26VF032BA -> MAX CLOCK 104 MHz / 80 MHz
////         * Normal Read - 40 Mhz
////         */
////
//////        Clock_Init();
//////        Led_Init();
//////        Timer_Init();
////        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);
////        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOQ)); //wait for enable
////        SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
////        while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI3)); //wait for enable
////        GPIOPinConfigure(GPIO_PQ0_SSI3CLK);
////        GPIOPinConfigure(GPIO_PQ1_SSI3FSS);
////        GPIOPinConfigure(GPIO_PQ2_SSI3XDAT0); //Tx
////        GPIOPinConfigure(GPIO_PQ3_SSI3XDAT1); //Rx
////        GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_1);
////        GPIOPinTypeSSI(GPIO_PORTQ_BASE,GPIO_PIN_0|GPIO_PIN_2|GPIO_PIN_3);
//////        SSIAdvModeSet(SSI3_BASE, SSI_ADV_MODE_READ_WRITE);
////        SSIConfigSetExpClk(SSI3_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER, 1000000, 8);
//////        IntMasterEnable();
//////        SSIIntEnable(SSI3_BASE, SSI_TXEOT);
//////        IntEnable(INT_SSI3);
////        SSIEnable(SSI3_BASE);
////}
////
////unsigned char* TEST_SingleByte_WriteRead(){
////
////    unsigned char character = 'A';
////    static unsigned char buffer;
////    PageProgram(0x3FE000, 1, &character);
////    wait();
////    ReadFlash(0x3FE000, 1, &buffer, false);
////
////    if (buffer == character) {
////        flag[0] = 1;
////    } else {
////        flag[0] = 0;
////    }
////    return buffer;
////
////}
////
////unsigned char* TEST_MultiByte_WriteRead(){
////    const char* string = "VASUdevan";
////    static unsigned char buffers[20];
////    PageProgram(0x3FC000, strlen(string), string);
////    wait();
////    ReadFlash(0x3FC000, strlen(string), buffers, true);
////
////    buffers[strlen(string)] = '\0';
////    if (!strcmp(string, (const char*)buffers)) {
////        flag[1] = 1;
////    } else {
////        flag[1] = 0;
////    }
////    return buffers;
////}
//
//uint8_t TEST_Sector_Write(unsigned char* buff)
//{
//        const char data[4096] = {0};
////        static unsigned char buff[4096];
//        memset(buff, 2, 4096);
//        uint32_t sector = 0;
//        uint8_t count = 1;
//        // Convert sector to byte address
//        uint32_t address = sector * SECTOR_SIZE;
//        uint32_t i = 0, j = 0;
//        uint32_t sector_address = 0, page_address = 0;
//
//        // SECTOR WRITE
//        for( i = 0; i < count; i++){
//            sector_address = (address + (i * SECTOR_SIZE));
//            if(!SectorErase(sector_address)){
//                return 0;
//            }
//            //sector is divided into 16 pages
//            for(j = 0; j < 16; j++){
//                //add 256 byte to move to next page and do page program
//                page_address = (address + (j * 256));
//                if(!PageProgram(page_address, 256, data + (j*256))){
//                    return 0;
//                }
//            }
//        }
//        wait();
//
//        ReadFlash(address, 4096, buff, true);
//        if (memcmp(data, buff, 4096) == 0) {
//            flag[5] = 1;  // Success
//        } else {
//            flag[5] = 0;  // Failure
//        }
//
//        return 1;
//}
//
//
//int main(void)
//{
//    Clock_Init();
//    Led_Init();
//    SPI_Init();
////    Timer_Init();
//    ResetFlash();
//    // NOTE: To protect BPR register and unlock write protect
//    GlobalWriteProtectUnlock();
//    LockBPR();
//
//    uint32_t id = ReadJEDECID();
//    uint8_t status = ReadStatus();
//    uint8_t config = ReadConfig();
//    uint8_t bpr[10];
//    ReadBPR(bpr);
//
//    SectorErase(0x3FC000);
//    unsigned char byte = TEST_SingleByte_WriteRead();
//    unsigned char* word1 = TEST_MultiByte_WriteRead();
//
//    SectorErase(0x3FC000);
//    uint32_t temp;
//    while(SSIDataGetNonBlocking(SSI3_BASE, &temp));
//
////    unsigned char word2[4096] = {0};
////    flag[3] = ReadFlash(0x3FC000, 4096, word2, true);
////
////    unsigned char* word3 = TEST_MultiByte_WriteRead();
////    flag[4] = TEST_spi_init();
//
////    unsigned char word4[4096];
////    memset(word4, 2, 4096);
////    TEST_Sector_Write(word4);
////    unsigned char* word4 = TEST_Sector_Write();
//
//    //sector write and read
//    while (1);
//
//}

// C std headers
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <SST26VF032B_new.h>

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

//static volatile bool g_bIntFlag = false;


uint32_t ui32SysClock;

// DEBUGGING PURPOSE
//uint8_t dummy[3] = {0};
//uint8_t get[6] = {1,1,1,1,1,1};
uint8_t flag[6] = {2,2,2,2,2,2};


//void
//Timer0BIntHandler(void)
//{
//    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
//    g_bIntFlag = true;
//
//}
//
////void
////SSI3IntHandler(void){
////    SSIIntClear(SSI3_BASE, SSI_TXEOT);
////    g_eotInt = true;
////}
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
//
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
//        Timer_Init();
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
//        IntMasterEnable();
//        SSIIntEnable(SSI3_BASE, SSI_TXEOT);
//        IntEnable(INT_SSI3);
        SSIEnable(SSI3_BASE);
}

unsigned char* TEST_SingleByte_WriteRead(){

    unsigned char character = 'A';
    static unsigned char buffer;
    PageProgram(0x3FE000, 1, &character);
    wait();
    ReadFlash(0x3FE000, 1, &buffer, false);

    if (buffer == character) {
        flag[0] = 1;
    } else {
        flag[0] = 0;
    }
    return buffer;

}

unsigned char* TEST_MultiByte_WriteRead(){
    const char* string = "VASUdevan";
    static unsigned char buffers[20];
    PageProgram(0x3FC000, strlen(string), string);
    wait();
    ReadFlash(0x3FC000, strlen(string), buffers, true);

//    buffers[strlen(string)] = '\0';
//    if (!strcmp(string, (const char*)buffers)) {
//        flag[1] = 1;
//    } else {
//        flag[1] = 0;
//    }
    return buffers;
}

int main(void)
{

    SPI_Init();
    // NOTE: To protect BPR register and unlock write protect
    GlobalWriteProtectUnlock();
    LockBPR();

    uint32_t id = ReadJEDECID();
    uint8_t status = ReadStatus();
    uint8_t config = ReadConfig();
    uint8_t bpr[10];
    ReadBPR(bpr);
    SectorErase(0x3FC000);
    unsigned char* byte = TEST_SingleByte_WriteRead();
    unsigned char* word1 = TEST_MultiByte_WriteRead();

    SectorErase(0x3FC000);
//    uint32_t temp;
//    while(SSIDataGetNonBlocking(SSI3_BASE, &temp));

    unsigned char word2[10];
    ReadFlash(0x3FC000, 9, word2, true);

    unsigned char* word3 = TEST_MultiByte_WriteRead();
    flag[2] = TEST_spi_init();
    while (1);

}

