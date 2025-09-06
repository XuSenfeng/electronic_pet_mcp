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
#include <sys/time.h>

static const char *TAG = "HTTPS_NO_VERIFY";
SemaphoreHandle_t xSemaphore;
char tempdata[1024] = {0}; // 用于存储接收到的数据

// 添加时间戳和速度计算相关变量
static uint64_t last_display_time = 0;
static uint32_t total_bytes_received = 0;
static uint32_t last_total_bytes = 0;
static uint64_t start_time = 0;

// 获取当前时间戳（毫秒）
uint64_t get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// HTTP事件处理器
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            // 重置统计变量
            start_time = get_current_time_ms();
            last_display_time = start_time;
            total_bytes_received = 0;
            last_total_bytes = 0;
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", 
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            // 更新接收字节数
            total_bytes_received += evt->data_len;
            
            // 检查是否已经过了一秒
            uint64_t current_time = get_current_time_ms();
            if (current_time - last_display_time >= 1000) {
                // 计算速度（字节/秒）
                uint32_t bytes_this_second = total_bytes_received - last_total_bytes;
                float speed_kbps = (bytes_this_second * 8.0) / 1024.0; // 转换为Kbps
                
                ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d, 总接收=%dKB字节, 本秒速度=%.2f Kbps", 
                         evt->data_len, (int)total_bytes_received / 1024, speed_kbps);
                
                // 更新显示时间和字节数
                last_display_time = current_time;
                last_total_bytes = total_bytes_received;
            }
            
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
            // 显示最终统计信息
            uint64_t total_time = get_current_time_ms() - start_time;
            float avg_speed = (total_bytes_received * 8.0) / (total_time / 1000.0) / 1024.0; // 平均速度 Kbps
            ESP_LOGI(TAG, "传输完成 - 总字节数: %d, 总时间: %d ms, 平均速度: %.2f Kbps", 
                     (int)total_bytes_received, (int)total_time, avg_speed);
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

// 添加OTA更新的统计变量
static uint64_t last_display_time_update = 0;
static uint32_t total_bytes_received_update = 0;
static uint32_t last_total_bytes_update = 0;
static uint64_t start_time_update = 0;

// HTTP事件处理器（用于OTA更新）
esp_err_t _http_event_handler_update(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            // 重置统计变量
            start_time_update = get_current_time_ms();
            last_display_time_update = start_time_update;
            total_bytes_received_update = 0;
            last_total_bytes_update = 0;
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", 
                     evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            // 更新接收字节数
            total_bytes_received_update += evt->data_len;
            
            // 检查是否已经过了一秒
            uint64_t current_time = get_current_time_ms();
            if (current_time - last_display_time_update >= 1000) {
                // 计算速度（字节/秒）
                uint32_t bytes_this_second = total_bytes_received_update - last_total_bytes_update;
                float speed_kbps = (bytes_this_second * 8.0) / 1024.0; // 转换为Kbps
                
                ESP_LOGI(TAG, "OTA下载 - HTTP_EVENT_ON_DATA, len=%d, 总接收=%dKB字节, 本秒速度=%.2f Kbps", 
                         evt->data_len, (int)total_bytes_received_update / 1024, speed_kbps);
                
                // 更新显示时间和字节数
                last_display_time_update = current_time;
                last_total_bytes_update = total_bytes_received_update;
            }
            
            esp_err_t err = esp_ota_write(update_handle, evt->data, evt->data_len);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to write OTA data: %s", esp_err_to_name(err));
                esp_ota_abort(update_handle);
                return false;
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            // 显示最终统计信息
            uint64_t total_time = get_current_time_ms() - start_time_update;
            float avg_speed = (total_bytes_received_update * 8.0) / (total_time / 1000.0) / 1024.0; // 平均速度 Kbps
            ESP_LOGI(TAG, "OTA下载完成 - 总字节数: %d, 总时间: %d ms, 平均速度: %.2f Kbps", 
                     (int)total_bytes_received_update, (int)total_time, avg_speed);
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
            // 解析JSON数据，格式为 {"code":200,"msg":"success","data":{"lichuang":{...},"XvsenfengAI":{...}}}
            cJSON *data = cJSON_GetObjectItem(json, "data");
            if (data == NULL) {
                ESP_LOGE(TAG, "未找到data字段");
                cJSON_Delete(json);
                return;
            }

#if defined(CONFIG_BOARD_TYPE_LICHUANG_DEV)
            cJSON *board_obj = cJSON_GetObjectItem(data, "lichuang");
#elif defined(CONFIG_BOARD_TYPE_XVSENFENGAI)
            cJSON *board_obj = cJSON_GetObjectItem(data, "XvsenfengAI");
#else
            cJSON *board_obj = NULL;
#endif

            if (board_obj == NULL) {
                ESP_LOGE(TAG, "未找到对应开发板的字段");
                cJSON_Delete(json);
                return;
            }

            cJSON *version = cJSON_GetObjectItem(board_obj, "version");
            if (cJSON_IsString(version) && (version->valuestring != NULL)) {
                ESP_LOGI(TAG, "Version: %s", version->valuestring);
            } else {
                ESP_LOGE(TAG, "未找到version字段或不是字符串");
            }

            cJSON *file = cJSON_GetObjectItem(board_obj, "file");
            if (cJSON_IsString(file) && (file->valuestring != NULL)) {
                ESP_LOGI(TAG, "Update file: %s", file->valuestring);
                snprintf(update_url_, sizeof(update_url_), "https://www.xvsenfeng.top%s", file->valuestring);
            } else {
                ESP_LOGE(TAG, "未找到file字段或不是字符串");
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