#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include <esp_http_server.h>
#include "driver/gpio.h"
#include "esp_netif_types.h"
#include "web_server/start_wifi.h"
#include "modbus/modbus_master.h"
#include "esp_vfs.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define GPIO_LED_1 GPIO_NUM_26 //LED 1 PIN 26
#define GPIO_LED_2 GPIO_NUM_25 //LED 2 PIN 25

bool event_1 = false;
bool event_2 = false;
bool event_3 = false;
bool event_4 = false;

int K; 

SemaphoreHandle_t xSemaphore = NULL;

/******************************************* CONTROL STATUS GPIO PIN BUTTON ***************************************/
/* Choose the method control LED 1 PIN */

static void led_status(int P,int S)
{
    if(P == 1 && K == 0){
        if(S == 1 ){
            gpio_set_level(GPIO_LED_2,1);
             event_2 = true;
        }else if(S == 0){
            gpio_set_level(GPIO_LED_2,0);
             event_2 = false;
        }
    }else if(P == 2){
        if(S == 1){
            gpio_set_level(GPIO_LED_2,1);
             event_2 = true;
             K = 1;
        }else if(S == 0){
            gpio_set_level(GPIO_LED_2,0);
             event_2 = false;
             K = 0;
        }
    } 
}

/******************************************* END CONTROL STATUS GPIO PIN BUTTON ***********************************/

/************************************************ CONFIGURE GPIO PIN LED ******************************************/
/* Start configure gpio PIN */

static void configure_gpio(int PIN)
{
    gpio_reset_pin(PIN);

    gpio_set_direction(PIN,GPIO_MODE_OUTPUT);
}

/********************************************** END CONFIGURE GPIO PIN LED ****************************************/

/************************************************** LED 1 MASTER CONTROL ******************************************/

static esp_err_t event1_handler(httpd_req_t *req){
   if(event_1 == false){
      httpd_resp_send(req,"on",HTTPD_RESP_USE_STRLEN); 
      event_1 = true;
      gpio_set_level(GPIO_LED_1,1);
   }else{
      httpd_resp_send(req,"off",HTTPD_RESP_USE_STRLEN); 
      event_1 = false;
      gpio_set_level(GPIO_LED_1,0);
   } 
    return ESP_OK;
}

/************************************************* END LED 1 MASTER CONTROL ***************************************/

/*************************************************** LED 2 MASTER CONTROL *****************************************/

static esp_err_t event2_handler(httpd_req_t *req){
   if(event_2 == false){
      httpd_resp_send(req,"on",HTTPD_RESP_USE_STRLEN); 
      led_status(2,1);
   }else{
      httpd_resp_send(req,"off",HTTPD_RESP_USE_STRLEN); 
      led_status(2,0);
   } 
    return ESP_OK;
}

/************************************************* END LED 2 MASTER CONTROL ***************************************/

/************************************************** LED 1 SLAVE CONTROL *******************************************/

static esp_err_t event3_handler(httpd_req_t *req){
   if(event_3 == false){
      httpd_resp_send(req,"on",HTTPD_RESP_USE_STRLEN); 
      event_3 = true;
      master_operation_func2(NULL);
   }else{
      httpd_resp_send(req,"off",HTTPD_RESP_USE_STRLEN); 
      event_3 = false;
      master_operation_func4(NULL);
   } 
    return ESP_OK;
}

/************************************************* END LED 1 SLAVE CONTROL ****************************************/

/*************************************************** LED 2 SLAVE CONTROL ******************************************/

static esp_err_t event4_handler(httpd_req_t *req){
   if(event_4 == false){
      httpd_resp_send(req,"on",HTTPD_RESP_USE_STRLEN); 
      event_4 = true;
      master_operation_func3(NULL);
   }else{
      httpd_resp_send(req,"off",HTTPD_RESP_USE_STRLEN); 
      event_4 = false;
      master_operation_func5(NULL);
   } 
    return ESP_OK;
}

/************************************************* END LED 2 SLAVE CONTROL ****************************************/

/************************************************** STATE PROGRAM TO SEND *****************************************/

static esp_err_t event_value_handler(httpd_req_t *req){
    char event_1k[] = " ";
    itoa((int)event_1,event_1k,10);
    char event_2k[] = " ";
    itoa((int)event_2,event_2k,10);
    strcat(event_1k,event_2k);
    char event_3k[] = " ";
    itoa((int)event_3,event_3k,10);
    strcat(event_1k,event_3k);
    char event_4k[] = " ";
    itoa((int)event_4,event_4k,10);
    strcat(event_1k,event_4k);

    httpd_resp_send(req,event_1k,HTTPD_RESP_USE_STRLEN); 

    return ESP_OK;
}

/************************************************* END STATE PROGRAM TO SEND **************************************/

/******************************************** READ TEMPERATURE OUTDOOR/SLAVE PIN **********************************/

