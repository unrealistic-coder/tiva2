#ifndef MB_SERVER_H
#define MB_SERVER_H

#include <stdint.h>

#define size_of_data(x) (int)((x + 7)/8) //return ceil(x) index
#define SHOW_BITS 0
#define DEBUG 0
#define TEST 0

// Maximum size of the tables (adjust as needed)
#define NUM_HOLDING_REGISTERS 100
#define NUM_INPUT_REGISTERS   100
#define NUM_OUTPUT_COILS      128 // addresses_of_coils = (no_of_bytes * 8) - 1
#define NUM_INPUT_CONTACTS    128
#define MAX_RESPONSE_DATA     252 //TCP MODBUS ADU = 253 bytes + MBAP (7 bytes) = 260 bytes. 251 excludes func_code and bye count

// typedef unsigned char

typedef struct {
    unsigned short t_id;                // transaction ID ,request number
    unsigned short p_id;                // protocol ID always 0 for modbus
    unsigned short length;              // no. of bytes follow, changes from request length
    unsigned char u_id;                 // unit id or slave id 
} mbap_header;

typedef struct {
    mbap_header mbap;
    unsigned char func_code;
    union {
        unsigned char byte_count;
        unsigned char exc_code;
    } body;
    unsigned char data[MAX_RESPONSE_DATA];
} mb_res_pdu;


// error codes
typedef enum {
    MODBUS_EXCEPTION_ILLEGAL_FUNCTION = 0x01,        // 01 hex: Illegal Function
    MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS = 0x02,    // 02 hex: Illegal Data Address
    MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE = 0x03,      // 03 hex: Illegal Data Value
    MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE = 0x04,    // 04 hex: Slave Device Failure
    MODBUS_EXCEPTION_ACKNOWLEDGE = 0x05,             // 05 hex: Acknowledge
    MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY = 0x06,       // 06 hex: Slave Device Busy
    MODBUS_EXCEPTION_NEGATIVE_ACKNOWLEDGE = 0x07,    // 07 hex: Negative Acknowledge
    MODBUS_EXCEPTION_MEMORY_PARITY_ERROR = 0x08,     // 08 hex: Memory Parity Error
    MODBUS_EXCEPTION_GATEWAY_PATH_UNAVAILABLE = 0x0A,// 0A hex: Gateway Path Unavailable
    MODBUS_EXCEPTION_GATEWAY_TARGET_FAILED = 0x0B    // 0B hex: Gateway Target Device Failed to Respond
} modbus_exception_code_t;

// Modbus tables
uint16_t holdingRegisters[NUM_HOLDING_REGISTERS];                // Holding registers (R/W)
uint16_t inputRegisters[NUM_INPUT_REGISTERS];                    // Input registers (Read-only)
uint8_t outputCoils[size_of_data(NUM_OUTPUT_COILS)];             // Coils (R/W)
uint8_t inputContacts[size_of_data(NUM_INPUT_CONTACTS)];         // Discrete inputs (Read-only)

// TODO:
// declare the arrays, extern volatile uint8_t array[]; in main.c

#if TEST
// UNIT Test functions
void test_read_coil();
void test_write_coil();
void test_read_reg();
void test_write_reg();
void test_build_modbus_response();
void test_serialise_buffer();

#endif

// modbus functions
uint8_t Read_Coil(uint8_t func_code, uint16_t start_coil_addr, uint16_t num_of_coils, uint8_t *coil_data);
uint8_t Write_Coil(uint8_t func_code, uint16_t start_coil_addr, uint16_t write_data);
uint8_t Read_Register(uint8_t func_code, uint16_t start_reg_addr, uint16_t num_of_reg, uint16_t *reg_data);
uint8_t Write_Register(uint8_t func_code, uint16_t start_reg_addr, uint16_t write_data);
uint16_t build_modbus_response(uint8_t* request, mb_res_pdu* frame);
uint8_t serialize_response(const mb_res_pdu *response, unsigned char *buffer);

#endif


