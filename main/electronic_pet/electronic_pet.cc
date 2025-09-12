/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
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
#include <esp_mac.h>
#include <esp_http_client.h>
#include <esp_crt_bundle.h>
#include <esp_tls.h>
// #include <base64.h>
#define TAG "ElectronicPet"
// 精力, 饱食度, 快乐度
int ElectronicPet::state_time_change_[E_PET_ACTION_NUMBER][E_PET_STATE_NUMBER] = {
    /* E_PET_ACTION_IDLE 空闲*/      {-1, -1, -1, 0, 0},
    /* E_PET_ACTION_PLAY 玩耍*/       {-3, -3, 5, 0, 0},
    /* E_PET_ACTION_SLEEP 睡觉*/      {2, -1, 0, 0, 0},
    /* E_PET_ACTION_WALK 走路*/       {-2, -2, 1, 0, 0},
    /* E_PET_ACTION_BATH 洗澡*/       {-1, -1, 1, 0, 0},
    /* E_PET_ACTION_WORK 工作*/       {-4, -2, -4, 0, 1},
    /* E_PET_ACTION_STUDY 学习*/      {-4, -2, -3, 1, 0},
    /* E_PET_ACTION_PLAY_MUSIC 听歌*/ {-1, -1, 2, 0, 0}
};

std::string ElectronicPet::action_name_[E_PET_ACTION_NUMBER] = {
    "空闲",
    "玩耍",
    "睡觉",
    "走路",
    "洗澡",
    "工作",
    "学习",
    "听歌"
};

#define WIFI_BUFFER_LENGTH          (1 * 1024)

#define MAX_HTTP_OUTPUT_BUFFER WIFI_BUFFER_LENGTH
char wifi_read_buf[WIFI_BUFFER_LENGTH];
int read_pos = 0, cannot_read = 0;

int buf_type = 0;

//事件回调
static esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr){
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return ESP_FAIL;
    }
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR://错误事件
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED://连接成功事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT://发送头事件
            ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER://接收头事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA://接收数据事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if(read_pos + evt->data_len > WIFI_BUFFER_LENGTH){
                ESP_LOGE(TAG, "buffer overflow");
                cannot_read = 1;
                break;
            }
            memcpy(wifi_read_buf + read_pos, evt->data, evt->data_len);
            ESP_LOGI(TAG, "wifi_read_buf: %s", wifi_read_buf);
            read_pos += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH://会话完成事件
            ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
            xEventGroupSetBits(pet->message_send_event_, HTTP_EVENT);
            break;
        case HTTP_EVENT_REDIRECT://重定向事件
            ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
            break;
        case HTTP_EVENT_DISCONNECTED://断开事件
            {
                ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
                int mbedtls_err = 0;
                esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
                if (err != 0) {
                    ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                    ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
                }
            }
            break;
        default:
            break;
        
    }
    return ESP_OK;
}


ElectronicPet::ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet constructor");
    message_send_event_ = xEventGroupCreate();
    if(message_send_event_ == nullptr){
        ESP_LOGE(TAG, "Failed to create message_send_event");
    }

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

    boardID = GetBoardID();
    ESP_LOGI(TAG, "boardID: %s", boardID.c_str());
    auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    std::string url = CONFIG_SERVER_BASE_SERVER_URL "/pets/pet/?boardID=" + boardID;
    if (!http->Open("GET", url)) {
        ESP_LOGE(TAG, "Failed to open HTTP connection for version check");
    }
    std::string data = http->ReadAll();
    
    cJSON *root = cJSON_Parse(data.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON response: %s", data.c_str());
        return;
    }
    cJSON *code = cJSON_GetObjectItem(root, "code");
    if (cJSON_IsNumber(code)) {
        if( code->valueint ==200) {
            use_web_server_ = true;
            ESP_LOGI(TAG, "Web server is enabled");
        }
    }
    cJSON_Delete(root);
    http->Close();
    if(use_web_server_){
        ESP_LOGI(TAG, "Using web server for things");
        ReadWebThings();
    } else {
        ReadCsv();
    }
    UploadState();

    client_ = new PMQTT_Clinet(boardID);
    client_->Publish_Message("log", "ElectronicPet initialized");
    timer = new ElectronicPetTimer(use_web_server_, boardID.c_str());
}