static esp_err_t outdoot_temp_value_handler(httpd_req_t *req){
    master_operation_func(NULL);

    char temp_actual[] = " ";
    itoa(temp_indoor,temp_actual,10);

    httpd_resp_send(req,temp_actual,HTTPD_RESP_USE_STRLEN); 
    return ESP_OK;
}

/******************************************* END READ TEMPERATURE OUTDOOR/SLAVE PIN *******************************/

/******************************************** READ TEMPERATURE INDOOR/MASTER PIN **********************************/

static esp_err_t indoot_temp_value_handler(httpd_req_t *req){
    int temp_input = 10;
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(GPIO_TEMP_ACTUAL, ADC_ATTEN_DB_11);

    temp_input = (((((adc1_get_raw(GPIO_TEMP_ACTUAL)*5.0)/1024.0)/10)-0.5)*100);
    char temp_actual[] = " ";
    itoa(temp_input,temp_actual,10);

    httpd_resp_send(req,temp_actual,HTTPD_RESP_USE_STRLEN); //temperatura indoor
    return ESP_OK;
}

/******************************************* END READ TEMPERATURE INDOOR/MASTER PIN *******************************/

/******************************************************* START WEB SIDE *******************************************/

static esp_err_t root_led(httpd_req_t *req)
{
    esp_err_t error;
    const char* response = (const char*) req->user_ctx; 
    error = httpd_resp_send(req,response,strlen(response));
    if(error != ESP_OK){
        ESP_LOGI(TAG3, "Error %d while seding Response", error);
    }
    else ESP_LOGI(TAG3,"Response sent sucessfully");
    return error;
}

/****************************************************** END START WEB SIDE *****************************************/

/****************************************************** CONTROL WEB SERVER  ****************************************/

static void server_get()
{
     static const httpd_uri_t event_value = {
      .uri       = "/event_value",
      .method    = HTTP_GET,
      .handler   = event_value_handler,
  };

    static const httpd_uri_t event1 = {
      .uri       = "/event1",
      .method    = HTTP_GET,
      .handler   = event1_handler,
  };

  static const httpd_uri_t event2 = {
      .uri       = "/event2",
      .method    = HTTP_GET,
      .handler   = event2_handler,
  };

   static const httpd_uri_t event3 = {
      .uri       = "/event3",
      .method    = HTTP_GET,
      .handler   = event3_handler,
  };

   static const httpd_uri_t event4 = {
      .uri       = "/event4",
      .method    = HTTP_GET,
      .handler   = event4_handler,
  };


  static const httpd_uri_t outdoot_temp_value = {
      .uri       = "/outdoot_temp_value",
      .method    = HTTP_GET,
      .handler   = outdoot_temp_value_handler,
  };


     static const httpd_uri_t indoot_temp_value = {
      .uri       = "/indoot_temp_value",
      .method    = HTTP_GET,
      .handler   = indoot_temp_value_handler,
  };

  static const httpd_uri_t root = {
      .uri       = "/",
      .method    = HTTP_GET,
      .handler   = root_led,
      .user_ctx  = chunk2
  };

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    if(httpd_start(&server, &config)==ESP_OK)
    {
        ESP_LOGI(TAG3, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server,&indoot_temp_value);
        httpd_register_uri_handler(server,&outdoot_temp_value);
        httpd_register_uri_handler(server,&event1);
        httpd_register_uri_handler(server,&event2);
        httpd_register_uri_handler(server,&event3);
        httpd_register_uri_handler(server,&event4);
        httpd_register_uri_handler(server,&event_value);
    }

   xSemaphoreGive(xSemaphore); 
   vTaskDelete(NULL);
}

/*************************************************** END CONTROL WEB SERVER  ***************************************/

/*************************************************** CONTROL GPIO PIN BUTTON ***************************************/
/* Read status pin button */

static void gpio_state()
{
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY)){
        for( ;; )
        {
            if(gpio_get_level(GPIO_NUM_27) == 1)
                {
                    led_status(1,1);
                }
                else
                {
                    led_status(1,0);
                }

            vTaskDelay(500/portTICK_PERIOD_MS);
        }
    }
    vTaskDelete(NULL);
}

/************************************************* END CONTROL GPIO PIN BUTTON *************************************/

void app_main(void)
{
    configure_gpio(GPIO_LED_1);
    configure_gpio(GPIO_LED_2);

    gpio_set_direction(GPIO_NUM_27,GPIO_MODE_INPUT);

    master_init(); //Modbus init
    vTaskDelay(10);

    wifi_start();

    xSemaphore = xSemaphoreCreateBinary(); 

    xTaskCreate(replace_HTML, "start", 30000, NULL, 1, NULL); // Convert html to C
    xTaskCreate(server_get, "server_start", 2048, NULL, 3, NULL); // Control web server
    xTaskCreate(gpio_state, "print_task", 2048, NULL, 4, NULL); // GPIO control
}