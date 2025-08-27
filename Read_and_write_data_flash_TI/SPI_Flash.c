
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ti/drivers/power/PowerCC26X2.h>
#include <ti_drivers_config.h>
#include "common_file.h"

/* Example/Board Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>

/* Example/Board Header files */
#include "SPI_Flash.h"

SPI_Params      spiParams;
SPI_Handle      masterSpi;
SPI_Transaction transaction;
bool            transferOK;


unsigned char write_enable_command = 0x06;
unsigned char write_disable_command = 0x04;

unsigned char busyFlag[3];

/*Commands for the Device ID Reading*/
unsigned char readDeviceID[2] = {0x9F, 0x00};//0x9E;//0xAF;//;
unsigned char status_address[2] = {0x0F, 0xC0};
unsigned char blocklock_address[2] = {0x0F, 0xA0};
unsigned char otp_address[2] = {0x0F, 0xB0};
unsigned char deviceId[2] = {0x00, 0x00};


/*--------------TRACK_ERASE PROGRESS----------------*/
/*--------------------------------------------------
 * 0 - No Operation
 * 1 - Erase Operation is Ongoing
 * 2 - Erase Operation is Complete
 * 3 - Erase Verification is Ongoing
 * 4 - Erase Verification is Complete
 *
 * */
void SPI_Begin(void) {
  SPI_init();
  SPI_Params_init(&spiParams);
  spiParams.mode                = SPI_MASTER;
  spiParams.frameFormat         = SPI_POL1_PHA1;//SPI_POL1_PHA1;
  spiParams.bitRate             = 8000000;
  spiParams.transferMode        = SPI_MODE_BLOCKING;
  spiParams.transferTimeout     = SPI_WAIT_FOREVER;
  spiParams.transferCallbackFxn = NULL;
  spiParams.dataSize            = 8;
  masterSpi = SPI_open(Board_SPI_MASTER, &spiParams);
}
/*-------------------Reset FFH-------------------------------------------------*/
void deviceReset(void) {
  unsigned char deviceReset_address = 0xFF;
  SPI_TxRx(&deviceReset_address, statusData, 1);
}

int readDevice() {
  memset(statusData, 0x00, dataLen);
  SPI_TxRx(readDeviceID, statusData, 4);
  statusBusy();                             // wait while status registers are busy
  memcpy(deviceId, statusData + 2, 2);
  if (deviceId[0] == 0x2C && deviceId[1] == 0x24) {
    return 0x08;
  }
  else {
    return 0xff;
  }
}
void blocklock_disable()
{
        unsigned char data[3] = {0x1F, 0xA0, 0x00};
        SPI_TxRx(data, statusData, 3);
        statusBusy();     // wait while status registers are busy
}

unsigned char status_read_command_with_0A()
{
    unsigned char data[2] = {0x0F, 0xA0};
    statusData[2] = 0xFF;
    SPI_TxRx(data, statusData, 3);
//    printf("worlking");
    statusBusy();     // wait while status registers are busy
    return statusData[2];
}
unsigned char status_read_command()
{
    unsigned char data[2] = {0x0F, 0xC0};
    statusData[2] = 0xFF;
    SPI_TxRx(data, statusData, 3);
//    printf("worlking");
    statusBusy();     // wait while status registers are busy
    return statusData[2];
}

void WriteEnable()  {
//    printf("worlking1\n");
  SPI_TxRx(status_address, busyFlag, 3);
  statusBusy();
  if (isBitSet(busyFlag[2], TWO_BIT) == 0)
  {
//      printf("worlking2\n");
    SPI_TxRx(&write_enable_command, statusData, 1);
    SPI_TxRx(status_address, busyFlag, 3);
    while (isBitSet(busyFlag[2], TWO_BIT) == 0)
    {
//        printf("worlking3\n");
      SPI_TxRx(status_address, busyFlag, 3);
    }
  }

}

unsigned char blockErase_command[4];
unsigned char command[10];
void EraseSector_1(block_page d)
{
    uint32_t addr = ((uint32_t)d.block_a << 7) | d.page|d.bit<<6;
//    printf("this is the eriase addrese %d\n",addr);
  blockErase_command[0] = 0x52;
  blockErase_command[1] = (addr >> 16) & 0xFF;
  blockErase_command[2] = (addr >> 8) & 0xFF;
  blockErase_command[3] = addr & 0xFF;
  WriteEnable();
  SPI_TxRx(blockErase_command, command, 4);
  statusBusy();
}
void write_to_data_and_crc(col c,unsigned char*write_var,uint32_t len1,block_page d,unsigned char*write_crc,uint8_t CRC_LEN)
{
    memcpy(statusData,write_var,len1);
    memcpy(statusData+len1,write_crc,CRC_LEN);

    PageWrite_data(c,statusData,len1+CRC_LEN,d);

}

void PageWrite_data(col c, unsigned char* write_var1, uint32_t len, block_page d)
{
  pagewrite_0x02_command1[0] = 0x02;
  pagewrite_0x02_command1[1] =  (c.a >> 7) & 0x1F;
  pagewrite_0x02_command1[2] = c.a & 0xFF;
  memcpy(pagewrite_0x02_command1 + 3, write_var1, len);
  WriteEnable();
//  WriteEnable();
  SPI_TxRx(pagewrite_0x02_command1,statusData, len + 3);
  //=================================================
  uint32_t addr = ((uint32_t)d.block_a << 7) | d.page | d.bit << 6;
//  printf("this is the page write address %d\n",addr);
  write_again:
  pageexecute_0x10_command[0] = 0x10;
  pageexecute_0x10_command[1] = (addr >> 16) & 0xFF;
  pageexecute_0x10_command[2] = (addr >> 8) & 0xFF;
  pageexecute_0x10_command[3] = addr & 0xFF;
    WriteEnable();
    SPI_TxRx(pageexecute_0x10_command,statusData, 4);
    statusBusy();
    if(isBitSet(busyFlag[2], FOUR_BIT) == 1){
        goto write_again;
//        printf("this is the again the write write data measage\n");
    }
    WriteEnable();
    uint8_t statusCheck[2] = {0x0F, 0x00};
    SPI_TxRx(statusCheck, statusCheck, 2);
//    if (statusCheck[1] & 0x08) {
//        printf("Program failed! Bit 3 = 1\n");
//    } else {
//        printf("Write success!\n");
//    }
}