ElectronicPet::~ElectronicPet(){
    ESP_LOGI(TAG, "ElectronicPet destructor");
}

void ElectronicPet::ReadCsv(){
    //---------------------
    // food
    //---------------------
    int i = 0;
    ReadCsvFood(&i);
    ReadCsvGames();
}
std::string ElectronicPet::GetFocusListJson(void){
    std::string result = "{\"success\": true, \"data\": [";
    bool first = true;
    for(int i = 0; i < focus_list_.size(); i++){
        if(focus_list_[i].mutual_focus){
            if(!first) result += ",";
            result += "{\"name\": \"" + focus_list_[i].name + "\", \"boardID\": \"" + focus_list_[i].boardID + "\", \"mutual_focus\": true}";
            first = false;
        }
    }
    result += "]}";
    return result;
}
void ElectronicPet::ReadWebThings(void){
    int thing_num = 0;
    ReadWebFood(&thing_num);
    ReadWebGames();
    ReadWebFocus();
}


/**
 * @brief 解码Unicode转义序列
 * @param input 包含Unicode转义序列的字符串
 * @return 解码后的字符串
 */
std::string DecodeUnicode(const std::string& input) {
    std::string result;
    result.reserve(input.length());
    
    for (size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '\\' && i + 1 < input.length() && input[i + 1] == 'u' && i + 5 < input.length()) {
            // 找到 \uXXXX 格式
            std::string hex_str = input.substr(i + 2, 4);
            unsigned int unicode_val;
            
            // 将十六进制字符串转换为整数
            if (sscanf(hex_str.c_str(), "%x", &unicode_val) == 1) {
                // 对于UTF-8编码，需要将Unicode码点转换为UTF-8字节序列
                if (unicode_val <= 0x7F) {
                    // 单字节UTF-8
                    result += static_cast<char>(unicode_val);
                } else if (unicode_val <= 0x7FF) {
                    // 双字节UTF-8
                    result += static_cast<char>(0xC0 | (unicode_val >> 6));
                    result += static_cast<char>(0x80 | (unicode_val & 0x3F));
                } else if (unicode_val <= 0xFFFF) {
                    // 三字节UTF-8
                    result += static_cast<char>(0xE0 | (unicode_val >> 12));
                    result += static_cast<char>(0x80 | ((unicode_val >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (unicode_val & 0x3F));
                }
                i += 5; // 跳过已处理的 \uXXXX
            } else {
                // 解析失败，保留原始字符
                result += input[i];
            }
        } else {
            result += input[i];
        }
    }
    
    return result;
}

void ElectronicPet::ReadWebFood(int *thing_num){
    // 使用GET请求获取商店列表信息
    auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    std::string url = CONFIG_SERVER_BASE_SERVER_URL "/shops/shoplists/?boardID=" + boardID;
    if (!http->Open("GET", url)) {
        ESP_LOGE(TAG, "无法打开HTTP连接获取商店列表");
        return;
    }
    // 使用cJSON解析返回的物品列表
    std::string data = http->ReadAll();
    // ESP_LOGI(TAG, "商店列表返回数据: %s", data.c_str());
    cJSON *root = cJSON_Parse(data.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "解析商店列表JSON失败: %s", data.c_str());
        http->Close();
        return;
    }
    if (!cJSON_IsArray(root)) {
        ESP_LOGE(TAG, "商店列表JSON不是数组格式");
        cJSON_Delete(root);
        http->Close();
        return;
    }
    int item_count = cJSON_GetArraySize(root);
    for (int i = 0; i < item_count; ++i) {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsObject(item)) continue;
        cJSON *id = cJSON_GetObjectItem(item, "id");
        cJSON *name = cJSON_GetObjectItem(item, "name");
        cJSON *money = cJSON_GetObjectItem(item, "money");
        cJSON *description = cJSON_GetObjectItem(item, "description");
        cJSON *num = cJSON_GetObjectItem(item, "num");
        cJSON *vigor = cJSON_GetObjectItem(item, "vigor");
        cJSON *satiety = cJSON_GetObjectItem(item, "satiety");
        cJSON *happiness = cJSON_GetObjectItem(item, "happiness");
        cJSON *iq = cJSON_GetObjectItem(item, "iq");
        cJSON *level = cJSON_GetObjectItem(item, "level");
        
        // 解码Unicode转义序列
        std::string decoded_name = "";
        std::string decoded_description = "";
        
        if (name && name->valuestring) {
            decoded_name = DecodeUnicode(name->valuestring);
        }
        if (description && description->valuestring) {
            decoded_description = DecodeUnicode(description->valuestring);
        }
        
        // 这里可以根据需要将物品添加到things_等容器中
        ESP_LOGI(TAG, "物品: id=%d, name=%s, money=%d, description=%s, num=%d, vigor=%d, satiety=%d, happiness=%d, iq=%d, level=%d",
            id ? id->valueint : -1,
            decoded_name.c_str(),
            money ? money->valueint : 0,
            decoded_description.c_str(),
            num ? num->valueint : 0,
            vigor ? vigor->valueint : 0,
            satiety ? satiety->valueint : 0,
            happiness ? happiness->valueint : 0,
            iq ? iq->valueint : 0,
            level ? level->valueint : 0
        );
        
        CreatOneThing((char*)decoded_name.c_str(), (char*)decoded_description.c_str(), 
            vigor ? vigor->valueint : 0, 
            satiety ? satiety->valueint : 0, 
            happiness ? happiness->valueint : 0, 
            money ? money->valueint : 0, 
            iq ? iq->valueint : 0, 
            level ? level->valueint : 0, 
            thing_num);
    }
    cJSON_Delete(root);
    http->Close();
}

