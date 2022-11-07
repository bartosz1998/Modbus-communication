#include "string.h"
#include "esp_log.h"
#include "params/modbus_params.h"
#include "mbcontroller.h"
#include "sdkconfig.h"

int temp_indoor;

#define MB_PORT_NUM     (2)   
#define MB_DEV_SPEED    (115200)  

#define CONFIG_MB_COMM_MODE_ASCII 1
#define CONFIG_MB_UART_TXD 23
#define CONFIG_MB_UART_RXD 22
#define CONFIG_MB_UART_RTS 18

#define UPDATE_CIDS_TIMEOUT_MS          (500)
#define UPDATE_CIDS_TIMEOUT_TICS        (UPDATE_CIDS_TIMEOUT_MS / portTICK_RATE_MS)

#define POLL_TIMEOUT_MS                 (1)
#define POLL_TIMEOUT_TICS               (POLL_TIMEOUT_MS / portTICK_RATE_MS)

#define INPUT_OFFSET(field) ((uint16_t)(offsetof(input_reg_params_t, field) + 1))
#define COIL_OFFSET(field) ((uint16_t)(offsetof(coil_reg_params_t, field) + 1))

#define STR(fieldname) ((const char*)( fieldname ))

#define OPTS(min_val, max_val, step_val) { .opt1 = min_val, .opt2 = max_val, .opt3 = step_val }

static const char *TAG2 = "MASTER MODBUS";

enum {
    MB_DEVICE_ADDR1 = 1 
};

enum {
    CID_INP_DATA_0 = 0,
    CID_INP_DATA_1,
    CID_INP_DATA_2,
    CID_INP_DATA_3,
    CID_INP_DATA_4,
    CID_INP_DATA_5,
};

/*************************************************** DATA TO SEND ************************************************/
/* Init message */

