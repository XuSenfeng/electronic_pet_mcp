/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */

#include "electronic_food.h"
#include "electronic_pet.h"
#include "application.h"
#include "esp_log.h"    

void Food::Use() {
    // 使用食物的逻辑
    ESP_LOGI("Food", "使用食物: %s, 饱食度: %d, 精神状态: %d, 金钱: %d", name_.c_str(), satiety_, vigor_, money_);
    ElectronicPet* pet = ElectronicPet::GetInstance();
    int state[E_PET_STATE_NUMBER] = {0};
    state[E_PET_STATE_SATITY] = satiety_; // 饱食度
    state[E_PET_STATE_VIGIR] = vigor_; // 精神状态
    state[E_PET_STATE_HAPPINESS] = happiness_; // 快乐度
    state[E_PET_STATE_MONEY] = money_; // 金钱
    pet->change_statue(state); // 改变宠物的状态
    // 发送消息给小智
    std::string message = "使用了" + name_ + ", 效果是" + thing_description_ +"，饱食度增加" + std::to_string(satiety_) + "，精神状态增加" + std::to_string(vigor_);
    // 发送消息给小智的逻辑
    auto& app = Application::GetInstance();
    app.SendMessage(message);
}

