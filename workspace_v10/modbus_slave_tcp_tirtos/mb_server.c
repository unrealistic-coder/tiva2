#include "mb_server.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*  Function Code Subroutines
    FC01(R) | FC05(W) - Coils(R/W) ,             FC02(R) - Discrete Inputs (Read-only)
    FC03(R) | FC06(W) - Holding Registers(R/W) , FC04(R) - Input Registers (Read-only) 
*/
static uint8_t* Get_Data8_Table(uint8_t func_code) {
    if (func_code == 0x01 || func_code == 0x05)
        return outputCoils;
    else if (func_code == 0x02)
        return inputContacts;
    else
        return NULL; // Handle invalid function codes
}

static uint16_t* Get_Data16_Table(uint8_t func_code) {
    if (func_code == 0x03 || func_code == 0x06)
        return holdingRegisters;
    else if (func_code == 0x04)
        return inputRegisters;
    else
        return NULL; // Handle invalid function codes
}
/*
        |----------------------|----------------|------------|----------------------------------|
        | Coil/Register Numbers| Data Addresses | Type       | Table Name                       |
        |----------------------|----------------|------------|----------------------------------|
        | 1-9999               | 0000 to 270E   | Read-Write | Discrete Output Coils            |
        | 10001-19999          | 0000 to 270E   | Read-Only  | Discrete Input Contacts          |
        | 30001-39999          | 0000 to 270E   | Read-Only  | Analog Input Registers           |
        | 40001-49999          | 0000 to 270E   | Read-Write | Analog Output Holding Registers  |
        |----------------------|----------------|------------|----------------------------------|
*/

uint8_t Read_Coil(uint8_t func_code, uint16_t start_coil_addr, uint16_t num_of_coils, uint8_t *coil_data) {

    // uint8_t *point_to_data_table;
    // if(func_code == 0x01)
    //     point_to_data_table = outputCoils;
    // else if(func_code == 0x02)
    //     point_to_data_table = inputContacts;

    // Get the appropriate data table
    volatile uint8_t *point_to_data_table = Get_Data8_Table(func_code);
    if (point_to_data_table == NULL) {
        // Handle error: invalid function code
        return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }

    if (num_of_coils <= 0 || num_of_coils >= NUM_OUTPUT_COILS) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }

    uint16_t end_coil_addr = start_coil_addr + (num_of_coils - 1);

    // Check data address validity (start and end addresses)
    if (start_coil_addr < NUM_OUTPUT_COILS * 8 && end_coil_addr < NUM_OUTPUT_COILS * 8 && start_coil_addr <= end_coil_addr) {
        // Valid addresses, proceed with further processing
    } else {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;  // Invalid address
    }

    
    // Clear the destination buffer
    memset(coil_data, 0, size_of_data(num_of_coils));
    // printf("%20s %20s %20s %20s %20s %20s %20s %10s\n", "coil_index", "current_coil_addr","source_byte_index","source_bit_index","dest_byte_index","dest_bit_index", "coil_value","coil_data");
    uint8_t coil_index = 0;
    // Loop through each coil in the specified range
    for (coil_index = 0; coil_index < num_of_coils; coil_index++) {
        uint16_t current_coil_addr = start_coil_addr + coil_index;

        // Calculate the byte and bit position in the source `coils` array
        uint8_t source_byte_index = current_coil_addr / 8;
        uint8_t source_bit_index = current_coil_addr % 8;

        // Get the coil value (0 or 1) from the source array
        uint8_t coil_value = (point_to_data_table[source_byte_index] >> (7 - source_bit_index)) & 0x01;

        // Calculate the byte and bit position in the destination `coil_data` buffer
        uint8_t dest_byte_index = coil_index / 8;
        uint8_t dest_bit_index = coil_index % 8;

        // Set the bit in the destination buffer
        coil_data[dest_byte_index] |= (coil_value << dest_bit_index);
        // printf("%20d %20d %20d %20d %20d %20d %20d %10x\n", coil_index, current_coil_addr,source_byte_index,source_bit_index,dest_byte_index,dest_bit_index, coil_value, coil_data[dest_byte_index]);
    
    }
    return 0;

}

