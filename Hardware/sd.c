#include "sd.h"

/* Volatile flag for SPI clock speed scaling */
volatile uint8_t sd_spi_speed_fast = 0;

/* Microsecond-level software delay for timing */
static void delay_us(uint32_t us)
{
    volatile uint32_t count = us * 8; /* Approx 8 loops per us at 72MHz */
    while (count--);
}

/* Helper macros for CS pin control */
#define SD_CS_LOW()     (SD_CS_PORT->BRR  = SD_CS_PIN)
#define SD_CS_HIGH()    (SD_CS_PORT->BSRR = SD_CS_PIN)

/**
  * @brief  初始化 SD 卡所使用的 GPIO 引脚并禁用 JTAG（保留 SWD）。
  * @param  无
  * @retval 无
  */
static void SD_SPI_Init(void)
{
    /* 开启 GPIOA, GPIOB 和 AFIO 时钟 (IOPAEN=bit2, IOPBEN=bit3, AFIOEN=bit0) */
    RCC->APB2ENR |= (1 << 2) | (1 << 3) | (1 << 0);
    (void)RCC->APB2ENR; /* 刷新流水线 */
    
    /* 禁用 JTAG，释放 PB3, PB4 作为普通 GPIO 使用 (SWJ_CFG=010) */
    AFIO->MAPR = (AFIO->MAPR & ~0x07000000) | 0x02000000;
    
    /* PB3, PB4, PB5 -> 推挽输出 50MHz (CNF=00, MODE=11 -> 0x3) */
    GPIOB->CRL &= ~0x00FFF000;
    GPIOB->CRL |=  0x00333000;
    
    /* PA11 -> 上拉输入 (CRH bits 15:12 = 0x8) */
    GPIOA->CRH &= ~0x0000F000;
    GPIOA->CRH |=  0x00008000;
    GPIOA->ODR |=  (1 << 11); /* 使能 PA11 上拉 */
    
    /* 初始化片选高电平 */
    SD_CS_HIGH();
}

/**
  * @brief  模拟 SPI 读写一个字节。
  * @param  Data: 写入总线的数据。
  * @retval 从总线读取的数据。
  */
static uint8_t SD_WriteByte(uint8_t Data)
{
    uint8_t receive = 0;
    
    for (int i = 0; i < 8; i++)
    {
        /* 输出 MOSI 数据位 */
        if (Data & 0x80)
        {
            SD_MOSI_PORT->BSRR = SD_MOSI_PIN;
        }
        else
        {
            SD_MOSI_PORT->BRR = SD_MOSI_PIN;
        }
        Data <<= 1;
        
        /* SCK 拉低 */
        SD_SCK_PORT->BRR = SD_SCK_PIN;
        if (!sd_spi_speed_fast) delay_us(5); /* 初始化时限制在 100KHz */
        
        /* SCK 拉高 */
        SD_SCK_PORT->BSRR = SD_SCK_PIN;
        
        /* 采样 MISO */
        receive <<= 1;
        if (SD_MISO_PORT->IDR & SD_MISO_PIN)
        {
            receive |= 0x01;
        }
        if (!sd_spi_speed_fast) delay_us(5);
    }
    
    return receive;
}

static uint8_t SD_ReadByte(void)
{
    return SD_WriteByte(0xFF);
}

/**
  * @brief  向 SD 卡发送 6 字节的 SPI 物理命令帧。
  * @param  Cmd: 命令号 (如 CMD0 则传入 0)。
  * @param  Arg: 4 字节的命令参数。
  * @param  Crc: 1 字节的 CRC 校准。
  * @retval 无
  */
static void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
    SD_WriteByte(Cmd | 0x40);          /* 传输命令格式标志 */
    SD_WriteByte((uint8_t)(Arg >> 24)); /* 参数 High-High */
    SD_WriteByte((uint8_t)(Arg >> 16)); /* 参数 High-Low */
    SD_WriteByte((uint8_t)(Arg >> 8));  /* 参数 Low-High */
    SD_WriteByte((uint8_t)(Arg));       /* 参数 Low-Low */
    SD_WriteByte(Crc);                 /* CRC 标志 */
}

