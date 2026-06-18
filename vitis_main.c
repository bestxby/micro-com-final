#include "xparameters.h"
#include "xuartps_hw.h"
#include "ff.h"
#include "sleep.h"
#include "xil_printf.h"

// 依据您的配置，UART1的基地址通常是 XPAR_PS7_UART_1_BASEADDR
// 新版 Vitis 可能会将其定义为 XPAR_XUARTPS_1_BASEADDR，请根据实际 xparameters.h 调整
#define UART_BASEADDR   XPAR_PS7_UART_1_BASEADDR 

FATFS fs;
FIL fil;

int main() {
    FRESULT res;
    char buffer[128];
    int idx = 0;
    UINT bytes_written;

    xil_printf("SD Logger Started...\n");

    // 1. 挂载 SD 卡
    res = f_mount(&fs, "0:/", 1);
    if (res != FR_OK) {
        xil_printf("Failed to mount SD card! Error code: %d\n", res);
        return -1;
    }

    xil_printf("SD card mounted successfully. Listening on EMIO UART...\n");

    while (1) {
        // 2. 轮询读取 EMIO 串口数据 (接收 STM32 的日志)
        if (XUartPs_IsReceiveData(UART_BASEADDR)) {
            char c = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
            buffer[idx++] = c;

            // 3. 遇到换行符，认为接收完了一条完整的日志 (STM32 发送的长度为 64)
            if (c == '\n' || idx >= 100) {
                buffer[idx] = '\0';
                xil_printf("Received: %s", buffer);
                
                // 4. 打开文件并追加写入
                res = f_open(&fil, "0:/ENV_LOG.TXT", FA_OPEN_APPEND | FA_WRITE);
                if (res == FR_OK) {
                    f_write(&fil, buffer, idx, &bytes_written);
                    f_close(&fil);

                    // 5. 写入成功，向 STM32 回传 0x00 (SAVED)
                    XUartPs_WriteReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET, 0x00);
                } else {
                    // 写入失败，向 STM32 回传 0x01 (ERROR)
                    XUartPs_WriteReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET, 0x01);
                }
                
                idx = 0; // 重置缓冲
            }
        }
    }
    return 0;
}
