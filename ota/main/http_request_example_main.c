#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <esp_efuse.h>
#include <esp_efuse_table.h>

static const char *TAG = "HTTPS_NO_VERIFY";
SemaphoreHandle_t xSemaphore;
char tempdata[1024] = {0}; // 用于存储接收到的数据
// HTTP事件处理器
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", 
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // 将接收到的数据存储到tempdata中
            if (evt->data_len < sizeof(tempdata)) {
                memcpy(tempdata, evt->data, evt->data_len);
                tempdata[evt->data_len] = '\0'; // 确保字符串以'\0'结尾
                ESP_LOGI(TAG, "Received data: %s", tempdata);
            } else {
                ESP_LOGE(TAG, "Received data is too large for tempdata buffer");
            }
            xSemaphoreGive(xSemaphore);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}
esp_ota_handle_t update_handle = 0; // OTA句柄
esp_partition_t *update_partition = NULL; // 更新分区
// 获取当前分区信息

// HTTP事件处理器
esp_err_t _http_event_handler_update(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", 
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            esp_err_t err = esp_ota_write(update_handle, evt->data, evt->data_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to write OTA data: %s", esp_err_to_name(err));
                esp_ota_abort(update_handle);
                return false;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
        default:
            break;
    }
    return ESP_OK;
}

// 执行HTTPS请求（不验证证书）
void https_request_without_verification(void)
{
    // 配置HTTP客户端
    esp_http_client_config_t config = {
        .url = "https://www.xvsenfeng.top/wechatapp/version/", // 测试用的HTTPS API
        .event_handler = _http_event_handler,   // 事件回调
        .skip_cert_common_name_check = true,    // 跳过域名检查
        .cert_pem = NULL,                       // 不提供CA证书
        .disable_auto_redirect = true,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    
    // 执行GET请求
    esp_err_t err = esp_http_client_perform(client);
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                (int)esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
        ESP_LOGI(TAG, "Received data successfully");
        cJSON *json = cJSON_Parse(tempdata);
        if (json == NULL) {
            ESP_LOGE(TAG, "Failed to parse JSON: %s", cJSON_GetErrorPtr());
        } else {
            char update_url_[256];
            // 处理JSON数据
            cJSON *version = cJSON_GetObjectItem(json, "version");
            if (cJSON_IsString(version) && (version->valuestring != NULL)) {
                ESP_LOGI(TAG, "Version: %s", version->valuestring);
            } else {
                ESP_LOGE(TAG, "Version not found or not a string");
            }
            cJSON *update_url = cJSON_GetObjectItem(json, "data");
            if (cJSON_IsString(update_url) && (update_url->valuestring != NULL)) {
                ESP_LOGI(TAG, "Update URL: %s", update_url->valuestring);
                sprintf(update_url_, "https://www.xvsenfeng.top%s", update_url->valuestring);
            } else {
                ESP_LOGE(TAG, "Update URL not found or not a string");
            }
            cJSON_Delete(json); // 释放JSON对象

            // 清理资源
            esp_http_client_cleanup(client);

            update_partition = esp_ota_get_next_update_partition(NULL);
            if (update_partition == NULL) {
                ESP_LOGE(TAG, "No partition found for OTA update");
                return;
            }

            if (esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle)) {
                esp_ota_abort(update_handle);
                ESP_LOGE(TAG, "Failed to begin OTA");
                return;
            }
            ESP_LOGI(TAG, "Writing to partition %s at offset 0x%lx", update_partition->label, update_partition->address);

            // 配置HTTP客户端
            esp_http_client_config_t config = {
                .url = update_url_, // 测试用的HTTPS API
                .event_handler = _http_event_handler_update,   // 事件回调
                .skip_cert_common_name_check = true,    // 跳过域名检查
                .cert_pem = NULL,                       // 不提供CA证书
                .disable_auto_redirect = true,
            };

            esp_http_client_handle_t client = esp_http_client_init(&config);
            
            // 执行GET请求
            esp_err_t err = esp_http_client_perform(client);

            if (err == ESP_OK) {
                ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                        esp_http_client_get_status_code(client),
                        (int)esp_http_client_get_content_length(client));
            } else {
                ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
            }

            esp_http_client_cleanup(client);

            err = esp_ota_end(update_handle);
            if (err != ESP_OK) {
                if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                    ESP_LOGE(TAG, "Image validation failed, image is corrupted");
                } else {
                    ESP_LOGE(TAG, "Failed to end OTA: %s", esp_err_to_name(err));
                }
                return;
            }
        
            err = esp_ota_set_boot_partition(update_partition);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to set boot partition: %s", esp_err_to_name(err));
                return;
            }
        
            ESP_LOGI(TAG, "Firmware upgrade successful");
            ESP_LOGI(TAG, "Rebooting to apply the update...");
            esp_restart(); // 重启设备以应用更新
        }
    } else {
        ESP_LOGE(TAG, "Failed to receive data");
        // 清理资源
        esp_http_client_cleanup(client);
    }

    



}

void app_main(void)
{
    xSemaphore = xSemaphoreCreateBinary();
    // 初始化NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 初始化网络
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // 连接WiFi（使用示例配置）
    ESP_ERROR_CHECK(example_connect());

    // 更可靠的等待WiFi连接方式
    ESP_LOGI(TAG, "等待WiFi连接...");
    while (1) {
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            ESP_LOGI(TAG, "已连接到AP: %s", ap_info.ssid);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // 执行HTTPS请求（不验证证书）
    https_request_without_verification();
}