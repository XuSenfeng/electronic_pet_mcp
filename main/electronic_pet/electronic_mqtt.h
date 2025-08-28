/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once
#include <string>
#include <esp_mqtt.h>
#include <cJSON.h>
#include <vector>

class PMQTT_Clinet{
public:
    PMQTT_Clinet(std::string boardID = "");
    ~PMQTT_Clinet();
    bool Message_Deal_Phone(cJSON* root, const std::string& payload);
    bool Message_Deal_Follow(cJSON* root, const std::string& payload);
    bool Publish_Message(std::string type, std::string payload);
private:
    Mqtt* mqtt_;
    std::string client_id_;
    std::string publish_topic_;
};

#define TOPIC_MESSAGE "electronic/boards/"