const mb_parameter_descriptor_t device_parameters[] = {
    { CID_INP_DATA_0, STR("Data_channel_0"), STR("Volts"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 0, 2,
                    INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -10, 10, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_1, STR("Temperature"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 0, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_2, STR("Relay_1_ON"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 2, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_3, STR("Relay_2_ON"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 4, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_4, STR("Relay_1_OFF"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 158, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
    { CID_INP_DATA_5, STR("Relay_2_OFF"), STR("C"), MB_DEVICE_ADDR1, MB_PARAM_INPUT, 160, 2,
            INPUT_OFFSET(input_data0), PARAM_TYPE_FLOAT, 4, OPTS( -40, 100, 1 ), PAR_PERMS_READ_WRITE_TRIGGER },
};

const uint16_t num_device_parameters = (sizeof(device_parameters)/sizeof(device_parameters[0]));

/********************************************** END DATA TO SEND **************************************************/

/****************************************** READ TEMPERATURE MODBUS ***********************************************/
/* Receive the message for temperature */

static void master_operation_func(void *arg)
{
    uint16_t cid = 1;
    esp_err_t err = ESP_OK;
    float value = 0;
    const mb_parameter_descriptor_t* param_descriptor = NULL;

    err = mbc_master_get_cid_info(cid, &param_descriptor);
    void* temp_data_ptr =  ((void*)&input_reg_params + param_descriptor->param_offset - 1);
    assert(temp_data_ptr);
    uint8_t type = 0;  
    err = mbc_master_get_parameter(cid, (char*)param_descriptor->param_key,(uint8_t*)&value, &type);
        if (err == ESP_OK) {
            *(float*)temp_data_ptr = value;
            temp_indoor = (int)value;
           
               ESP_LOGI(TAG2, "%s value = %f read successful.",
                        (char*)param_descriptor->param_key,
                                    value);
}}

/******************************************* END READ TEMPERATURE MODBUS ******************************************/

/********************************************** CONTROL LED 1 ON **************************************************/
/* Send command for Relay LED 1 ON */

static void master_operation_func2(void *arg)
{
    uint16_t cid2 = 2;
    esp_err_t err = ESP_OK;
    float value = 0;
    const mb_parameter_descriptor_t* param_descriptor = NULL;

    err = mbc_master_get_cid_info(cid2, &param_descriptor);
    void* temp_data_ptr =  ((void*)&input_reg_params + param_descriptor->param_offset - 1);
    assert(temp_data_ptr);
    uint8_t type = 0;  
    err = mbc_master_get_parameter(cid2, (char*)param_descriptor->param_key,(uint8_t*)&value, &type);
        if (err == ESP_OK) {
            *(float*)temp_data_ptr = value;
                ESP_LOGI(TAG2, "%s",
                        (char*)param_descriptor->param_key);
}}

/********************************************** END CONTROL LED 1 ON **********************************************/

/************************************************ CONTROL LED 2 ON ************************************************/
/* Send command for Relay LED 2 ON */

static void master_operation_func3(void *arg)
{
    uint16_t cid3 = 3;
    esp_err_t err = ESP_OK;
    float value = 0;
    const mb_parameter_descriptor_t* param_descriptor = NULL;

    err = mbc_master_get_cid_info(cid3, &param_descriptor);
    void* temp_data_ptr =  ((void*)&input_reg_params + param_descriptor->param_offset - 1);
    assert(temp_data_ptr);
    uint8_t type = 0;  
    err = mbc_master_get_parameter(cid3, (char*)param_descriptor->param_key,(uint8_t*)&value, &type);
        if (err == ESP_OK) {
            *(float*)temp_data_ptr = value;
                ESP_LOGI(TAG2, "%s",
                        (char*)param_descriptor->param_key);
}}

/************************************************ CONTROL LED 1 OFF ***********************************************/
/* Send command for Relay LED 1 OFF */

static void master_operation_func4(void *arg)
{
    uint16_t cid3 = 4;
    esp_err_t err = ESP_OK;
    float value = 0;
    const mb_parameter_descriptor_t* param_descriptor = NULL;

    err = mbc_master_get_cid_info(cid3, &param_descriptor);
    void* temp_data_ptr =  ((void*)&input_reg_params + param_descriptor->param_offset - 1);
    assert(temp_data_ptr);
    uint8_t type = 0;  
    err = mbc_master_get_parameter(cid3, (char*)param_descriptor->param_key,(uint8_t*)&value, &type);
        if (err == ESP_OK) {
            *(float*)temp_data_ptr = value;
                ESP_LOGI(TAG2, "%s",
                        (char*)param_descriptor->param_key);
}}

/********************************************* END CONTROL LED 1 OFF **********************************************/

/*********************************************** CONTROL LED 2 OFF ************************************************/
/* Send command for Relay LED 2 OFF */

static void master_operation_func5(void *arg)
{
    uint16_t cid3 = 5;
    esp_err_t err = ESP_OK;
    float value = 0;
    const mb_parameter_descriptor_t* param_descriptor = NULL;

    err = mbc_master_get_cid_info(cid3, &param_descriptor);
    void* temp_data_ptr =  ((void*)&input_reg_params + param_descriptor->param_offset - 1);
    assert(temp_data_ptr);
    uint8_t type = 0;  
    err = mbc_master_get_parameter(cid3, (char*)param_descriptor->param_key,(uint8_t*)&value, &type);
        if (err == ESP_OK) {
            *(float*)temp_data_ptr = value;
               ESP_LOGI(TAG2, "%s",
                        (char*)param_descriptor->param_key);
}}

/*********************************************** END CONTROL LED 2 OFF ********************************************/

/**************************************** INIT MASTER MODBUS COMMUNICATION ****************************************/
/* Start modbus master communication */

static esp_err_t master_init(void)
{
    mb_communication_info_t comm = {
            .port = MB_PORT_NUM,
#if CONFIG_MB_COMM_MODE_ASCII
            .mode = MB_MODE_ASCII,
#elif CONFIG_MB_COMM_MODE_RTU
            .mode = MB_MODE_RTU,
#endif
            .baudrate = MB_DEV_SPEED,
            .parity = MB_PARITY_NONE
    };
    void* master_handler = NULL;

    esp_err_t err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);
    MB_RETURN_ON_FALSE((master_handler != NULL), ESP_ERR_INVALID_STATE, TAG2,
                                "mb controller initialization fail.");
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
                            "mb controller initialization fail, returns(0x%x).",
                            (uint32_t)err);
    err = mbc_master_setup((void*)&comm);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
                            "mb controller setup fail, returns(0x%x).",
                            (uint32_t)err);

    // Set UART pin numbers
    err = uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD,
                              CONFIG_MB_UART_RTS, UART_PIN_NO_CHANGE);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
            "mb serial set pin failure, uart_set_pin() returned (0x%x).", (uint32_t)err);

    err = mbc_master_start();
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
                            "mb controller start fail, returns(0x%x).",
                            (uint32_t)err);

    // Set driver mode to Half Duplex
    err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
            "mb serial set mode failure, uart_set_mode() returned (0x%x).", (uint32_t)err);

    vTaskDelay(5);
    err = mbc_master_set_descriptor(&device_parameters[0], num_device_parameters);
    MB_RETURN_ON_FALSE((err == ESP_OK), ESP_ERR_INVALID_STATE, TAG2,
                                "mb controller set descriptor fail, returns(0x%x).",
                                (uint32_t)err);

    master_operation_func4(NULL);
    master_operation_func5(NULL);                           
    ESP_LOGI(TAG2, "Modbus master stack initialized...");
    return err;
}

/***************************************** END INIT MASTER MODBUS COMMUNICATION ***********************************/
