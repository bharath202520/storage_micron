/*
 * SPI_Flash.h
 *
 *  Created on: Nov 11, 2022
 *      Author: Pratik RP
 */

#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_

#define ONE_BIT   0b00000001
#define TWO_BIT   0b00000010
#define THREE_BIT 0b00000100
#define FOUR_BIT  0b00001000
#define FIVE_BIT  0b00010000
#define SIX_BIT   0b00100000
#define SEVEN_BIT 0b01000000
#define EIGHT_BIT 0b10000000
//#define b_a 0
//#define page_a 2


typedef struct
{
    unsigned char eporch_id[15];
    unsigned char eporch_time[10];
   uint32_t T;
   uint32_t F;
   uint32_t crc_write;
   uint32_t crc_read;
}r_data;
#define DATA_LEN 2048
//#define CRC_LEN 4
#define CRC_FIXED_LEN 4
#define dataLen 2200
unsigned char statusData[dataLen];

unsigned char write_crc[CRC_FIXED_LEN];
unsigned char read_crc[CRC_FIXED_LEN];
unsigned char write_var[DATA_LEN];
unsigned char readData_var[DATA_LEN];
unsigned char pageexecute_0x10_command[4];// this variable is using read command and address is store
unsigned char pagewrite_0x02_command1[3 +DATA_LEN+CRC_FIXED_LEN]; // this is the variable is using the command adress and  len data is store

void SPI_Begin(void);
void SPI_TxRx(unsigned char* TX_buff, unsigned char* RX_buff, uint16_t Data_Length);
void deviceReset(void);

//void readStatus(unsigned char* TX_buff, unsigned char* RX_buff);



void crc_32_write(unsigned char *data,uint32_t length,r_data*var);
void crc_32_read(unsigned char *data,uint32_t length,r_data*var);
//uint32_t crc_321(unsigned char *data,uint32_t length,unsigned char*crc);
typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int sec;
} ram_data;

ram_data ram_var;
bool is_leap_year(int year);
void WriteEnable();
typedef struct {
  uint16_t block_a;
  uint8_t bit;
  uint8_t page;
} block_page;

typedef struct {
  unsigned int a:13;
} col;




void epoch_to_datetime(uint64_t epoch, ram_data *dt,r_data*var);

//void sector_checkup(void);

void Store_ConfigRegister_write(unsigned char* value,uint8_t Bytes);
void Fetch_ConfigRegister_read(unsigned char* readData,uint8_t Bytes);

void PageRead(unsigned char* readData, uint8_t Bytes, bool address_generator);
//void PageRead(unsigned char* readData,uint8_t Bytes);

void PageWrite(unsigned char* value, uint8_t Bytes);
void PageWrite_data(col c, unsigned char* write_var1, uint32_t len, block_page d);
//void PageWrite_data(col c, unsigned char* write_var, uint32_t len, block_page d);
//void PageRead_data(col c, block_page d, unsigned char* readData_var, uint32_t len);
void PageRead_data(col c, block_page d, unsigned char* readData_var1, uint32_t len);

void pageExecute();

void ecg_data_write(unsigned char*, uint16_t );
void ecg_data_Read(unsigned char*, uint16_t );

void ecg_data_Read_blankcheck(unsigned char* data_R, uint16_t Bytes, bool address_generator);
//void ecg_data_Read_blankcheck(unsigned char* data_R, uint16_t Bytes);

int readDevice();
void readDevice_code();

// new code
void statusBusy();
void statusBusy_program();


void blocklock_disable();
//int blocklock_disable();


int blocklock_enable();
unsigned char status_read_command();
unsigned char status_read_command_with_0A();


int isBitSet(int variable, int data);

void read_address_generator();
void write_address_generator();
//void generate_raw_number();

//uint32_t generate_raw_number(uint32_t data, uint16_t page, uint16_t blockNumber);
//void retrieve_page_block(uint32_t data, uint16_t *page, uint16_t *blockNumber);

//void PageRead_data(col c, block_page d, unsigned char* readData_var1, uint32_t len)
//void write_to_data_and_crc(col c,unsigned char*write_var,uint32_t len1,block_page d,unsigned char*write_crc,uint8_t CRC_LEN);
void read_to_data_and_crc(col c,unsigned char*read_var,uint32_t len1,block_page d,unsigned char*read_crc,uint8_t CRC_LEN);
void write_to_data_and_crc(col c,unsigned char*write_var,uint32_t len1,block_page d,unsigned char*write_crc,uint8_t CRC_LEN);


#endif /* SPI_FLASH_H_ */