void ElectronicPet::ReadWebFocus(void){
    focus_list_.clear();
    // 使用GET请求获取关注列表信息
    auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    std::string url = CONFIG_SERVER_BASE_SERVER_URL "/pets/focus/?boardID=" + boardID;
    if (!http->Open("GET", url)) {
        ESP_LOGE(TAG, "无法打开HTTP连接获取关注列表");
        return;
    }
    // 使用cJSON解析返回的物品列表
    std::string json_str = http->ReadAll();
    // ESP_LOGI(TAG, "关注列表返回数据: %s", data.c_str());
    cJSON *root = cJSON_Parse(json_str.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "解析关注列表JSON失败: %s", json_str.c_str());
        http->Close();
        return;
    }
    cJSON *data = cJSON_GetObjectItem(root, "data");
    if (!cJSON_IsArray(data)) {
        ESP_LOGE(TAG, "关注列表JSON不是数组格式");
        cJSON_Delete(data);
        http->Close();
        return;
    }
    int item_count = cJSON_GetArraySize(data);
    for (int i = 0; i < item_count; ++i) {
        cJSON *item = cJSON_GetArrayItem(data, i);
        if (!cJSON_IsObject(item)) continue;
        cJSON *name = cJSON_GetObjectItem(item, "name");
        cJSON *boardID_item = cJSON_GetObjectItem(item, "id");
        cJSON *mutual_focus = cJSON_GetObjectItem(item, "is_mutual");
        
        FocueInfo info;
        if (name && name->valuestring) {
            info.name = DecodeUnicode(name->valuestring);
        }
        if (boardID_item && boardID_item->valuestring) {
            info.boardID = boardID_item->valuestring;
        }
        if (mutual_focus && cJSON_IsBool(mutual_focus)) {
            info.mutual_focus = cJSON_IsTrue(mutual_focus);
        } else {
            info.mutual_focus = false;
        }
        
        focus_list_.push_back(info);
        
        ESP_LOGI(TAG, "关注: name=%s, boardID=%s, mutual_focus=%d",
            info.name.c_str(),
            info.boardID.c_str(),
            info.mutual_focus
        );
    }
    cJSON_Delete(root);
    http->Close();
}


