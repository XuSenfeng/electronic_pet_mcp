/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "electronic_pet.h"
#include "esp_log.h"
#include "application.h"
#include "settings.h"
#include "string.h"
#include "electronic_food.h"
#include "board.h"
#include "display/display.h"
#define TAG "ElectronicPet"

ElectronicPet::ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet constructor");
    client_ = new PMQTT_Clinet();
    Settings settings("e_pet", true);
    state_[E_PET_STATE_SATITY].value = settings.GetInt("state_" + std::to_string(E_PET_STATE_SATITY), 100);
    state_[E_PET_STATE_HAPPINESS].value = settings.GetInt("state_" + std::to_string(E_PET_STATE_HAPPINESS), 100);
    state_[E_PET_STATE_VIGIR].value = settings.GetInt("state_" + std::to_string(E_PET_STATE_VIGIR), 100);
    state_[E_PET_STATE_IQ].value = settings.GetInt("state_" + std::to_string(E_PET_STATE_IQ), 20);
    state_[E_PET_STATE_MONEY].value = settings.GetInt("state_" + std::to_string(E_PET_STATE_MONEY), 0);

    level_ = settings.GetInt("level", 1);
    experience_ = settings.GetInt("experience", 1);

    SetStateName(0, "精力");
    SetStateName(1, "饱食度");
    SetStateName(2, "快乐度");
    SetStateName(3, "智商");
    SetStateName(4, "金钱");

    printf("state_0 %s: %d, state_1 %s: %d, state_2 %s: %d\n", GetStateName(0), GetState(0), GetStateName(1), GetState(1), GetStateName(2), GetState(2));
    action_ = (electronic_pet_action_e)settings.GetInt("action", E_PET_ACTION_IDLE);
    ReadCsvThings();
    client_->PUublish_Message("log", "ElectronicPet initialized");
    
}

ElectronicPet::~ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet destructor");
}


ElectronicPet* ElectronicPet::GetInstance() {
    return Application::GetInstance().GetMyPet();
}


void ElectronicPet::change_statue(int *change_state){
    std::lock_guard<std::mutex> lock(mutex_);
    for(int i = 0; i < E_PET_STATE_DIVIDING_LINE; i++){
        state_[i].value += change_state[i];
        if(state_[i].value > 100) state_[i].value = 100;
        if(state_[i].value < 0) state_[i].value = 0;
    }
    for(int i = E_PET_STATE_DIVIDING_LINE; i < E_PET_STATE_NUMBER; i++){
        state_[i].value += change_state[i];
        if(state_[i].value < 0) state_[i].value = 0;
    }
    Settings settings("e_pet", true);
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        settings.SetInt("state_" + std::to_string(i), state_[i].value);
    }
}

void ElectronicPet::SetState(int state, int value){
    std::lock_guard<std::mutex> lock(mutex_);
    if(state >= E_PET_STATE_NUMBER){
        ESP_LOGE(TAG, "Invalid state number: %d", state);
        return;
    }
    state_[state].value = value;
    Settings settings("e_pet", true);
    settings.SetInt("state_" + std::to_string(state), value);
}

std::string ElectronicPet::GetStateDescriptor(){
    // 遍历state_数组，合成描述符串
    std::string descriptor = "状态: ";
    for(int i = 0; i < E_PET_STATE_NUMBER; i++){
        descriptor += state_[i].name;
        descriptor += ": ";
        descriptor += std::to_string(state_[i].value);
        if(i != E_PET_STATE_NUMBER - 1){
            descriptor += ", ";
        }
    }
    return descriptor;
}


void ElectronicPet::SetAction(int action){
    std::lock_guard<std::mutex> lock(mutex_);
    if(action >= E_PET_ACTION_NUMBER){
        ESP_LOGE(TAG, "Invalid action number: %d", action);
        return;
    }
    action_last_ = action_;
    action_ = (electronic_pet_action_e)action;
    Settings settings("e_pet", true);
    settings.SetInt("action", action_);
    ESP_LOGI(TAG, "Set action: %d", action_);
}

