//****************************************************************************
//*
//*
//* File: COMModbusServer.h
//*
//* Synopsis: Modbus Server Definition
//*
//*  
//*
//****************************************************************************

#ifndef COMMODBUSTCPSERVER_H
#define COMMODBUSTCPSERVER_H

//============================================================================
//                              > INCLUDE FILES <
//----------------------------------------------------------------------------
#include <Common.h>
#include "COMTimer.h"
#include "UPSInterface.h"


#define SET_YEAR_INDEX     1
#define SET_MONTH_INDEX     2
#define SET_DATE_INDEX     3
#define SET_HOUR_INDEX    4
#define SET_MINUTE_INDEX   5
#define SET_SECONDS_INDEX  6
//----------------------------------------------------------------------------

//============================================================================
//                              > MACROS <
//----------------------------------------------------------------------------

#if (_MBTCP_DEBUG == 1)
#define MBTCP_printf(...)     UARTprintf(__VA_ARGS__)
#else
#define MBTCP_printf(...)     {};
#endif

#define MAX_MODBUS_BUFFER_SIZE  (256)
#define MODBUS_NO_OF_GROUPS     (21)//(20)

#define SUCCESS                 (0)
#define FAIL                    (1)



#define MBTCP_MAX_CONNECTION  (3)
#define MBTCP_DEFAULT_PORT  (502) /* TCP listening port. */
#define MBTCP_BUF_SIZE     ( 256 + 7 ) /* Must hold a complete Modbus TCP frame. */
#define MBTCP_CONNECTION_TIMEOUT (3000) // in millisecond --> 1000* 10 tick (10ms tick) = 10 sec
#define MBTCP_MAX_TIMEOUT    (60000) // in millisecond --> 6000* 10 tick (10ms tick) = 1 min


#define MBTCP_TID          (0)
#define MBTCP_PID          (2)
#define MBTCP_LEN          (4)
#define MBTCP_UID          (6)
#define MBTCP_FUNC         (7)

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define   MODBUSTCP_WARNING_ITEM_GROUPINDX             (0)
#define   MODBUSTCP_CAPSET_CAPLIST_GROUPINDX           (1)
#define   MODBUSTCP_CTRLITEM_GROUPINDX                 (2)
#define   MODBUSTCP_RESCTRL_GROUPINDX                  (3)
#define   MODBUSTCP_SETTPARM1_GROUPINDX                (4)
#define   MODBUSTCP_SETTPARM2_GROUPINDX                (5)
#define   MODBUSTCP_UPSWORK_STAT1_GROUPINDX            (6)
#define   MODBUSTCP_UPSWORK_STAT2_GROUPINDX            (7)
#define   MODBUSTCP_UPSBATTINFO_GROUPINDX              (8)
#define   MODBUSTCP_UPSWORK_MODE_GROUPINDX             (9)
#define   MODBUSTCP_UPSFAULT_INFO_GROUPINDX            (10)
#define   MODBUSTCP_OUTPUTSOCK1_GROUPINDX              (11)
#define   MODBUSTCP_OUTPUTSOCK2_GROUPINDX              (12)
#define   MODBUSTCP_LOSSPNTDATA_GROUPINDX              (13)
#define   MODBUSTCP_SETTPARM_SUCC_FAIL_GROUPINDX       (14)
#define   MODBUSTCP_REMOTESHUTDN_GROUPINDX             (15)
#define   MODBUSTCP_REMOTESHUTDN_TEST_GROUPINDX        (16)
#define   MODBUSTCP_CPUINFO_GROUPINDX                  (17)
#define   MODBUSTCP_UPSMODEL_GROUPINDX                 (18)
#define   MODBUSTCP_UPSRATINGINFO_GROUPINDX            (19)
#define   MODBUSTCP_DATETIMESET_GROUPINDX              (20)

#define  START_ADDRESS_OF_WARNING_ITEM     (0)
#define  END_ADDRESS_OF_WARNING_ITEM       (3)
#define  MAX_LENGHT_OF_WARNING_ITEM        (4) // 4 + 0=4


