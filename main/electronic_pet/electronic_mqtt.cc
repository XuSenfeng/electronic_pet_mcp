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
/*

消息通知: 
1. 用于用户之间的交流, 需要直接进行播放提醒




*/

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
    // 处理服务器的消息回调函数
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
            Message_Deal(root, payload);
        }


        
        cJSON_Delete(root);
    });

    if (!mqtt_->Connect(CONFIG_PET_MQTT_SERVER, 1883, client_id_.c_str(), "", "")) {
        ESP_LOGE(TAG, "Failed to connect to endpoint");
        return ;
    }
    // 发布的地址 /qin/
    publish_topic_ = CONFIG_PET_MQTT_TOPIC;
    // 测试的订阅 jiao/+
    std::string real_address_str(CONFIG_PET_MQTT_LOCAL_TOPIC);
    real_address_str += "/+";
    // 开始订阅
    mqtt_->Subscribe(real_address_str, 2);

}
// 处理远程的对话消息, 实现播报通知
bool PMQTT_Clinet::Message_Deal(cJSON* root, const std::string& payload) {
    /*
    {
        "type": 1,
        "msg": "远端消息"
    }
     */
    cJSON* message = cJSON_GetObjectItem(root, "msg");
    if (message == nullptr || !cJSON_IsString(message)) {
        ESP_LOGE(TAG, "Message is not specified or invalid");
        return false;
    }
    std::string message_str = message->valuestring;
    ESP_LOGI(TAG, "Received message: %s", message_str.c_str());
    // 发送提示信息
    auto& app = Application::GetInstance();
    // 让小智直接使用复读模式
    message_str = "你现在是一个复读机器,请直接念出来冒号之后的句子,不要增加任何的信息: 获取到消息通知," +  message_str;
    app.SendMessage(message_str);
    return true;
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