uint8_t Write_Coil(uint8_t func_code, uint16_t start_coil_addr, uint16_t write_data){

    volatile uint8_t *point_to_data_table = Get_Data8_Table(func_code);
    if (point_to_data_table == NULL) {
        // Handle error: invalid function code
        return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }

    if (start_coil_addr >= NUM_OUTPUT_COILS )
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    uint8_t source_byte_index = start_coil_addr / 8;
    uint8_t source_bit_index = start_coil_addr % 8;
    if (write_data == 0xFF00)
        outputCoils[source_byte_index] |= (1 << (7 - source_bit_index));
    else if ( write_data == 0x0000 )
        outputCoils[source_byte_index] &= ~(1 << (7 - source_bit_index));
    else
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    return 0;

}

uint8_t Read_Register(uint8_t func_code, uint16_t start_reg_addr, uint16_t num_of_reg, uint16_t *reg_data){
    
    uint16_t *point_to_data_table = Get_Data16_Table(func_code);
    if (point_to_data_table == NULL) {
        // Handle error: invalid function code
        return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }

    if (num_of_reg <= 0 || num_of_reg >= NUM_HOLDING_REGISTERS) {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
    }
    uint16_t end_reg_addr = start_reg_addr + num_of_reg - 1;
    // Check data address validity (start and end addresses)
    if (start_reg_addr < NUM_HOLDING_REGISTERS &&  end_reg_addr < NUM_HOLDING_REGISTERS && start_reg_addr <= end_reg_addr) {
        // Valid addresses, proceed with further processing
    } else {
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;  // Invalid address
    }

    uint8_t data_index = 0;
    while(data_index < num_of_reg)
        reg_data[data_index++] = point_to_data_table[start_reg_addr++];
    return 0;

}

uint8_t Write_Register(uint8_t func_code, uint16_t start_reg_addr, uint16_t write_data){

    uint16_t *point_to_data_table = Get_Data16_Table(func_code);
    if (point_to_data_table == NULL) {
        // Handle error: invalid function code
        return MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
    }

    if (start_reg_addr >= NUM_HOLDING_REGISTERS )
        return MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

    point_to_data_table[start_reg_addr] = write_data;
    return 0;
}

