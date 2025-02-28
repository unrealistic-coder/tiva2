/*
 * I2C_Bus.c
 *
 *  Created on: 23-Aug-2021
 *      Author: rushikesh.sonone
 */

#include "I2C_Bus.h"

void Init_External_EEPROM(void)
{
    ///------------------------------------------------------------///
    //Here FRAM will communicate with micro controller using I2c Protocol
    //// Initializing I2C protocol;
    //// I2C 6 can be use for SEND and RECEIVE Data

    ///------------------------------------------------------------///
    //enable I2C module 6
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C6);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C6));
    ///------------------------------------------------------------///

    ///------------------------------------------------------------///
    //enable GPIO peripheral that contains I2C 6
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    ///------------------------------------------------------------///

    ///------------------------------------------------------------///
    // Configure the pin muxing for I2C6 functions on port B6 and B7.
   // GPIOPinConfigure(GPIO_PB6_I2C6SCL);
   // GPIOPinConfigure(GPIO_PB7_I2C6SDA);
    ///------------------------------------------------------------///

    ///------------------------------------------------------------///
    // Select the I2C function for these pins.
    //Configuring SCL on PORT B6
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_6);
    //Configuring SDL on Port B7
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_7);
    ///------------------------------------------------------------///

    // Enable and initialize the I2C0 master module.  Use the system
    // clock for the I2C6 module.
    // The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and
    // if true the data rate will
    // be set to 400kbps.
    ///------------------------------------------------------------///

    //Setting clock for I2C here as 100kbps
    I2CMasterInitExpClk(I2C6_BASE, SYSTEM_CLOCK, false);
    ///------------------------------------------------------------///

    //clear I2C FIFOs
    HWREG(I2C6_BASE + I2C_O_FIFOCTL) = 80008000;
    ///------------------------------------------------------------///
    // I2C glitch filter that allows the application code to
    // robustly handle transient noises that may affect the master

    //Setting GLITCH_FILTER_8
    I2CMasterGlitchFilterConfigSet(I2C6_BASE, I2C_MASTER_GLITCH_FILTER_8);

}

uint16_t I2C_Receive(uint32_t I2CBase, uint32_t slave_addr, uint8_t Register)
{
    //specify that we are writing (a register address) to the
    //slave device
    uint32_t DataReceived = 0;

    I2CMasterSlaveAddrSet(I2CBase, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2CBase, Register);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2CBase, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2CBase));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2CBase, slave_addr, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2CBase, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2CBase));

    //return data pulled from the specified register
    DataReceived = I2CMasterDataGet(I2CBase);

    return DataReceived;
}

void I2C_Receive_Burst (uint32_t I2CBase, uint32_t slave_addr, uint8_t reg, uint16_t numElem, uint8_t *buff)
{
    uint32_t i;
    // / Set slave address and write mode
    I2CMasterSlaveAddrSet (I2CBase, slave_addr, false);
    // / specify the address to read
    I2CMasterDataPut(I2CBase, 12);
    // / send START (BIT1) and RUN (BIT0)
    // / from idle to transmit mode, page 1023 UG
    I2CMasterControl (I2CBase, I2C_MASTER_CMD_BURST_SEND_START);
    // wait for MCU to finish transaction
    while ( I2CMasterBusy (I2CBase));

    // specify that we are going to read from slave device
    I2CMasterSlaveAddrSet (I2CBase, slave_addr, true);

    // / Repeated START condition followed by RECEIVE (master remains in Master Receive state). page 1024
    // / repeated start

    I2CMasterControl (I2CBase, I2C_MASTER_CMD_SINGLE_RECEIVE);

    // wait for MCU to finish transaction
    while ( I2CMasterBusy (I2CBase));

    // / the first byte is ready
    buff[0] = I2CMasterDataGet (I2CBase);

    for (i = 1 ; i <numElem - 1 ; i ++) {
        // / ReceivE operation (master remains in Master Receive state).

        I2CMasterControl (I2CBase, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        // wait for MCU to finish transaction
        while ( I2CMasterBusy (I2CBase));
        // // read the data
        buff[i] = I2CMasterDataGet (I2CBase);
    }

    // / last element
    // / RECEIVE followed by STOP condition (master goes to Idle state).
    I2CMasterControl (I2CBase, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    // wait for MCU to finish transaction
    while ( I2CMasterBusy (I2CBase));
    buff[numElem - 1 ] = I2CMasterDataGet (I2CBase);


}

uint32_t I2C_Transmit_Burst(uint32_t I2CBase, const uint8_t SlaveAddr, uint8_t *DataPtr, const uint32_t NumBytes)
{
    uint32_t ErrState    = 0;
    uint32_t Bytes       = 0;
    uint32_t Wait        = 0;

    if (I2CBase != 0)
    {
        // Wait for I2C controller to be idle
        while ( I2CMasterBusy(I2CBase) );

        // Prepare write to slave address
        I2CMasterSlaveAddrSet(I2CBase, SlaveAddr, false);

        // Whait until I2C bus is not busy
        while ( I2CMasterBusBusy(I2CBase) );

        while ( (Bytes < NumBytes)&&(ErrState==0) )
        {
            // Transfer next byte to I2C hardware and wait for idle state
            I2CMasterDataPut(I2CBase, DataPtr[Bytes]);
            while ( I2CMasterBusy(I2CBase) );

            // Single byte send requested
            if (NumBytes == 1)
            {
                // Initiate single send
                I2CMasterControl(I2CBase, I2C_MASTER_CMD_SINGLE_SEND);
            }
            // First byte to transmit
            else if ( Bytes == 0 )
            {
                // Initiate start
                I2CMasterControl(I2CBase, I2C_MASTER_CMD_BURST_SEND_START);
            }
            // Last byte to transmit
            else if ( Bytes == NumBytes-1 )
            {
                // Initiate stop
                I2CMasterControl(I2CBase, I2C_MASTER_CMD_BURST_SEND_FINISH);
            }
            // All other bytes
            else
            {
                I2CMasterControl(I2CBase, I2C_MASTER_CMD_BURST_SEND_CONT);
            }

            // Wait a few cycles then wait for I2C controller to be idle
            for (Wait=15; Wait>0; Wait--);

            while ( I2CMasterBusy(I2CBase) );

            // Check for I2C Errors
            ErrState = I2CMasterErr(I2CBase);

            if (ErrState == 0)
            {
                Bytes++;
            }
        }
    }

    return Bytes;
}