#define  SECOND_ADDRESS_OF_WARNGITEM         (START_ADDRESS_OF_WARNING_ITEM + 1) // (0+1)
#define  THIRD_ADDRESS_OF_WARNGITEM          (START_ADDRESS_OF_WARNING_ITEM + 2) // (0+2)
#define  FOURTH_ADDRESS_OF_WARNGITEM         (START_ADDRESS_OF_WARNING_ITEM + 3) // (0+3)


#define  START_ADDRESS_OF_CAPSET_CAPLIST   (14)   //CAPSET =Capability setting
#define  END_ADDRESS_OF_CAPSET_CAPLIST     (17)
#define  MAX_LENGHT_OF_CAPSET_CAPLIST      (4)  // 4 + 4 = 8


#define  SECOND_ADDRESS_OF_CAPSET_CAPLIST     (START_ADDRESS_OF_CAPSET_CAPLIST + 1) //(14 + 1)
#define  THIRD_ADDRESS_OF_CAPSET_CAPLIST      (START_ADDRESS_OF_CAPSET_CAPLIST + 2) // (14 + 2)
#define  FOURTH_ADDRESS_OF_CAPSET_CAPLIST     (START_ADDRESS_OF_CAPSET_CAPLIST + 3) // (14 + 3)


#define  START_ADDRESS_OF_CTRLITEM         (26)  //control item
#define  END_ADDRESS_OF_CTRLITEM           (26) // < 27
#define  MAX_LENGHT_OF_CTRLITEM            (1)    // 8 + 1 = 9


#define  START_ADDRESS_OF_RESCTRL          (37)   // result of control
#define  END_ADDRESS_OF_RESCTRL            (37)
#define  MAX_LENGHT_OF_RESCTRL             (1)   // 9 + 1 = 10


#define  START_ADDRESS_OF_SETTPARM1         (48)   // setting parameter of default value
#define  END_ADDRESS_OF_SETTPARM1           (48)
#define  MAX_LENGHT_OF_SETTPARM1             (1)  // 10 + 1 =11

#define  START_ADDRESS_OF_SETTPARM2         (59)  // setting parameter of default value
#define  END_ADDRESS_OF_SETTPARM2           (59)
#define  MAX_LENGHT_OF_SETTPARM2             (1)   // 11 + 1 = 12


#define  START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT      (170)
#define  END_ADDRESS_OF_UPSWORK_STAT1_UPS_STATUS      (182)
#define  MAX_LENGHT_OF_UPSWORK_STAT1                   (13)     // 12 + 13 =25


#define  ADDRESS_OF_INPUT_FREQUENCY             (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 1)// (170 + 1)
#define  ADDRESS_OF_OUTPUT_VOLTAGE              (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 2)// (170 + 2)
#define  ADDRESS_OF_OUTPUT_FREQUENCY            (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 3)// (170 + 3)
#define  ADDRESS_OF_OUTPUT_CURRENT              (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 4)// (170 + 4)
#define  ADDRESS_OF_OUTPUT_LOAD_PERCENT         (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 5)// (170 + 5)
#define  ADDRESS_OF_P_BUS_VOLTAGE               (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 6)// (170 + 6)
#define  ADDRESS_OF_N_BUS_VOLTAGE               (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 7)// (170 + 7)
#define  ADDRESS_OF_P_BATT_VOLTAGE              (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 8)// (170 + 8)
#define  ADDRESS_OF_N_BATT_VOLTAGE              (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 9)// (170 + 9)
#define  ADDRESS_OF_MAX_TEMP_DETECT_POINTERS    (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 10)// (170 + 10)
#define  ADDRESS_OF_UPS_STATUS0                  (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 11)// (170 + 11)
#define  ADDRESS_OF_UPS_STATUS1                  (START_ADDRESS_OF_UPSWORK_STAT1_INP_VOLT + 12)// (170 + 12)

#define  START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS     (198)
#define  END_ADDRESS_OF_UPSWORK_STAT2_INP_T_VOLT            (203)
#define  MAX_LENGHT_OF_UPSWORK_STAT2                         (6)  // 25 + 6 = 31


