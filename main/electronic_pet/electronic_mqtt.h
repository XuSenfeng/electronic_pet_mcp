/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once
#include <string>
#include <esp_mqtt.h>
#include <cJSON.h>
#include <vector>

class PMQTT_Clinet{
public:
    PMQTT_Clinet();
    ~PMQTT_Clinet();

    bool PUublish_Message(std::string type, std::string payload);
private:
    Mqtt* mqtt_;
    std::string client_id_;
    std::string publish_topic_;
};