// parse the request , do the opertaion of func code and frame the response and give to TCP management layer
uint16_t build_modbus_response(uint8_t* request, mb_res_pdu* frame) {
    uint16_t response_length = 0;

    frame->mbap.t_id = (request[0] << 8) | request[1]; // 2 byte  
    frame->mbap.p_id = (request[2] << 8) | request[3]; // 2 byte
    // frame->mbap.length = (request[4] << 8) | request[5]; //2 byte
    //length = unit_id + func_code + byte_count (read)
    //length = (request[4] << 8) | request[5]; (write)
    frame->mbap.length = 
    frame->mbap.u_id = request[6]; // 1 byte
    frame->func_code = request[7]; // 1 byte

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //debugging
    // printf("MBAP inside function\n");
    // printf("0x%04X 0x%04X 0x%04X 0x%02X 0x%02X\n", frame->mbap.t_id, frame->mbap.p_id, frame->mbap.length, frame->mbap.u_id, frame->func_code);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////

    uint16_t start_address = (request[8] << 8) | request[9];
    uint16_t quantity = (request[10] << 8) | request[11];
    // exception data value check - hex 3
    // ...

    modbus_exception_code_t exception = (modbus_exception_code_t)0;

    uint8_t coil_data[size_of_data(quantity)];  // Coils buffer, dynamic size
    uint16_t reg_data[quantity];            // Registers buffer, dynamic size


    // Handle the function code
    switch (frame->func_code) {
        case 0x01: // Read Coils
        case 0x02: // Read Discrete Inputs
            exception = (modbus_exception_code_t)Read_Coil(frame->func_code, start_address, quantity, coil_data);
            if (exception == 0) {
                // Successful read, fill the response
                frame->body.byte_count = size_of_data(quantity);
                memcpy(frame->data, coil_data, frame->body.byte_count);
            }
            frame->mbap.length = 1 + 1 + 1 + frame->body.byte_count; // uid+fc+byte_count + data=bytecount
            response_length = 7 + 1 + 1 + frame->body.byte_count;
            break;

        case 0x03: // Read Holding Registers
        case 0x04: // Read Input Registers
            exception = (modbus_exception_code_t)Read_Register(frame->func_code, start_address, quantity, reg_data);
            if (exception == 0) {
                // Successful read, fill the response
                frame->body.byte_count = quantity * 2;
                uint8_t i = 0;
                for (i = 0; i < quantity; i++) {
                    frame->data[i * 2] = (reg_data[i] >> 8) & 0xFF; // High byte first
                    frame->data[i * 2 + 1] = reg_data[i] & 0xFF;    // Low byte second
                }
            }
            frame->mbap.length = 1 + 1 + 1 + frame->body.byte_count; // uid+fc+byte_count + data=bytecount
            response_length = 7 + 1 + 1 + frame->body.byte_count;
            break;

        case 0x05: // Write Single Coil
            exception = (modbus_exception_code_t)Write_Coil(frame->func_code, start_address, (request[10] << 8) | request[11]);
            frame->mbap.length = (request[4] << 8) | request[5];
            //otherwise response == request
            if (exception == 0) {
                // no byte count
                // Echo back the request with the same function code
                frame->data[0] = request[8]; // Start address (High byte)
                frame->data[1] = request[9]; // Start address (Low byte)
                frame->data[2] = request[10]; // Write data (High byte)
                frame->data[3] = request[11]; // Write data (Low byte)
                frame->body.byte_count = 4; // Fixed byte count for 4 bytes in Write Coil response
            }
            response_length = 7 + frame->mbap.length - 1;
            break;

        case 0x06: // Write Single Register
            exception = (modbus_exception_code_t)Write_Register(frame->func_code, start_address, (request[10] << 8) | request[11]);
            frame->mbap.length = (request[4] << 8) | request[5];
            //otherwise response == request
            // no byte count
            if (exception == 0) {
                // Echo back the request with the same function code
                frame->data[0] = request[8]; // Start address (High byte)
                frame->data[1] = request[9]; // Start address (Low byte)
                frame->data[2] = request[10]; // Write data (High byte)
                frame->data[3] = request[11]; // Write data (Low byte)
                frame->body.byte_count = 4; // Fixed byte count for 4 bytes in Write Register response
            }
            response_length = 7 + frame->mbap.length - 1;
            break;

        default:
            // Handle invalid function codes
            exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
            break;
    }
    // If exception occurred, modify the function code to exception code
    if (exception != 0) {
        frame->func_code |= 0x80; // Set exception flag
        frame->body.exc_code = exception; // Fill in exception code
        response_length = 7 + 2;
    } 
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //debug
    #if DEBUG
    printf("Inside build\n");
    printf("response_length :%d\n", response_length);
    printf("byte count: 0x%02X\n", frame->body.byte_count);
    for (int i = 0; i < frame->body.byte_count; i++) {
        printf("0x%02X ", frame->data[i]);
    }
    printf("\n");
    #endif
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    return response_length;
}

uint8_t serialize_response(const mb_res_pdu *response, unsigned char *buffer) {
    uint8_t index = 0;

    // MBAP Header
    buffer[index++] = response->mbap.t_id >> 8;
    buffer[index++] = response->mbap.t_id & 0xFF;
    buffer[index++] = response->mbap.p_id >> 8;
    buffer[index++] = response->mbap.p_id & 0xFF;
    buffer[index++] = response->mbap.length >> 8;
    buffer[index++] = response->mbap.length & 0xFF;
    buffer[index++] = response->mbap.u_id;

    // Function Code
    buffer[index++] = response->func_code;

    // Body
    if ((response->func_code & 0x80) != 0) { // Exception response
        buffer[index++] = response->body.exc_code;
    } else { // Normal Read response
        uint8_t i = 0;
        if (response->func_code == 5 || response->func_code == 6) {
            // Skip adding byte count for function codes 0, 1, 2, and 3
            for (i = 0; i < response->body.byte_count; i++) {
                buffer[index++] = response->data[i]; // Data
            }
        } else { // write response
            // Add byte count and data for other function codes
            buffer[index++] = response->body.byte_count; // Byte count
            for (i = 0; i < response->body.byte_count; i++) {
                buffer[index++] = response->data[i]; // Data
            }
        }
    }

    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //debug
    #if DEBUG
    printf("Inside serial\n");
    for(int i = 0; i < index; i++) {
        printf("0x%02X ",buffer[i]);
    }
    printf("\n");
    #endif
    // //////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return index;  // Returns the total length of serialized data
}