#define  ADDRESS_OF_LINE_VOLT_OF_RT         (START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS + 1)// (198 + 1)
#define  ADDRESS_OF_LINE_VOLT_OF_ST         (START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS + 2)// (198 + 2)
#define  ADDRESS_OF_INP_R_VOLT              (START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS + 3)// (198 + 3)
#define  ADDRESS_OF_INP_S_VOLT              (START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS + 4)// (198 + 4)
#define  ADDRESS_OF_INP_T_VOLT              (START_ADDRESS_OF_UPSWORK_STAT2_LINE_VOLT_OF_RS + 5)// (198 + 5)


#define START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT           (188)
#define END_ADDRESS_OF_UPSBATTINFO_BATT_REMN_TIME        (192)
#define MAX_LENGHT_OF_UPSBATTINFO                         (5) //  31 + 5 = 36


#define  ADDRESS_OF_BATT_PIECE_NUM         (START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT  + 1)// (188 + 1)
#define  ADDRESS_OF_BATT_GROUP_NUM         (START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT  + 2)// (188 + 2)
#define  ADDRESS_OF_BATT_CAPACITY          (START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT  + 3)// (188 + 3)
#define  ADDRESS_OF_BATT_REMN_TIME         (START_ADDRESS_OF_UPSBATTINFO_BATT_VOLT  + 4)// (188 + 4)


#define START_ADDRESS_OF_UPSWORK_MODE     (208)
#define END_ADDRESS_OF_UPSWORK_MODE       (208)
#define MAX_LENGHT_OF_UPSWORK_MODE         (1)  //36 + 1 = 37


#define START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE                   (675)
#define END_ADDRESS_OF_UPSFAULT_INFO_UPS_RUN_STAT_BEFORE_FAULT      (686)
#define MAX_LENGHT_OF_UPSFAULT_INFO                                  (12)   // 37 + 12 = 49


#define   ADDRESS_OF_BATT_VOLT_BEFORE_FAULT             (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 1) //(675 + 1)
#define   ADDRESS_OF_INP_FRQNCY_BEFORE_FAULT            (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 2) //(675 + 2)
#define   ADDRESS_OF_INP_VOLT_BEFORE_FAULT              (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 3) //(675 + 3)
#define   ADDRESS_OF_INVTR_OUTP_FRQNCY_BEFORE_FAULT     (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 4) //(675 + 4)
#define   ADDRESS_OF_INVTR_OUTP_VOLT_BEFORE_FAULT       (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 5) //(675 + 5)
#define   ADDRESS_OF_N_BUS_VOLT_BEFORE_FAULT            (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 6) //(675 + 6)
#define   ADDRESS_OF_P_BUS_VOLT_BEFORE_FAULT            (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 7) //(675 + 7)
#define   ADDRESS_OF_OUTP_LOAD_BEFORE_FAULT             (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 8) //(675 + 8)
#define   ADDRESS_OF_OUTP_CURRENT_BEFORE_FAULT          (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 9) //(675 + 9)
#define   ADDRESS_OF_TEMP_BEFORE_FAULT                  (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 10) //(675 + 10)
#define   ADDRESS_OF_UPS_RUN_STAT_BEFORE_FAULT          (START_ADDRESS_OF_UPSFAULT_INFO_FAULT_TYPE  + 11) //(675 + 11)


#define START_ADDRESS_OF_OUTPUTSOCK1       (838)
#define END_ADDRESS_OF_OUTPUTSOCK          (838)
#define MAX_LENGHT_OF_OUTPUTSOCK            (1)   // 49 + 1 = 50


#define START_ADDRESS_OF_OUTPUTSOCK2          (907)
#define END_ADDRESS_OF_OUTPUT_SOCK            (907)
#define MAX_LENGHT_OF_OUTPUT_SOCK              (1)   // 50 + 1 = 51


#define START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP    (842)
#define END_ADDRESS_OF_LOSSPNTDATA_BYPS_VOLT_LLP               (849)
#define MAX_LENGHT_OF_LOSSPNTDATA                               (8)   // 51 + 8 = 59


