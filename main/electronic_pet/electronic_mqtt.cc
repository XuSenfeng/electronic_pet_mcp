/*
* @Descripttion: 
* @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
* Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
*/
#include "electronic_mqtt.h"
#include "esp_log.h"
#include "settings.h"
#define TAG "PMQTT_Client"
#include "esp_mac.h"
#include "iot/thing_manager.h"
#include "application.h"


PMQTT_Clinet::PMQTT_Clinet() : mqtt_(nullptr) {
    Settings settings("mqtt", false);
    client_id_ = settings.GetString("client_id");
    if (mqtt_ != nullptr) {
        ESP_LOGW(TAG, "Mqtt client already started");
        delete mqtt_;
    }

    mqtt_ = new EspMqtt();
    mqtt_->SetKeepAlive(90);

    mqtt_->OnDisconnected([this]() {
        ESP_LOGI(TAG, "Disconnected from endpoint");
    });

    mqtt_->OnMessage([this](const std::string& topic, const std::string& payload) {
        ESP_LOGI(TAG, "Received message on topic %s: %s", topic.c_str(), payload.c_str());
        // 解析payload，假设它是一个 JSON 字符串
        cJSON* root = cJSON_Parse(payload.c_str());
        if (root == nullptr) {
            ESP_LOGE(TAG, "Failed to parse JSON payload: %s", payload.c_str());
            return;
        }
        
        // 获取设备类型
        cJSON* type = cJSON_GetObjectItem(root, "type");
        if (type == nullptr || !cJSON_IsNumber(type)) {
            ESP_LOGE(TAG, "Device type is not specified or invalid");
            cJSON_Delete(root);
            return;
        }
        int type_value = type->valueint;
        ESP_LOGI(TAG, "Device type value: %d", type_value);
        // 根据设备类型创建 Thing
        if(type_value == 1){
        
        }


        
        cJSON_Delete(root);
    });

    if (!mqtt_->Connect(CONFIG_PET_MQTT_SERVER, 1883, client_id_.c_str(), "", "")) {
        ESP_LOGE(TAG, "Failed to connect to endpoint");
        return ;
    }
    publish_topic_ = CONFIG_PET_MQTT_TOPIC;
    std::string real_address_str(publish_topic_);
    real_address_str += "/+";
    mqtt_->Subscribe(real_address_str, 2);

}

bool PMQTT_Clinet::PUublish_Message(std::string type, std::string payload) {
    if (mqtt_ == nullptr || !mqtt_->IsConnected()) {
        ESP_LOGE(TAG, "MQTT client is not connected");
        return false;
    }
    std::string topic = publish_topic_ + "/" + type;
    ESP_LOGI(TAG, "Publishing message to topic %s: %s", topic.c_str(), payload.c_str());
    return mqtt_->Publish(topic, payload, 2);
}

