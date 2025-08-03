
#include <stdio.h>
#include <bl_uart.h>
//#include <bl602_gpio.h>
//#include <bl602.h>
//#include <bl602_uart.h>
#include <bl602.h>
#include <bl602_glb.h>
#include <bl_timer.h>
#include <bl_gpio.h>
#include "stdio.h"
#include <assert.h>
//#include <FreeRTOS.h>
//#include <task.h>
//#include <utils_log.h>

#define SIM_EN_PIN 17
#define UART_PORT   1
#define MAX_RESPONSE_LEN 1024     

void read_uart_response_convert_string(void) {
    char response[MAX_RESPONSE_LEN];
    int index = 0;

    uint32_t timeout_ms = 1000;  // Timeout nếu không nhận được byte nào mới sau 1 giây
    uint32_t last_recv_time = bflb_platform_get_time_ms(); // Bạn cần có hàm lấy thời gian ms

    while (1) {
        int ch = bl_uart_data_recv(UART_PORT);
        if (ch >= 0 && ch != 0xFF) {
            // Nếu là ký tự in được, lưu vào buffer
            if (index < MAX_RESPONSE_LEN - 1) {
                response[index++] = (char)ch;
            }
            last_recv_time = bflb_platform_get_time_ms(); // Cập nhật thời gian nhận cuối cùng
        }

        // Nếu timeout (không nhận thêm byte nào trong khoảng thời gian nhất định), kết thúc chuỗi
        if ((bflb_platform_get_time_ms() - last_recv_time) > timeout_ms) {
            break;
        }
    }

    response[index] = '\0';  // Null-terminate chuỗi

    printf("Phản hồi từ module SIM:\r\n%s\r\n", response);
}
void main(void)
{   int rc = bl_uart_init(
        UART_PORT,  //  UART Port 1
        4,          //  Tx Pin default is 4 
        3,          //  Rx Pin default is 3 
        255,        //  CTS Unused
        255,        //  RTS Unused
        115200      //  Baud Rate
    );
    assert(rc == 0);
    
     printf("Starting module Sim...\r\n");
     bflb_platform_delay_ms(1000);
    //  Enable the SIM module
    bl_gpio_enable_output(SIM_EN_PIN, 0, 0);
    bl_gpio_output_set(SIM_EN_PIN, 0);  //  Set the SIM Enable Pin to high
    bflb_platform_delay_ms(1000);  
    bl_gpio_output_set(SIM_EN_PIN, 1);
    bflb_platform_delay_ms(1000);  //  Wait for the SIM module to initialize  
    printf("Module Sim started\r\n");
    while (1) {
        int last_ch = 0;
        for (;;) {
            //  Read one byte from UART Port, returns -1 if nothing read
            int ch = bl_uart_data_recv(UART_PORT);
            if (ch < 0) { 
                bflb_platform_delay_ms(100);  //  Wait a bit before retrying
                continue; }  //  Loop until  receive something
            if (ch == 0xFF) { continue; }  //  Ignore 0xFF, which is not valid data
            if(ch >=0){
                read_uart_response_convert_string();
                break;  //  Break the loop if we received something
            }
            //if (ch != last_ch) { printf("0x%02x ", ch); last_ch = ch; }
        }
        printf("\r\nHand shaking to Module Sim succes\r\n");

        bl_uart_data_send(UART_PORT, 'A');
        bl_uart_data_send(UART_PORT, 'T');
        bl_uart_data_send(UART_PORT, '\r');
        bl_uart_data_send(UART_PORT, '\n');
        printf("Sent 'AT'\r\n");

        // Đọc phản hồi từ module SIM
        read_uart_response_convert_string();
       
        bflb_platform_delay_ms(2000);
    }

    
}

        //const char *msg = "AT+CCHOPEN\r\n";
        // const char *p = msg;
        // while (*p) {
        //     bl_uart_data_send(Monitor_ID, *p++);
        // }
        // bflb_platform_delay_ms(1000);