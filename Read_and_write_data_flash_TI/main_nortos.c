#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "common_file.h"
#include<stdio.h>

#include <NoRTOS.h>

/* Example/Board Header files */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>

#include <ti/drivers/apps/LED.h>

#include <ti/drivers/power/PowerCC26X2.h>
#include <ti_drivers_config.h>
#include "SPI_Flash.h"
#include<stdlib.h>

/*Commands for the Device ID Reading*/
SPI_Params      spiParams;
SPI_Handle      masterSpi;
SPI_Transaction transaction;
bool            transferOK;




#pragma pack(1)
LED_Handle led0Handle0;
unsigned char d1[10];
unsigned char block_lock_before_status;
unsigned char block_lock_after_status;
unsigned char write_enable_before_status;
unsigned char write_enable_after_status;
unsigned char before_erase_sattus;
unsigned char after_erase_sattus;
unsigned char before_write_data_status;
unsigned char after_write_data_status;
unsigned char before_read_Data_status;
unsigned char after_read_Data_status;
#define bit_type 1
#define block_type 0

#define b_a  20
#define page_a 0
uint64_t epoch = 1751372804;
typedef struct
{
    uint64_t true_parts;
    uint64_t flas_parts;
}t_f_data;

t_f_data t_f_var;
unsigned char device_id_true_or_flase_Var;

#define total_s (b_a * 64)+ page_a
//r_data  r_var[total_s];
int main(void)  {
  Board_init();
  NoRTOS_start(); /* Start NoRTOS */
  SPI_Begin();
  led0Handle0 = LED_open(BOARD_LED, NULL);
  LED_setOn(led0Handle0, 1);
  r_data  r_var[total_s];
  deviceReset();
  col c={0};
  block_page d={0,0,0};
  uint32_t z=0;
  t_f_var.flas_parts=0;
  t_f_var.true_parts=0;
  device_id_true_or_flase_Var= readDevice();
  if (device_id_true_or_flase_Var==0x08)
  {
     strcpy((char*)d1,"correct");
  }
  else
  {
      strcpy((char*)d1,"wrong");

  }
  printf("<<<<%d>>>>\n",total_s);
  for(int j=0;j<b_a;j++)
  {
//      for(int k=0;k<=1;k++)
//      {

          for(int i=0;i<64;i++)
          {
                  d.block_a=j;
                  d.bit=1;
                  d.page=i;

        //            d={0,0,i};
              epoch_to_datetime(epoch, &ram_var,&r_var[z]);
              epoch=epoch+1;
              block_lock_before_status=status_read_command_with_0A();
              blocklock_disable();
              block_lock_after_status=status_read_command_with_0A();
         //---------------------------------------
              write_enable_before_status=status_read_command();
              WriteEnable();
              write_enable_after_status=status_read_command();
        //---------------------------------------
              before_erase_sattus=status_read_command();
              EraseSector_1(d);
              after_erase_sattus=status_read_command();
        //-----------------------------------
              before_write_data_status=status_read_command();
              memset(write_var,'z',DATA_LEN);
              crc_32_write(write_var,DATA_LEN,&r_var[z]);
              PageWrite_data(c,write_var,DATA_LEN,d);
              //write_to_data_and_crc(c,write_var,DATA_LEN,d,write_crc,CRC_FIXED_LEN);
              after_write_data_status=status_read_command();
                    z++;
                    if(z==total_s)
                    {
//                        z=0;
                      break;
                    }
          }
//      }
  }
//crc_write=crc_32(write_var,DATA_LEN,&r_var);
//r_var.crc_data=crc_write;
//crc_write=crc_32(write_var,DATA_LEN,write_crc);

//printf("-------This is the crc data %X\n",crc_write);
  z=0;

  for(int j=0;j<b_a;j++)
  {
          for(int i=0;i<64;i++)
          {
        //      d={0,0,i}
              d.block_a=j;
              d.bit=1;
              d.page=i;
        //      PageWrite_data(c,write_var,DATA_LEN,d);

              before_read_Data_status=status_read_command();
              PageRead_data(c,d,readData_var,DATA_LEN);
              //read_to_data_and_crc(c,readData_var,DATA_LEN,d,read_crc,CRC_FIXED_LEN);
              after_read_Data_status=status_read_command();

              crc_32_read(readData_var,DATA_LEN,&r_var[z]);
              //crc_read=crc_32(readData_var,DATA_LEN);
              //crc_read=crc_32(readData_var,DATA_LEN,read_crc);
              z++;
                    if(z==total_s)
                    {
                      break;
                    }
          }
  }
  for(int i=0;i<total_s;i++)
  {
      if(r_var[i].crc_write==r_var[i].crc_read)
      {
          //    printf("-------both crc values is success-----------\n");
          r_var[i].T=+1;
          t_f_var.true_parts++;
      }
      else
      {
          //    printf("-------both crc values is failed-------------\n");
          r_var[i].F=+1;
          t_f_var.flas_parts++;
      }
  }

  printf("%lld\t",t_f_var.true_parts);
  printf("%lld\n",t_f_var.flas_parts);
  return 0;
}
