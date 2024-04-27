#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_http_client.h"
#include "connect_wifi.h"
#include "DS1307.h"
#include <cJSON.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define LED_BUILTIN 2
#define BUTTON 15

bool button_state = 0;
bool flag=0;

static const char *TAG = "main";

#define HTTP_RESPONSE_BUFFER_SIZE 1024

// API key from OpenWeatherMap 
char open_weather_map_api_key[] = "8da942a014087749f002b478cdad1495";
//latitude and longitude
char lat[] = "9.353819884105555";
char lon[] = "76.96800136547792";

//#define HTTP_RESPONSE_BUFFER_SIZE 1024
char timestamp[24];
char *response_data = NULL;
size_t response_len = 0;
bool all_chunks_received = false;

void get_temp_pressure_humidity(const char *json_string)
{
   
    cJSON *root = cJSON_Parse(json_string);
    cJSON *obj = cJSON_GetObjectItemCaseSensitive(root, "main");

    float temp = cJSON_GetObjectItemCaseSensitive(obj, "temp")->valuedouble;
    int pressure = cJSON_GetObjectItemCaseSensitive(obj, "pressure")->valueint;
    int humidity = cJSON_GetObjectItemCaseSensitive(obj, "humidity")->valueint;
    printf("Temperature: %0.00fÂ°F\nPressure: %d hPa\nHumidity: %d%%\n", temp, pressure, humidity);
    
    cJSON_Delete(root);
    free(response_data);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Resize the buffer to fit the new chunk of data
            response_data = realloc(response_data, response_len + evt->data_len);
            memcpy(response_data + response_len, evt->data, evt->data_len);
            response_len += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH:
            all_chunks_received = true;
            ESP_LOGI("OpenWeatherAPI", "Received data: %s", response_data);
            get_temp_pressure_humidity(response_data);
            break;
        default:
            break;
    }
    return ESP_OK;
}


void openweather_api_http(void *pvParameters)
{
//link https://api.openweathermap.org/data/2.5/weather?lat=9.353819884105555&lon=76.96800136547792&appid=8da942a014087749f002b478cdad1495

    char open_weather_map_url[200];
    snprintf(open_weather_map_url,
             sizeof(open_weather_map_url),
             "%s%s%s%s%s%s%s",
             "http://api.openweathermap.org/data/2.5/weather?",
             "lat=",
                lat,
             "&lon=",
             lon,
             "&appid=",
             open_weather_map_api_key);

    esp_http_client_config_t config = {
        .url = open_weather_map_url,
        .method = HTTP_METHOD_GET,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);
    vTaskDelete(NULL);
}

void app_main(void)
{
    gpio_pad_select_gpio(LED_BUILTIN);
    gpio_set_direction(LED_BUILTIN, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(BUTTON);
    gpio_set_direction(BUTTON, GPIO_MODE_INPUT);

    ds1307Begin(true);
    //Config date and time
    ds1307SetDate(27,4,24,6);
    ds1307SetTime(10,50,0);

	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	connect_wifi();

    while (1)
    {
        button_state = gpio_get_level(BUTTON);

        if(button_state==1)
        {
            flag = !flag;
        }

        if(flag == 1)
        {   
            gpio_set_level(LED_BUILTIN, 1);
            //printf("weather\n");
            if (wifi_connect_status)
            {
                xTaskCreate(&openweather_api_http, "openweather_api_http", 8192, NULL, 6, NULL);
            }
            vTaskDelay( 1000 / portTICK_PERIOD_MS);

        }
        else
        {
            gpio_set_level(LED_BUILTIN, 0);
            //printf("clock\n");
            ds1307GetTimestamp(timestamp);
            printf("Timestamp: %s\n", timestamp);
            vTaskDelay( 1000 / portTICK_PERIOD_MS);

        }
    }
}