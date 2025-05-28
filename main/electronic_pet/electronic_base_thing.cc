/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "electronic_base_thing.h"
#include "settings.h"
#include "esp_log.h"

#define TAG "BaseThing"

void BaseThing::SetNum(int num){
    std::lock_guard<std::mutex> lock(mutex_);
    num_ = num;
    Settings settings("e_pet", true);
    settings.SetInt("things_num_" + std::to_string(num_), num_);
    ESP_LOGI(TAG, "Set num: %d", num_);
}