#define  ADDRESS_OF_HIGH_EFF_MODE_VOLT_LLP     (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 1)// (842 + 1)
#define  THIRD_ADDRESS_OF_LOSSPNTDATA          (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 2)// (842 + 2)
#define  FOURTH_ADDRESS_OF_LOSSPNTDATA         (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 3)// (842 + 3)
#define  ADDRESS_OF_BYPS_FREQ_HLP              (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 4)// (842 + 4)
#define  ADDRESS_OF_BYPS_FREQ_LLP              (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 5)// (842 + 5)
#define  ADDRESS_OF_BYPS_VOLT_HLP              (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 6)// (842 + 6)
#define  ADDRESS_OF_BYPS_VOLT_LLP              (START_ADDRESS_OF_LOSSPNTDATA_HIGH_EFF_MODE_VOLT_HLP  + 7)// (842 + 7)


#define START_ADDRESS_OF_SETTPARM_SUCC_FAIL      (901)
#define END_ADDRESS_OF_SETTPARM_SUCC_FAIL        (901)
#define MAX_LENGHT_OF_SETTPARM_SUCC_FAIL          (1)  // 59 + 1 = 60


#define START_ADDRESS_OF_REMOTESHUTDN               (939)
#define END_ADDRESS_OF_REMOTESHUTDN_RESTORE         (943)
#define MAX_LENGHT_OF_REMOTESHUTDN                   (5)  // 60 + 5 = 65


#define  ADDRESS_OF_TEST_FOR_SPECIF_TIME      (START_ADDRESS_OF_REMOTESHUTDN   + 1)// (939 + 1)
#define  ADDRESS_OF_SHUTDN_RESTORE_N          (START_ADDRESS_OF_REMOTESHUTDN   + 2)// (939 + 2)
#define  ADDRESS_OF_SHUTDN_RESTORE_S          (START_ADDRESS_OF_REMOTESHUTDN   + 3)// (939 + 3)
#define  ADDRESS_OF_SHUTDN_RESTORE_R          (START_ADDRESS_OF_REMOTESHUTDN   + 4)// (939 + 4)


#define START_ADDRESS_OF_REMOTESHUTDN_TEST        (986)
#define END_ADDRESS_OF_REMOTESHUTDN_TEST          (986)
#define MAX_LENGHT_OF_REMOTESHUTDN_TEST            (1)   // 65 + 1 = 66


#define START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ                  (992)
#define END_ADDRESS_OF_CPUINFO                                    (1002)
#define MAX_LENGHT_OF_CPUINFO                                      (11)   // 66 + 11 = 78


#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER0              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 1) // (992 + 1)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER1              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 2) // (992 + 2)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER2              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 3) // (992 + 3)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER3              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 4) // (992 + 4)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER4              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 5) // (992 + 5)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER5              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 6) // (992 + 6)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER6              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 7) // (992 + 7)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER7              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 8) // (992 + 8)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER8              (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 9) // (992 + 9)
#define  ADDRESS_OF_MAIN_CPU_FIRMWR_VER9             (START_ADDRESS_OF_CPUINFO_PROTOCOL_ID_INQ  + 10) // (992 + 10)


#define START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM                 (1010)
#define END_ADDRESS_OF_UPSMODEL                                  (1027)
#define MAX_LENGHT_OF_UPSMODEL                                    (17)  // 78 + 17 = 95


#define  ADDRESS_OF_BATT_STAND_VOLT_PER_UNIT          (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 1)//(1010 + 1)
#define  ADDRESS_OF_INP_PHASE                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 2)//(1010 + 2)
#define  ADDRESS_OF_OUTP_PHASE                        (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 3)//(1010 + 3)
#define  ADDRESS_OF_NOM_INP_VOLT                      (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 4)//(1010 + 4)
#define  ADDRESS_OF_NOM_OUTP_VOLT                     (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 5)//(1010 + 5)
#define  ADDRESS_OF_OUTP_PF                           (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 6)//(1010 + 6)
#define  ADDRESS_OF_OUTP_RATED_VA_LOW                 (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 7)//(1010 + 7)
#define  ADDRESS_OF_OUTP_RATED_VA_HIGH                (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 8)//(1010 + 8)
#define  ADDRESS_OF_UPSMODEL0                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 9)//(1010 + 9)
#define  ADDRESS_OF_UPSMODEL1                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 10)//(1010 + 10)
#define  ADDRESS_OF_UPSMODEL2                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 11)//(1010 + 11)
#define  ADDRESS_OF_UPSMODEL3                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 12)//(1010 + 12)
#define  ADDRESS_OF_UPSMODEL4                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 13)//(1010 + 13)
#define  ADDRESS_OF_UPSMODEL5                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 14)//(1010 + 14)
#define  ADDRESS_OF_UPSMODEL6                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 15)//(1010 + 15)
#define  ADDRESS_OF_UPSMODEL7                         (START_ADDRESS_OF_UPSMODEL_BATT_PIECE_NUM  + 16)//(1010 + 16)