/**
  * @brief  获取 SD 卡的 R1 响应。
  * @param  Response: 预期的 R1 状态字节。
  * @retval 0 = 收到匹配响应, 0xFF = 超时。
  */
static SD_Error SD_GetResponse(uint8_t Response)
{
    uint32_t count = 0xFFFF;
    while (count--)
    {
        if (SD_ReadByte() == Response)
        {
            return SD_RESPONSE_NO_ERROR;
        }
    }
    return SD_RESPONSE_FAILURE;
}

/**
  * @brief  获取写入数据后的数据响应字节。
  * @param  无
  * @retval 数据响应标记 (如 0x05 代表写入成功)。
  */
static uint8_t SD_GetDataResponse(void)
{
    uint32_t count = 0xFFFF;
    uint8_t response;
    
    while (count--)
    {
        response = SD_ReadByte();
        if ((response & 0x11) == 0x01) /* 检查起始标志位与结束位 */
        {
            response &= 0x1F;
            break;
        }
    }
    
    /* 等待 SD 卡忙信号 (拉低为忙) 结束 */
    while (SD_ReadByte() == 0x00);
    
    return response;
}

/**
  * @brief  发送 CMD0 使得 SD 卡进入空闲 (Idle) SPI 模式。
  * @param  无
  * @retval R1 响应。
  */
static SD_Error SD_GoIdleState(void)
{
    SD_CS_LOW();
    SD_SendCmd(0, 0, 0x95); /* CMD0, 参数 0, CRC 0x95 */
    SD_Error rvalue = SD_GetResponse(SD_IN_IDLE_STATE);
    SD_CS_HIGH();
    SD_ReadByte(); /* 提供额外的 8 个时钟周期 */
    return rvalue;
}

/**
  * @brief  初始化 SD 卡通信。
  * @param  无
  * @retval 0 = 初始化成功, 0xFF = 失败。
  */
SD_Error SD_Init(void)
{
    sd_spi_speed_fast = 0; /* 开始时使用低速时钟 */
    SD_SPI_Init();
    
    /* 上电时需要将 CS 置高，并发送至少 74 个时钟脉冲以使卡准备就绪 */
    SD_CS_HIGH();
    for (int i = 0; i < 15; i++)
    {
        SD_WriteByte(0xFF);
    }
    
    /* 1. 复位进入 SPI 空闲模式 (CMD0) */
    uint32_t retry = 0;
    while (SD_GoIdleState() != SD_IN_IDLE_STATE)
    {
        retry++;
        if (retry > 100)
        {
            return SD_RESPONSE_FAILURE; /* 复位失败 */
        }
    }
    
    /* 2. 区分卡型号并向卡发送接口参数 (CMD8) */
    SD_CS_LOW();
    SD_SendCmd(8, 0x000001AA, 0x87); /* CMD8, 参数 0x1AA, CRC 0x87 */
    uint8_t r1 = SD_GetResponse(SD_IN_IDLE_STATE);
    if (r1 == SD_RESPONSE_NO_ERROR)
    {
        /* 丢弃 R7 响应返回的 4 字节数据 */
        SD_ReadByte();
        SD_ReadByte();
        SD_ReadByte();
        SD_ReadByte();
    }
    SD_CS_HIGH();
    SD_ReadByte();

    /* 3. 激活卡片并初始化其内部状态 (ACMD41 循环) */
    retry = 0;
    while (1)
    {
        /* ACMD41 需要先发送 CMD55 作为前导 */
        SD_CS_LOW();
        SD_SendCmd(55, 0, 0xFF);
        r1 = SD_ReadByte();
        SD_CS_HIGH();
        SD_ReadByte();
        
        if (r1 <= 1)
        {
            /* 发送 ACMD41，参数设为支持高容量卡 (HCS = bit 30 = 0x40000000) */
            SD_CS_LOW();
            SD_SendCmd(41, 0x40000000, 0xFF);
            r1 = SD_ReadByte();
            SD_CS_HIGH();
            SD_ReadByte();
            
            if (r1 == 0x00)
            {
                break; /* 初始化就绪 */
            }
        }
        else
        {
            /* CMD55 握手失败，降级尝试 CMD1 */
            SD_CS_LOW();
            SD_SendCmd(1, 0, 0xFF);
            r1 = SD_ReadByte();
            SD_CS_HIGH();
            SD_ReadByte();
            if (r1 == 0x00)
            {
                break; /* CMD1 初始化就绪 */
            }
        }
        
        retry++;
        if (retry > 400)
        {
            return SD_RESPONSE_FAILURE; /* 初始化超时 */
        }
        
        /* 延时以防死循环对卡供电造成压力 */
        for (volatile int d = 0; d < 1000; d++);
    }
    
    /* 4. 将块大小设置为标准的 512 字节 (CMD16) */
    SD_CS_LOW();
    SD_SendCmd(16, 512, 0xFF);
    r1 = SD_ReadByte();
    SD_CS_HIGH();
    SD_ReadByte();
    
    if (r1 != 0x00)
    {
        return SD_RESPONSE_FAILURE;
    }
    
    /* 初始化就绪，切换到高速 SPI 模式 */
    sd_spi_speed_fast = 1;
    return SD_RESPONSE_NO_ERROR;
}

