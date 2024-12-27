#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef volatile u8 vu8;
typedef volatile uint16_t vu16;
typedef volatile u32 vu32;

#define HID_PAD           (*(vu32 *)0x10146000 ^ 0xFFF)
#define NOTI_LED_REGISTER 0x2D
#define WIFI_LED_REGISTER 0x2A
#define I2C1_REGS_BASE    (0x10161000)
#define I2C2_REGS_BASE    (0x10144000)
#define I2C3_REGS_BASE    (0x10148000)
#define I2C_STOP          (1u)
#define I2C_START         (1u<<1)
#define I2C_ERROR         (1u<<2)
#define I2C_DIRE_WRITE    (0u)
#define I2C_IRQ_ENABLE    (1u<<6)
#define I2C_ENABLE        (1u<<7)
#define I2C_GET_ACK(reg)  ((bool)((reg)>>4 & 1u))

typedef struct
{
    vu8  REG_I2C_DATA;
    vu8  REG_I2C_CNT;
    vu16 REG_I2C_CNTEX;
    vu16 REG_I2C_SCL;
} I2cRegs;

static bool i2cStartTransfer(u8 regAddr, bool read, I2cRegs *const regs)
{
    const u8 devAddr = 0x4A;


    u32 i = 0;
    for(; i < 8; i++)
    {
        while(regs->REG_I2C_CNT & I2C_ENABLE);

        // Select device and start.
        regs->REG_I2C_DATA = devAddr;
        regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_START;
        while(regs->REG_I2C_CNT & I2C_ENABLE);
        if(!I2C_GET_ACK(regs->REG_I2C_CNT)) // If ack flag is 0 it failed.
        {
            regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
            continue;
        }

        // Select register and change direction to write.
        regs->REG_I2C_DATA = regAddr;
        regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_DIRE_WRITE;
        while(regs->REG_I2C_CNT & I2C_ENABLE);
        if(!I2C_GET_ACK(regs->REG_I2C_CNT)) // If ack flag is 0 it failed.
        {
            regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
            continue;
        }

        // Select device in read mode for read transfer.
        if(read)
        {
            regs->REG_I2C_DATA = devAddr | 1u; // Set bit 0 for read.
            regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_START;
            while(regs->REG_I2C_CNT & I2C_ENABLE);
            if(!I2C_GET_ACK(regs->REG_I2C_CNT)) // If ack flag is 0 it failed.
            {
                regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
                continue;
            }
        }

        break;
    }

    if(i < 8) return true;
    else return false;
}

u32 waitInput()
{
  u32 key;
  while(true)
  {
    key = HID_PAD;

    if(!key)continue;
    u32 i;
    for(i = 0; i < 0x13000 && key == HID_PAD; i++);
    if(i == 0x13000) break;
  }

  return key;
}

bool LED(u8 regAddr, u8 data)
{
  const u8 *in = &data;
  int size = 1;
  I2cRegs *regs = (I2cRegs*)I2C2_REGS_BASE;
          
  if(!i2cStartTransfer(regAddr, false, regs)) return false;

  while(--size)
  {
    regs->REG_I2C_DATA = *in++;
    regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_DIRE_WRITE;
    while(regs->REG_I2C_CNT & I2C_ENABLE);
    if(!I2C_GET_ACK(regs->REG_I2C_CNT))
    {
      regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
      return false;
    }
  }

  regs->REG_I2C_DATA = *in;
  regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_DIRE_WRITE | I2C_STOP;
  while(regs->REG_I2C_CNT & I2C_ENABLE);
  if(!I2C_GET_ACK(regs->REG_I2C_CNT))
  {
    regs->REG_I2C_CNT = I2C_ENABLE | I2C_IRQ_ENABLE | I2C_ERROR | I2C_STOP;
    return false;
  }

  return true;
}

void I2C_init(void)
{
  u8 bus[3] = {I2C1_REGS_BASE, I2C2_REGS_BASE, I2C3_REGS_BASE};
  for(int i = 0; i < 2; i++){
    I2cRegs *regs = (I2cRegs*)bus[i];
    while(regs->REG_I2C_CNT & I2C_ENABLE);
    regs->REG_I2C_CNTEX = 2;
    regs->REG_I2C_SCL = 1280;
  }
}

void main(void)
{
  I2C_init();

  int noti_led_status = LED(NOTI_LED_REGISTER, 4);
  int wifi_led_status = LED(WIFI_LED_REGISTER, 8);
  
  while(noti_led_status & wifi_led_status){
    if(!LED(NOTI_LED_REGISTER, (u8)waitInput()))break;
  }
}