void ElectronicPet::ReturnLastAction(){
    std::lock_guard<std::mutex> lock(mutex_);
    electronic_pet_action_e temp_action_ = action_;
    action_ = action_last_;
    action_last_ = temp_action_;

    Settings settings("e_pet", true);
    settings.SetInt("action", action_);
    ESP_LOGI(TAG, "Return last action: %d", action_);
}

void ElectronicPet::ReadCsvFood(int *i){
    const char *file_hello = "/sdcard/electronic_pet_food.csv";
    ESP_LOGI(TAG, "Reading file: %s", file_hello);
    // 打开文件
    FILE *f = fopen(file_hello, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[256];
    
    while (fgets(line, sizeof(line), f)) {
        char name[30];
        char description[100];
        int vigor_, satiety_, happiness_, money_, iq_, level;
        int ret = sscanf(line, "%[^,],%d,%d,%d,%d,%d,%d,%[^\n]", name, &vigor_, &satiety_, &happiness_, &money_, &iq_, &level, description);
        if (ret == 8) {
 
            Settings settings("e_pet", true);
            int num = settings.GetInt("things_num_" + std::to_string(*i), 0);

            ESP_LOGI(TAG, "Parsed line: %s, thing %d %s: %s", line, *i, name, description);
            ESP_LOGI(TAG, "Parsed data: num %d, name=%s, vigor=%d, satiety=%d, happiness=%d, money=%d, iq=%d, level=%d, description=%s", num, name, vigor_, satiety_, happiness_, money_, iq_, level, description);
                
            int state[E_PET_STATE_NUMBER] = {0};
            state[E_PET_STATE_SATITY] = satiety_;
            state[E_PET_STATE_MONEY] = money_;
            state[E_PET_STATE_VIGIR] = vigor_;
            state[E_PET_STATE_HAPPINESS] = happiness_;
            state[E_PET_STATE_IQ] = iq_;
            Food *food = new Food(name, {0}, description, state, num, level);

            things_.push_back(food);
            *i += 1;
        } else {
            ESP_LOGW(TAG, "Failed to parse line: %s", line);
        }
    }
}

void ElectronicPet::ReadCsvGames(){
    const char *file_hello = "/sdcard/electronic_pet_games.csv";
    ESP_LOGI(TAG, "Reading file: %s", file_hello);
    // 打开文件
    FILE *f = fopen(file_hello, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[256];
    char *message = (char *)malloc(4096);
    while (fgets(line, sizeof(line), f)) {
        char name[30];
        char description[100];
        
        int ret = sscanf(line, "%[^,],%[^,],%[^,]", name, description, message);

        if (ret == 3) {
            ESP_LOGI(TAG, "Parsed line: %s", line);
            ESP_LOGI(TAG, "Parsed data: name=%s, description=%s, message %s", name, description, message);
            GameInfo game;
            game.name = name;
            game.desc = description;
            game.message = message;
            games_.push_back(game);
        } else {
            ESP_LOGW(TAG, "Failed to parse line: %s", line);
        }
    }
    free(message);
    fclose(f);
}

void ElectronicPet::ReadCsvThings(){
    //---------------------
    // food
    //---------------------
    int i = 0;
    ReadCsvFood(&i);
    ReadCsvGames();
}

//electronic_pet_upgrade_task.csv
std::string ElectronicPet::ReadUpgradeTaskCsv(int level){
    const char *file_hello = "/sdcard/electronic_pet_upgrade_task.csv";
    ESP_LOGI(TAG, "Reading file: %s", file_hello);
    // 打开文件
    FILE *f = fopen(file_hello, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return "";
    }
    char line[256];
    // 读取第level行
    int current_level = 0;
    std::string description_str;
    while (fgets(line, sizeof(line), f)) {
        current_level++;
        if (current_level == level) {
            int temp;
            char description[256];
            int ret = sscanf(line, "%d,%[^\n]", &temp, description);
            description_str = description;
            if (ret == 2) {
                ESP_LOGI(TAG, "Parsed line: %s", line);
                ESP_LOGI(TAG, "Parsed data: level=%d, description=%s", temp, description_str.c_str());
                // 成功解析，返回描述字符串
            } else {
                ESP_LOGW(TAG, "Failed to parse line: %s", line);
            }
            break;
        }
    }
    fclose(f);
    return description_str;
}


void ElectronicPet::StateEventDeal(){
    int good_state = 0;
    for(int i = 0; i < E_PET_STATE_GOOD_LINE; i++){
        if(state_[i].value > 75){
            good_state++;
        }
    }
    if(good_state == E_PET_STATE_GOOD_LINE){
        // 触发好心情事件
        experience_ += 1;
    }

    if(GetState(E_PET_STATE_SATITY) < 30){
        // 触发饥饿事件
        ESP_LOGI(TAG, "Hunger event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[你当前状态: 喵喵肚肚, 饿了要吃东西]";
        app.SendMessage(message);
    }else if(GetState(E_PET_STATE_HAPPINESS) < 30 && 
            !(GetAction() == E_PET_ACTION_PLAY || GetAction() == E_PET_ACTION_PLAY_MUSIC ||
            GetAction() == E_PET_ACTION_WALK || GetAction() == E_PET_ACTION_BATH ||
            GetAction() == E_PET_ACTION_SLEEP)){
        // 触发快乐事件
        ESP_LOGI(TAG, "Happiness event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[你当前状态: 喵喵不开心, 希望主人陪我玩耍]";
        app.SendMessage(message);
    }else if(GetState(E_PET_STATE_VIGIR) == 0 && GetAction() != E_PET_ACTION_SLEEP){
        // 触发精力事件
        ESP_LOGI(TAG, "Vigour event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[你当前状态: 喵喵累晕倒了...]";
        app.SendMessage(message);
        SetAction(E_PET_ACTION_SLEEP);
    }else if(GetState(E_PET_STATE_VIGIR) < 30 && !(GetAction() == E_PET_ACTION_SLEEP)){
        // 触发精力事件
        ESP_LOGI(TAG, "Vigour event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[你当前状态: 喵喵要累死了, 呜呜呜]";
        app.SendMessage(message);
    }
}

std::string ElectronicPet::GetUpdateTask(void){
    ESP_LOGI(TAG, "Upgrade task");
    // 升级
    if(isUpGraded()){
        // 获取升级任务描述
        std::string description = ReadUpgradeTaskCsv(level_);
        if(description.empty()){
            description = "暂时没有升级任务描述, 你可以随机给出一个问题考考用户";
            return description;
        }
        return description;
    }
    ESP_LOGI(TAG, "No upgrade task available");
    // 没有升级任务
    std::string description = "当前的状态不可以升级";
    return description;
}
/// @brief 升级函数
/// @details 升级函数会检查是否可以升级，如果可以升级，则升级等级，并更新经验值
bool ElectronicPet::Upgrade(void){
    ESP_LOGI(TAG, "Upgrade task");
    // 升级
    if(isUpGraded()){
        level_++;
        experience_ -= level_ * level_;
        if(experience_ < 0){
            experience_ = 0;
        }
        Settings settings("e_pet", true);
        settings.SetInt("level", level_);
        settings.SetInt("experience", experience_);
        ESP_LOGI(TAG, "Upgrade success, new level: %d", level_);
        int state[E_PET_STATE_NUMBER] = {0};
        state[E_PET_STATE_SATITY] = 10;
        state[E_PET_STATE_MONEY] = 30;
        state[E_PET_STATE_VIGIR] = 20;
        state[E_PET_STATE_HAPPINESS] = 30;
        state[E_PET_STATE_IQ] = 3;
        change_statue(state);
        auto display = Board::GetInstance().GetDisplay();
        if(display == nullptr) {
            ESP_LOGE(TAG, "Display instance is null");
            return false;
        }
        display->UpdataUILevel(level_);
        return true;
    }


    return false; // 升级失败
}