#if TEST

void test_read_coil(){
    printf("Testing Read coil function\n");
    // Example: Set some coils in the `coils` array
    outputCoils[0] = 0b10101010; // Coils 0-7
    outputCoils[1] = 0b11001100; // Coils 8-15

    uint8_t coil_data[16]; // Buffer for the response (size based on the number of coils)

    // Read 10 coils starting from coil address 2
    Read_Coil(0x01, 2, 10, coil_data);

    // Print the result
    printf("Coil Data: ");
    for (int i = 0; i < ((10 + 7) / 8); i++) {
        printf("0x%02X ", coil_data[i]);
    }
    printf("\n");

    #if SHOW_BITS
    // Example: Print each coil status individually
    for (int i = 0; i < 10; i++) {
        uint8_t byte_index = i / 8;
        uint8_t bit_index = i % 8;
        uint8_t coil_status = (coil_data[byte_index] >> bit_index) & 0x01;
        printf("%d", coil_status);
    }
    #endif
}


void test_write_coil(){
    printf("Testing Write coil function\n");

    // Example: Set some coils in the `coils` array
    outputCoils[0] = 0b00000001; // Coils 0-7
    outputCoils[1] = 0b00000010; // Coils 8-15

    uint8_t coil_data[16]; // Buffer for the response (size based on the number of coils)

    Write_Coil(0x05, 8, 0xFF00);

    // Print the result
    printf("Coil Data: ");
    for (int i = 0; i < 2; i++) {
        printf("0x%02X ", outputCoils[i]);
    }
    printf("\n");

}

void test_read_reg(){
    printf("Testing Read register\n");

    // Directly initialize holding registers with example values
    holdingRegisters[0] = 0x1234;
    holdingRegisters[1] = 0x5678;
    holdingRegisters[2] = 0x9ABC;
    holdingRegisters[3] = 0xDEF0;
    holdingRegisters[4] = 0x1111;
    holdingRegisters[5] = 0x2222;
    holdingRegisters[6] = 0x3333;
    holdingRegisters[7] = 0x4444;
    holdingRegisters[8] = 0x5555;
    holdingRegisters[9] = 0x6666;

    uint16_t reg_data[2]; // Buffer for 5 registers

    // Read 5 registers starting from address 2
    Read_Register(0x03, 0, 2, reg_data);

    // Print the result in hexadecimal
    printf("Read Register Data (in Hex):\n");
    for (int i = 0; i < 2; i++) {
        printf("0x%04X ",reg_data[i]);
    }
    printf("\n");

}

void test_write_reg(){
    printf("Testing Write register\n");
    // Directly initialize holding registers with example values
    holdingRegisters[0] = 0x1234;
    holdingRegisters[1] = 0x5678;
    holdingRegisters[2] = 0x9ABC;
    holdingRegisters[3] = 0xDEF0;
    holdingRegisters[4] = 0x1111;
    holdingRegisters[5] = 0x2222;
    holdingRegisters[6] = 0x3333;
    holdingRegisters[7] = 0x4444;
    holdingRegisters[8] = 0x5555;
    holdingRegisters[9] = 0x6666;

    // Read 5 registers starting from address 2
    Write_Register(0x06, 2, 0x3333);

    // Print the result in hexadecimal
    printf("Read Register Data (in Hex):\n");
    for (int i = 0; i < 10; i++) {
        printf("0x%04X ",holdingRegisters[i]);
    }
    printf("\n");

}

void test_build_modbus_response() {

    printf("testing build response function\n");
    // Example Modbus request: 
    // Transaction ID = 0x0001, Protocol ID = 0x0000, Length = 0x0006, Unit ID = 0x01, Function Code = 0x03 (Read Holding Registers)
    // Starting Address = 0x0000, Quantity = 0x0002 (Reading two holding registers)
    uint8_t request[] = { 
        0x00, 0x01,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length (Excluding the header)
        0x01,         // Unit ID (Slave Address)
        0x03,         // Function Code (Read Holding Registers)
        0x00, 0x00,   // Starting Address (0)
        0x00, 0x02    // Quantity (2 registers)
    };
    mb_res_pdu response_frame;
    uint16_t response_length = build_modbus_response(request , &response_frame);

    printf("Response Length: %d\n", response_length);
}