std::string ElectronicPet::GetBoardID(void){
    char mac[6];
    esp_read_mac((uint8_t*)mac, ESP_MAC_WIFI_STA);
    char mac_str[13];
    snprintf(mac_str, sizeof(mac_str), "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(mac_str);
    
}


ElectronicPet* ElectronicPet::GetInstance() {
    ElectronicPet* pet = Application::GetInstance().GetMyPet();
    if (pet == nullptr) {
        return nullptr;
    }
    return pet;
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
    settings.SetInt("experience", experience_);
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

bool ElectronicPet::CreatOneThing(char *name, char *description, int vigor_, int satiety_, int happiness_, int money_, int iq_, int level, int *i){
    Settings settings("e_pet", true);
    int num = settings.GetInt("things_num_" + std::to_string(*i), 0);
    int state[E_PET_STATE_NUMBER] = {0};
    state[E_PET_STATE_SATITY] = satiety_;
    state[E_PET_STATE_MONEY] = money_;
    state[E_PET_STATE_VIGIR] = vigor_;
    state[E_PET_STATE_HAPPINESS] = happiness_;
    state[E_PET_STATE_IQ] = iq_;
    Food *food = new Food(name, {0}, description, state, num, level);
    things_.push_back(food);
    *i += 1;
    return true;
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
    char name[30];
    char description[100];
    int vigor_, satiety_, happiness_, money_, iq_, level;
    while (fgets(line, sizeof(line), f)) {

        int ret = sscanf(line, "%[^,],%d,%d,%d,%d,%d,%d,%[^\n]", name, &vigor_, &satiety_, &happiness_, &money_, &iq_, &level, description);
        if (ret == 8) {
            ESP_LOGI(TAG, "Parsed data: name=%s, vigor=%d, satiety=%d, happiness=%d, money=%d, iq=%d, level=%d, description=%s", 
                     name, vigor_, satiety_, happiness_, money_, iq_, level, description);
            CreatOneThing(name, description, vigor_, satiety_, happiness_, money_, iq_, level, i);

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

void ElectronicPet::ReadWebGames(void){
    // 使用GET请求获取游戏列表信息
    auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    std::string url = CONFIG_SERVER_BASE_SERVER_URL "/pets/prompts/?boardID=" + boardID;
    if (!http->Open("GET", url)) {
        ESP_LOGE(TAG, "无法打开HTTP连接获取游戏列表");
        return;
    }
    // 使用cJSON解析返回的物品列表
    std::string data = http->ReadAll();
    // ESP_LOGI(TAG, "游戏列表返回数据: %s", data.c_str());
    cJSON *root = cJSON_Parse(data.c_str());
    if (root == NULL) {
        ESP_LOGE(TAG, "解析游戏列表JSON失败: %s", data.c_str());
        http->Close();
        return;
    }
    cJSON *prompts_data = cJSON_GetObjectItem(root, "data");
    if (!cJSON_IsArray(prompts_data)) {
        ESP_LOGE(TAG, "游戏列表JSON不是数组格式");
        cJSON_Delete(root);
        http->Close();
        return;
    }
    int item_count = cJSON_GetArraySize(prompts_data);
    ESP_LOGI(TAG, "Found %d games in JSON", item_count);
    for (int i = 0; i < item_count; i++) {
        cJSON *item = cJSON_GetArrayItem(prompts_data, i);
        if (!cJSON_IsObject(item)) continue;
        cJSON *title = cJSON_GetObjectItem(item, "title");
        cJSON *description = cJSON_GetObjectItem(item, "description");
        cJSON *prompt = cJSON_GetObjectItem(item, "prompt");
        // 解码Unicode转义序列
        std::string decoded_title = "";
        std::string decoded_prompt = "";
        std::string decoded_description = "";
        if (description && description->valuestring) {
            decoded_description = DecodeUnicode(description->valuestring);
        }
        if (title && title->valuestring) {
            decoded_title = DecodeUnicode(title->valuestring);
        }
        if (prompt && prompt->valuestring) {
            decoded_prompt = DecodeUnicode(prompt->valuestring);
        }
        GameInfo game;
        game.name = decoded_title;
        game.desc = decoded_description;
        game.message = decoded_prompt;
        games_.push_back(game);
        ESP_LOGI(TAG, "游戏: title=%s, description=%s, prompt=%s",
            decoded_title.c_str(),
            decoded_description.c_str(),
            decoded_prompt.c_str()
        );
    }
    
    // 清理资源
    cJSON_Delete(root);
    http->Close();
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
        std::string message = "[系统提示: 你当前状态, 肚子饿了, 需要吃东西]";
        app.SendMessage(message);
    }else if(GetState(E_PET_STATE_HAPPINESS) < 30 && 
            !(GetAction() == E_PET_ACTION_PLAY || GetAction() == E_PET_ACTION_PLAY_MUSIC ||
            GetAction() == E_PET_ACTION_WALK || GetAction() == E_PET_ACTION_BATH ||
            GetAction() == E_PET_ACTION_SLEEP)){
        // 触发快乐事件
        ESP_LOGI(TAG, "Happiness event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[系统提示: 你当前状态, 不开心, 希望主人陪我玩耍]";
        app.SendMessage(message);
    }else if(GetState(E_PET_STATE_VIGIR) == 0 && GetAction() != E_PET_ACTION_SLEEP){
        // 触发精力事件
        ESP_LOGI(TAG, "Vigour event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[系统提示: 你当前状态, 累晕倒了...]";
        app.SendMessage(message);
        SetAction(E_PET_ACTION_SLEEP);
    }else if(GetState(E_PET_STATE_VIGIR) < 30 && !(GetAction() == E_PET_ACTION_SLEEP)){
        // 触发精力事件
        ESP_LOGI(TAG, "Vigour event triggered");
        // 发送消息给小智
        auto &app = Application::GetInstance();
        std::string message = "[系统提示: 你当前状态, 要累死了, 呜呜呜]";
        app.SendMessage(message);
    }
}

std::string ElectronicPet::GetUpdateTask(void){
    ESP_LOGI(TAG, "Upgrade task");
    // 升级
    if(isUpGraded()){
        // // 获取升级任务描述
        // std::string description = ReadUpgradeTaskCsv(level_);
        // if(description.empty()){
        //     description = "暂时没有升级任务描述, 你可以随机给出一个问题考考用户";
        //     return description;
        // }
        memset(wifi_read_buf, 0, sizeof(wifi_read_buf));
        read_pos = 0;
        cannot_read = 0;
        xEventGroupClearBits(message_send_event_, HTTP_EVENT);
        esp_http_client_config_t config = {
            .url = "https://apis.tianapi.com/naowan/index?key="  CONFIG_UPDATE_API_KEY "&num=1",
            .method = HTTP_METHOD_GET,
            .event_handler = _http_event_handle,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };
        ESP_LOGI(TAG, "start get data");
        esp_http_client_handle_t client = esp_http_client_init(&config);//初始化配置
        ESP_LOGD(TAG, "esp_http_client_init");
        esp_http_client_perform(client);//执行请求

        
        xEventGroupWaitBits(message_send_event_, HTTP_EVENT, pdFALSE, pdTRUE, 1000);
        ESP_LOGI(TAG, "HTTP event triggered, wifi_read_buf: %s", wifi_read_buf);
        esp_http_client_cleanup(client);//断开并释放资源


        return wifi_read_buf;
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

void ElectronicPet::UploadState(void){
    ESP_LOGI(TAG, "Upload state");
    // 上传状态
    auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
    std::string url = CONFIG_SERVER_BASE_SERVER_URL "/pets/pet/state/?boardID=" + boardID;
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "boardID", boardID.c_str());
    cJSON_AddNumberToObject(root, "level", level_);
    cJSON_AddNumberToObject(root, "exp", experience_);
    cJSON_AddNumberToObject(root, "satiety", state_[E_PET_STATE_SATITY].value);
    cJSON_AddNumberToObject(root, "happiness", state_[E_PET_STATE_HAPPINESS].value);
    cJSON_AddNumberToObject(root, "vigor", state_[E_PET_STATE_VIGIR].value);
    cJSON_AddNumberToObject(root, "iq", state_[E_PET_STATE_IQ].value);
    cJSON_AddNumberToObject(root, "money", state_[E_PET_STATE_MONEY].value);
    std::string data = cJSON_Print(root);
    ESP_LOGI(TAG, "Upload state data: %s", data.c_str());
    http->SetContent(std::move(data));
    http->SetHeader("Client-Id", Board::GetInstance().GetUuid().c_str());
    http->SetHeader("Content-Type", "application/json");
    if (!http->Open("POST", url)) {
        ESP_LOGE(TAG, "无法打开HTTP连接上传状态");
        return;
    }
    ESP_LOGI(TAG, "Upload state data: %s", data.c_str());
    auto status_code = http->GetStatusCode();
    if (status_code != 200) {
        ESP_LOGE(TAG, "上传状态失败, 状态码: %d", status_code);
    }
    http->Close();
}