/**
  * @brief  将一个 512 字节的扇区块写入 SD 卡指定物理地址。
  * @param  pBuffer: 保存待写入数据的内存缓冲区指针。
  * @param  WriteAddr: 目标物理起始地址 (通常为物理扇区号)。
  * @param  BlockSize: 必须为 512。
  * @retval 0 = 写入成功, 0xFF = 写入失败。
  */
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize)
{
    SD_Error rvalue = SD_RESPONSE_FAILURE;

    SD_CS_LOW();
    
    /* 1. 发送写块命令 CMD24 */
    SD_SendCmd(24, WriteAddr, 0xFF);
    
    /* 2. 等待 R1 无错误响应 (0x00) */
    if (SD_ReadByte() == 0x00)
    {
        /* 发送一个空闲字节 */
        SD_ReadByte();
        
        /* 3. 写入数据开始令牌 (0xFE) */
        SD_WriteByte(0xFE);
        
        /* 4. 传输 512 字节的数据块 */
        for (uint16_t i = 0; i < BlockSize; i++)
        {
            SD_WriteByte(*pBuffer);
            pBuffer++;
        }
        
        /* 5. 写入 2 字节假的 CRC 校验 */
        SD_WriteByte(0xFF);
        SD_WriteByte(0xFF);
        
        /* 6. 检测数据响应状态 */
        if (SD_GetDataResponse() == SD_DATA_OK)
        {
            rvalue = SD_RESPONSE_NO_ERROR;
        }
    }
    
    SD_CS_HIGH();
    SD_ReadByte();
    
    return rvalue;
}

/**
  * @brief  从 SD 卡的指定物理扇区地址读取 512 字节。
  * @param  pBuffer: 保存读取数据的内存缓冲区指针。
  * @param  ReadAddr: 起始物理地址 (通常为物理扇区号)。
  * @param  BlockSize: 必须为 512。
  * @retval 0 = 读取成功, 0xFF = 读取失败。
  */
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize)
{
    SD_Error rvalue = SD_RESPONSE_FAILURE;

    SD_CS_LOW();
    
    /* 1. 发送读块命令 CMD17 */
    SD_SendCmd(17, ReadAddr, 0xFF);
    
    /* 2. 等待 R1 响应 (0x00) */
    if (SD_ReadByte() == 0x00)
    {
        /* 3. 等待数据块开始令牌 (0xFE) */
        if (SD_GetResponse(0xFE) == SD_RESPONSE_NO_ERROR)
        {
            /* 4. 读取 512 字节的数据 */
            for (uint16_t i = 0; i < BlockSize; i++)
            {
                *pBuffer = SD_ReadByte();
                pBuffer++;
            }
            
            /* 5. 抛弃 2 字节 CRC */
            SD_ReadByte();
            SD_ReadByte();
            
            rvalue = SD_RESPONSE_NO_ERROR;
        }
    }
    
    SD_CS_HIGH();
    SD_ReadByte();
    
    return rvalue;
}