void test_serialise_buffer() {
    printf("testing serialise buffer\n");
    mb_res_pdu response = {
    .mbap = { .t_id = 1, .p_id = 0, .length = 6, .u_id = 1 },
    .func_code = 0x83,
    .body = { .byte_count = 4 },
    .data = { 0x12, 0x34, 0x56, 0x78 }
    };
    unsigned char buffer[260];
    int length = serialize_response(&response, buffer);
    
    // Print serialized response
    for (int i = 0; i < length; i++) {
        printf("0x%02X ", buffer[i]);
    }

}

void test_modbus_request_response() {
    printf("Testing complete Modbus request-response cycle\n");
    // Create a sample Modbus request
    // Function Code: 0x03 (Read Holding Registers)
    // Transaction ID: 0x1234, Protocol ID: 0x0000, Unit ID: 0x01
    // Start Address: 0x0002, Quantity: 0x0002 (read 2 registers)
    uint8_t readCoilsRequest[] = {
        0x00, 0x01,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x01,        // Function Code (Read Coils)
        0x00, 0x00,  // Start Address
        0x00, 0x0A   // Quantity (10 Coils)
    };

    uint8_t readDiscreteInputsRequest[] = {
        0x00, 0x02,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x02,        // Function Code (Read Discrete Inputs)
        0x00, 0x10,  // Start Address
        0x00, 0x0A   // Quantity (10 Inputs)
    };

    uint8_t readHoldingRegistersRequest[] = {
        0x00, 0x03,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x03,        // Function Code (Read Holding Registers)
        0x00, 0x20,  // Start Address
        0x00, 0x02   // Quantity (2 Registers)
    };

    uint8_t readInputRegistersRequest[] = {
        0x00, 0x04,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x04,        // Function Code (Read Input Registers)
        0x00, 0x30,  // Start Address
        0x00, 0x02   // Quantity (2 Registers)
    };

    uint8_t writeSingleCoilRequest[] = {
        0x00, 0x05,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x05,        // Function Code (Write Single Coil)
        0x00, 0x00,  // Coil Address
        0xFF, 0x00   // Coil Value (ON)
    };

    uint8_t writeSingleRegisterRequest[] = {
        0x00, 0x06,  // Transaction ID
        0x00, 0x00,  // Protocol ID
        0x00, 0x06,  // Length
        0x01,        // Unit ID
        0x06,        // Function Code (Write Single Register)
        0x00, 0x01,  // Register Address
        0x12, 0x34   // Register Value (0x1234)
    };


    // Buffer for the response
    mb_res_pdu response_frame = {0};
    unsigned char serialized_response[260]; // Maximum response size
    // uint8_t *request = writeSingleCoilRequest;
    // Build the response based on the request
    uint16_t response_length = build_modbus_response(writeSingleCoilRequest, &response_frame);

    // Serialize the response into a byte buffer
    int serialized_length = serialize_response(&response_frame, serialized_response);

    // Print request details
    printf("\nRequest Details:\n");
    for (int i = 0; i < sizeof(writeSingleCoilRequest); i++) {
        printf("0x%02X ", writeSingleCoilRequest[i]);
    }
    printf("\n");

    // Print response details
    printf("\nResponse Details:\n");
    for (int i = 0; i < serialized_length; i++) {
        printf("0x%02X ", serialized_response[i]);
    }
    printf("\n");

    // Validate response
    printf("\nValidation:\n");
    printf("Transaction ID: 0x%04X\n", response_frame.mbap.t_id);
    printf("Protocol ID: 0x%04X\n", response_frame.mbap.p_id);
    printf("Unit ID: 0x%02X\n", response_frame.mbap.u_id);
    printf("Function Code: 0x%02X\n", response_frame.func_code);
    printf("Byte Count: 0x%02X\n", response_frame.body.byte_count);
    printf("Data: ");
    for (int i = 0; i < response_frame.body.byte_count; i++) {
        printf("0x%02X ", response_frame.data[i]);
    }
    printf("\n");

    // Check if serialized length matches the response length
    printf("\nSerialized length: %d, Response length: %d\n",
           serialized_length, response_length);
}

#endif
// // TODO: handle the other coils and reg by func code
// // handle write multiple
// // struct to frame response
// // handle exception
// // sende the response back
// // check the output by prining coild table and register table