#define START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT            (1162)
#define END_ADDRESS_OF_UPSRATINGINFO_OUTP_VOLT              (1165)
#define MAX_LENGHT_OF_UPSRATINGINFO                          (4)  // 95 + 4 = 99


#define  ADDRESS_OF_OUTP_CURRENT         (START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT   + 1)//(1162 + 1)
#define  ADDRESS_OF_OUTP_FREQ            (START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT   + 2)//(1162 + 1)
#define  ADDRESS_OF_OUTP_VOLT            (START_ADDRESS_OF_UPSRATINGINFO_BATT_VOLT   + 3)//(1162 + 1)


#define START_ADDRESS_OF_MODBUS_DATETIME    (1170)
#define MAX_LENGTH_OF_MODBUS_DATETIME       (6)  // 1170 + 6 = 1176
#define END_ADDRESS_OF_MODBUS_DATETIME      (START_ADDRESS_OF_MODBUS_DATETIME + MAX_LENGTH_OF_MODBUS_DATETIME)

#define MODBUS_DATE_ADDRESS            (START_ADDRESS_OF_MODBUS_DATETIME)
#define MODBUS_MONTH_ADDRESS           (START_ADDRESS_OF_MODBUS_DATETIME+1)
#define MODBUS_YEAR_ADDRESS            (START_ADDRESS_OF_MODBUS_DATETIME+2)
#define MODBUS_HOUR_ADDRESS            (START_ADDRESS_OF_MODBUS_DATETIME+3)
#define MODBUS_MINUTE_ADDRESS          (START_ADDRESS_OF_MODBUS_DATETIME+4)
#define MODBUS_SECONDS_ADDRESS         (START_ADDRESS_OF_MODBUS_DATETIME+5)
//----------------------------------------------------------------------------

typedef struct __attribute__((packed))
{
    uint16_t startadd;
    uint16_t endadd;
    uint16_t length;
    bool MBDataSentFlag;

}MODBUSTCP_MAP_STRUCT;

typedef union __attribute__((packed))
{
    uint16_t word;
    uint8_t  bytes[2];
    struct
    {
        uint16_t    bit0:1;
        uint16_t    bit1:1;
        uint16_t    bit2:1;
        uint16_t    bit3:1;
        uint16_t    bit4:1;
        uint16_t    bit5:1;
        uint16_t    bit6:1;
        uint16_t    bit7:1;
        uint16_t    bit8:1;
        uint16_t    bit9:1;
        uint16_t    bit10:1;
        uint16_t    bit11:1;
        uint16_t    bit12:1;
        uint16_t    bit13:1;
        uint16_t    bit14:1;
        uint16_t    bit15:1;
    }Bits;
 }MODBUS_16BIT_STRUCT;


//============================================================================
//                              > ENUMERATED TYPES <
//----------------------------------------------------------------------------

//Standard Modbus Error Codes for master/server
typedef enum
{
    MODBUS_EXCPT__SUCCESS               = 0x00,
    MODBUS_EXCPT__ILLEGAL_FNC           = 0x01,
    MODBUS_EXCPT__ILLEGAL_DATA_ADDR     = 0x02,
    MODBUS_EXCPT__ILLEGAL_DATA_VALUE    = 0x03,
    MODBUS_EXCPT__SLAVE_DEVICE_FAILURE  = 0x04,
    MODBUS_EXCPT__ACKNOWLEDGE           = 0x05,   // Modbus Standard
    MODBUS_EXCPT__SLAVE_DEVICE_BUSY     = 0x06,   // Modbus Standard
    MODBUS_EXCPT__NEGATIVE_ACKNOWLEDGE  = 0x07,   // Modbus Standard
    MODBUS_EXCPT__MEMORY_PARITY_ERROR   = 0x08   // Modbus Standard
} ENUM_MODBUS_EXCEPTION_CODE;

