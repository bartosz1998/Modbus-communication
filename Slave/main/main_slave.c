#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "mbcontroller.h"       
#include "modbus_params.h"      
#include "esp_log.h"            
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   
#define MB_SLAVE_ADDR   (CONFIG_MB_SLAVE_ADDR)      
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  

#define MB_PAR_INFO_GET_TOUT                (10) 
#define MB_CHAN_DATA_MAX_VAL                (6)
#define MB_READ_MASK                        (MB_EVENT_INPUT_REG_RD \
                                                | MB_EVENT_COILS_RD)
#define MB_WRITE_MASK                       ( MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK                  (MB_READ_MASK | MB_WRITE_MASK)

static const char *TAG = "SLAVE MODBUS";

int tempActual = 20;

/******************************************* CONFIGURE PIN GPIO ******************************************************/
static void configure_gpio(int PIN)
{
    gpio_reset_pin(PIN);

    gpio_set_direction(PIN, GPIO_MODE_OUTPUT);
}

/***************************************** END CONFIGURE PIN GPIO ***************************************************/
/* DATA TO SEND */

static void temp_actual_data(void)
{
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    
    tempActual = (((((adc1_get_raw(ADC1_CHANNEL_6)*5.0)/1024.0)/10)-0.5)*100);
    printf("Actual temperature %d \n",tempActual);

    input_reg_params.input_data0 = tempActual;
    input_reg_params.input_data1 = 1; //ON LED1
    input_reg_params.input_data2 = 1; //ON LED2

    input_reg_params.input_data4 = 0; //OFF LED1
    input_reg_params.input_data5 = 0; //OFF LED2
}

void app_main(void)
{
    configure_gpio(GPIO_NUM_25);
    configure_gpio(GPIO_NUM_26);

    mb_param_info_t reg_info; 
    mb_communication_info_t comm_info; 
    mb_register_area_descriptor_t reg_area; 

    esp_log_level_set(TAG, ESP_LOG_INFO);
    void* mbc_slave_handler = NULL;

    mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler); 

#if CONFIG_MB_COMM_MODE_ASCII
    comm_info.mode = MB_MODE_ASCII,
#elif CONFIG_MB_COMM_MODE_RTU
    comm_info.mode = MB_MODE_RTU,
#endif
    comm_info.slave_addr = MB_SLAVE_ADDR;
    comm_info.port = MB_PORT_NUM;
    comm_info.baudrate = MB_DEV_SPEED;
    comm_info.parity = MB_PARITY_NONE;
    mbc_slave_setup((void*)&comm_info);

    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = (offsetof(input_reg_params_t, input_data0) >> 1);
    reg_area.address = (void*)&input_reg_params.input_data0;
    reg_area.size = sizeof(float) << 2;
    mbc_slave_set_descriptor(reg_area);

    reg_area.type = MB_PARAM_INPUT;
    reg_area.start_offset = (offsetof(input_reg_params_t, input_data4) >> 1);
    reg_area.address = (void*)&input_reg_params.input_data4;
    reg_area.size = sizeof(float) << 2;
    mbc_slave_set_descriptor(reg_area);



    mbc_slave_start();


    uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD, 
                CONFIG_MB_UART_RTS,
                UART_PIN_NO_CHANGE);

    uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);

    ESP_LOGI(TAG, "Start modbus ...");

    for(;holding_reg_params.holding_data0 < MB_CHAN_DATA_MAX_VAL;) {
        mb_event_group_t event = mbc_slave_check_event(MB_READ_WRITE_MASK);
        

        if(event & MB_EVENT_INPUT_REG_RD){
            temp_actual_data();
            mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT);
            ESP_LOGI(TAG, "INPUT READ (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                (uint32_t)reg_info.time_stamp,
                (uint32_t)reg_info.mb_offset,
                (uint32_t)reg_info.type,
                (uint32_t)reg_info.address,
                (uint32_t)reg_info.size);
            if((uint32_t)reg_info.mb_offset == 2){
                gpio_set_level(GPIO_NUM_25,1);
            }else if((uint32_t)reg_info.mb_offset == 4){
                gpio_set_level(GPIO_NUM_26,1);
            }else if((uint32_t)reg_info.mb_offset == 158){
                gpio_set_level(GPIO_NUM_25,0);
            }else if((uint32_t)reg_info.mb_offset == 160){
                gpio_set_level(GPIO_NUM_26,0);
            }
        }}

    ESP_LOGI(TAG,"Modbus controller destroyed.");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
}
