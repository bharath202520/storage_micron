/*
 * common_file.h
 *
 *  Created on: 03-Dec-2024
 *      Author: Admin
 */

#ifndef APPLICATION_COMMON_FILE_H_
#define APPLICATION_COMMON_FILE_H_

#include <stdint.h>
#include <ti/drivers/apps/LED.h>

#ifdef seconds_enable
#include <ti/sysbios/hal/Seconds.h>
#endif

/*declarations for the LED & GPIO Handles*/
//LED_Handle BOARD_LED_HANDLE;
//LED_Handle POWER_LED_HANDLE;

/*-------------------DECLRATION_OF_MAC_VARIABLE-------------------------*/
uint8_t mac_address[6];

/*-------------------DECLRATION_OF_RSSI_VARIABLE-------------------------*/
int8_t rssiData;

#define MAX_PACKET_SIZE                     90

#pragma pack(1)
typedef struct{
    int longPress;
    int longPressCounter;
    int long_press_counter;
    bool reboot;
}button_callback_event;


#ifdef seconds_enable
typedef struct{
    Seconds_Time timeStamp;
    uint64_t global_time;
    uint64_t epoch_unix_counter;
    unsigned char epochCounter;
}time_callback_var;
#endif

typedef struct{
    uint8_t Bat_periodic_event_count;
    uint8_t battery_percent;
}battery_callback_var;
//battery_event_var

typedef struct{
    unsigned char deviceID;
    unsigned char bitByte;
    uint32_t Flash_size;
    uint32_t Flash_98Percent;
    uint8_t erase_active;
}storage_callback_var;

typedef struct{
    uint32_t Flash_write_address;
    uint32_t Flash_read_address;
    uint32_t WriteCount;
    uint32_t ReadCount;
    uint8_t dummyData;
    uint64_t epoch_unix_counter;
}startup_registers;

typedef struct{
    int total_data_size;    // data sync pending
    uint32_t WriteCount;
    uint32_t ReadCount;
    uint32_t Flash_size;
    uint16_t seperator_one;
}uart_starting_data;


enum Uart_Mode{
    UART_SUCCESS_COMMAND,
    UART_FAILURE_COMMAND,
    UART_BREKURE_COMMAND,
    UART_FAULRES_COMMAND,
    UART_ERASE_COMMAND,
    UART_DATA_TRANSMIT,
//    UART_DATA_READY,
    UART_START_COMMAND = 0x10,
    UART_NOT_STARTED = 0xFF
};

typedef enum Uart_Mode Uart_Mode;

typedef struct{
    unsigned char mac_address[12];
    unsigned char return_command[8];
    uint8_t termination_byte[2];// = {0x0d, 0x0A};
    unsigned char board_mac_address[12];
    uint64_t return_command_val;
    uint8_t verify_mac_address;
    Uart_Mode Mode;
}uart_checksum;

typedef struct {
    size_t number_of_bytes_read;
    uint8_t input_uart_data[100];
    int input_uart_data_size;
}uart_data_receive;

typedef struct{
    int total_data_size;        // 2 byte
    uint32_t WriteCount;    //     4 byte     write count of the storage
    uint32_t ReadCount;     //     4 byte     read cout of the storage after dta getting transmit
    uint32_t Flash_size;    //     4 byte     total flash_size in calculation
    uint8_t storage_data[230]; // 230 Byte
    uint32_t data_sync_pending; // 4 Byte
    uint8_t termination_byte[2];// 2 Byte
}starting_data_structure;

typedef struct
{
    int total_data_size;
    uint32_t WriteCount;    //     4 byte     write count of the storage
    uint32_t ReadCount; //     4 byte     read cout of the storage after dta getting transmit
    uint32_t Flash_size;
    uint8_t termination_byte[2]; // 2 Byte
}storage_empty_msg;

typedef struct{
    uint8_t storage_data[230];  // 230 Byte
    uint32_t data_sync_pending; // 4 Byte
    uint8_t termination_byte[2];// 2 Bye
}internal_data_structure;

typedef struct{
    uint8_t storage_data[230];  // 230 Byte
    uint32_t data_sync_pending; // 4 Byte
    uint8_t termination_byte[2];// 2 Byte
    uint8_t terminate_line[2];  // 2 Byte
}ending_data_structure;

typedef struct{
    int data_to_transmit;
    int data_to_transmit_counter;
    uint8_t status;
    uint32_t sync_pending_data;
}uart_data_conditioning;


typedef struct{
    uint32_t WriteCount;
    uint32_t ReadCount;
    uint16_t read_blockNumber;
    uint16_t read_pageNumber;
    uint16_t read_columnNumber;
    uint16_t write_blockNumber;
    uint16_t write_pageNumber;
    uint16_t write_columnNumber;
    uint32_t Flash_write_address;
    uint32_t Flash_read_address;
    uint32_t Flash_read_Complete;
    uint32_t Flash_write_Complete;
    uint32_t Data_retrieve_per;
}storage_data;
#pragma pack()

#endif /* APPLICATION_COMMON_FILE_H_ */

// total_data_size = 2 byte
// WriteCount = 4 Byte
// ReadCount = 4 Byte
// Flash_size = 4 Byte

// Loop Start thay
// Storage Data = 230 Bytes
// Data_retrieve_per = 4 Byte
// termination_byte = 0x0D, 0x0A;

// termination_byte = 0x0C, 0x0C


/*----------------------OBSERVATIONS-------------------*/
/*  1) STORE 10, 100, 500 of blocks - write and read all the data - working properly.
 *  1) STORE 1000 Number of blocks - write and read all the data- Working Properly.
 *  2) Store 2000 Number of blocks - write and read all the data - Working Properly.
 *  3) Store 3000 Number of blocks - write and read all the data - Pending
 *      3.1) It was showing 1 data out of 37 data was broken and manipulated.
 *      3.2) I did re-read all the data, but still all the was the same without writing a new data.
 *          3.2.1) It confirms sometime 1 byte goes malfunction, causing data to be altered.
 *      3.3) I erase that particular one sector and rewrite and re-read the data
 *          3.3.1) it wont work, it was showing data 0xFF,
 *      3.4) 1 change i did mysteriously,
 *          -> change i from i to j in a loop
 *          -> it was causing an issue, which should not be a cause according to programmatic flow.
 *      3.5) problem was found in the register level of the BITMASK (Instead of using 0x7FF, we need to use 0xFFF)/
 *          -> Also when we erase 4095 blocks in total, there are few blocks which are bad in nature.
 *          -> we need to identify the bad blocks and remove it from usage, so i stored number of that block to the one variable of aray.
 *          -> According to datasheet for 4096 blocks, we may have 20 blocks which is bad and not correct.
 *          -> therefore considering that array of 100 int would suffice to these function.
 *  4) Store 4000 Number of blocks - write and read all the data - Pending
 *
 *  5) Erase the Storage Completely.
 *      5.1-> Check if first data of all the column is 0xff or not using compare function
 *      5.2-> compare and store it to the one variable, with iteration of 5.
 *      5.3-> store 0x01 if storage erased properly, store 0xff if storage erased not properly.
 *      5.4-> erase the storage again, if storage is properly erased and verified with the readblocks and all.
 *
 *  6) Store config data to the last healthy data.
 *
 *
 *  1 Hour - 21600 packet store thay
 *  1 Block : 1008 data packet storage
 *  1008*25 = 25200
 *
 */