// Standard Modbus function Codes for master/server
typedef enum
{
    MODBUS_FNC_READ_COILS           = 0x01,     //Read Coil Status (0xxxx)
    MODBUS_FNC_READ_INPUTS          = 0x02,     //Read Input Status (1xxxx)
    MODBUS_FNC_READ_HOLDING_REG     = 0x03,     //Read Holding Registers (4xxxx)
    MODBUS_FNC_READ_INPUT_REG       = 0x04,     //Read Input Registers (3xxxx)
    MODBUS_FNC_WRITE_SINGLE_COIL    = 0x05,     //Write Single Coil (0xxxx)
    MODBUS_FNC_WRITE_SINGLE_REG     = 0x06,     //Write Single Register (4xxxx)
    MODBUS_FNC_WRITE_MULTI_COILS    = 0x0F,     //Write Multiple Coils (0xxxx)
    MODBUS_FNC_WRITE_MULTI_REG      = 0x10,     //Write Multiple Registers (4xxxx)
    MODBUS_FNC_WRITE_MASK_REG       = 0x16, // optional?
    MODBUS_FNC_READ_WRITE_MULTI_REG = 0x17 // optional?
} ENUM_MODBUS_FNC;

//============================================================================
//                              > STRUCTURES and TYPEDEFS <
//----------------------------------------------------------------------------
typedef struct __attribute__((packed))
{
    uint8_t functionCode; //yet to decided Function code is 1 byte or 2 byte
    uint16_t startAddress;
    uint16_t length;
    uint8_t groupindex;
}COM_Request;

typedef struct __attribute__((packed))
{
    // Link counters - 7
    uint32_t  framesRecv;
    uint32_t  frameSent;
    uint32_t  goodTransaction;
    uint32_t  goodException;
    uint32_t  clientConnections;
    uint32_t  otherError;

} MODBUS_TCP_DIAG_COUNTER;

//----------------------------------------------------------------------------

typedef struct __attribute__((packed))
{
    // ============
    int sockfd;
    struct sockaddr_in remote;
    // =============
    // Receiver
    uint8_t      rxBuffer[MBTCP_BUF_SIZE];    // NULL on init
    uint16_t     rxBufferSize;  // 0 on init
    // =============
    // Transmitter
    uint8_t      txBuffer[MBTCP_BUF_SIZE];    // NULL on init
    uint16_t     txRemainingSize;  // 0 on init

    uint32_t connectionTimeoutTime;
    uint32_t startTimeConnectionTimeout;
} MODBUS_TCP_CONN_STRUCT;


typedef struct __attribute__((packed))
{
    // ============

    MODBUS_TCP_DIAG_COUNTER    modbusDiag;
    MODBUS_TCP_CONN_STRUCT     modbusConnection[MBTCP_MAX_CONNECTION];

    uint8_t mb_clientIndex;
    BOOL modbusTCPEnable;
    int listenSockfd;
    MODBUS_16BIT_STRUCT shutdownRestoreResults;
    MODBUS_16BIT_STRUCT SettingParamSuccFailResults;

} MODBUS_TCP_RAM_STRUCT;

//----------------------------------------------------------------------------

//============================================================================
//                              > EXTERNS <
//----------------------------------------------------------------------------

extern MODBUS_TCP_RAM_STRUCT modbusTCPStruct;
extern MODBUSTCP_MAP_STRUCT ModbusMAPUPSDt[MODBUS_NO_OF_GROUPS];

//----------------------------------------------------------------------------

//============================================================================
//                              > FUNCTION PROTOTYPES <
//----------------------------------------------------------------------------

void ModbusTCPServer_Task();
void ModbusTCPServer_Init(void);
uint32_t ModbusTCPServer_DiagnosticParameters(uint16_t ParamIndex, BOOL Reset);
void ModbusTCPSever_UpdateConnectionTimeout(uint16_t Timout);

//----------------------------------------------------------------------------


#endif //COMMODBUSSERVER_H
//==============================================================================
// END OF COMMODBUSSERVER_H
//==============================================================================