unsigned char pageread_0x13_command[4];
unsigned char pageread_0x03_command[4];

void read_to_data_and_crc(col c,unsigned char*read_var,uint32_t len1,block_page d,unsigned char*read_crc,uint8_t CRC_LEN)
{
    PageWrite_data(c,statusData,len1+CRC_LEN,d);
    memcpy(read_var,statusData,len1);
    memcpy(read_crc,statusData+len1,len1+CRC_LEN);
}


void PageRead_data(col c, block_page d, unsigned char* readData_var1, uint32_t len)
{
uint32_t addr = ((uint32_t)d.block_a << 7) | d.page | d.bit<<6;
//printf("this is the page read address %d\n",addr);
pageread_0x13_command[0] = 0x13;
pageread_0x13_command[1] = (addr >> 16) & 0xFF;    //(addr >> 16) & 0x1F;
pageread_0x13_command[2] = (addr >> 8) & 0xFF;
pageread_0x13_command[3] = addr & 0xFF;

  SPI_TxRx(pageread_0x13_command,statusData, 4);
  statusBusy();

  //---------------------------------------
  pageread_0x03_command[0] = 0x03;
  pageread_0x03_command[1] = (c.a >> 8) & 0x1F;
  pageread_0x03_command[2] = c.a & 0xFF;
  pageread_0x03_command[3] = 0x00;    // its a dummy byte


  SPI_TxRx(pageread_0x03_command,statusData,len + 4);
  memcpy(readData_var1, statusData + 4, len);
}


void SPI_TxRx(unsigned char* TX_buff, unsigned char* RX_buff, uint16_t Data_Length) {
  memset((void *) RX_buff, 0, Data_Length);
  transaction.count = Data_Length;
  transaction.txBuf = (void *) TX_buff;
  transaction.rxBuf = (void *) RX_buff;

  transferOK = SPI_transfer(masterSpi, &transaction);
  if (!transferOK) {
      printf("❌ SPI_TxRx transfer failed\n");
    }
}
void statusBusy() {
  memset(busyFlag, 0xFF, 3);
  SPI_TxRx(status_address, busyFlag, 3);
  while (isBitSet(busyFlag[2], ONE_BIT) == 1) {
    SPI_TxRx(status_address, busyFlag, 3);
  }
}

int isBitSet(int variable, int data) {
  return  (variable & data) ? 1 : 0;
}
void crc_32_write(unsigned char *data,uint32_t length,r_data*var)
{
    uint32_t crc_var = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc_var ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc_var = (crc_var >> 1) ^ ((crc_var & 1) ? 0xEDB88320 : 0);
        }
    }
    crc_var=~crc_var;
//    return crc_var;
    var->crc_write=crc_var;
}
void crc_32_read(unsigned char *data,uint32_t length,r_data*var)
{
    uint32_t crc_var = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc_var ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc_var = (crc_var >> 1) ^ ((crc_var & 1) ? 0xEDB88320 : 0);
        }
    }
    crc_var=~crc_var;
//    return crc_var;
    var->crc_read=crc_var;
}

//uint32_t crc_321(unsigned char *data,uint32_t length,unsigned char*crc)
//{
//    uint32_t crc_var = 0xFFFFFFFF;
//    for (size_t i = 0; i < length; i++)
//    {
//        crc_var ^= data[i];
//        for (uint8_t j = 0; j < 8; j++) {
//            crc_var = (crc_var >> 1) ^ ((crc_var & 1) ? 0xEDB88320 : 0);
//        }
//    }
//    crc_var=~crc_var;
//
//    crc[0]=(crc_var>>24)&0xFF;
//    crc[1]=(crc_var>>16)&0xFF;
//    crc[2]=(crc_var>>8)&0xFF;
//    crc[3]=(crc_var&0xFF);
//    return crc_var;
//}





bool is_leap_year(int year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}
//void epoch_to_datetime(uint32_t epoch, ram_data *dt)
void epoch_to_datetime(uint64_t epoch, ram_data *dt,r_data*var)
{
//    epoch++
    sprintf(var->eporch_id, "%llu", epoch);
    static const int days_in_month[12] = {
        31,28,31,30,31,30,31,31,30,31,30,31
    };

    dt->sec = epoch % 60;
    epoch /= 60;
    dt->min = epoch % 60;
    epoch /= 60;
    dt->hour = epoch % 24;
    uint32_t days = epoch / 24;

    dt->year = 1970;
    while (1) {
        int dim = is_leap_year(dt->year) ? 366 : 365;
        if (days >= dim) {
            days -= dim;
            dt->year++;
        } else {
            break;
        }
    }

    int m;
    for (m = 0; m < 12; m++) {
        int dim = days_in_month[m];
        if (m == 1 && is_leap_year(dt->year)) dim++;  // February leap year
        if (days >= dim) {
            days -= dim;
        } else {
            break;
        }
    }

    dt->month = m + 1;
    dt->day = days + 1;
      sprintf(var->eporch_time, "%02d:%02d:%02d",dt->hour, dt->min, dt->sec);

}
