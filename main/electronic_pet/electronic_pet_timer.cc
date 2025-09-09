/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "electronic_pet_timer.h"
#include "esp_log.h"
#include "electronic_pet.h"
#include <string>
#include <algorithm>
#include <stdio.h>
#include <cstring>
#include "stdlib.h"
#include "application.h"
#include "settings.h"
#define MOUNT_POINT              "/sdcard"
#define TAG "ElectronicPetTimer"


typedef void (*callback_f)(void*);
typedef struct{
    callback_f callback;
    void *arg;
}callback_t;

void test_callback(void* arg){
    // 发送消息给小智
    auto &app = Application::GetInstance();
    if(app.GetDeviceState() == kDeviceStateIdle){
        std::string message = "你的状态:感觉无聊偷偷看一看主人在干什么";
        app.SendMessage(message);
    }
}

// 回调函数的列表
callback_t callback_catalogue[20] = {
    {test_callback, (void *)"test1"},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
    {NULL, NULL},
};

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

ElectronicPetTimer::ElectronicPetTimer(bool from_web, const char* boardID) : from_web_(from_web), boardID(boardID) {

    // clock_ticks_ = 0;
    Settings settings("e_pet", true);
    clock_ticks_ = settings.GetInt("clock_ticks", 0);
    // 读取定时器事件列表
    esp_timer_create_args_t clock_timer_args = {
        .callback = [](void* arg) {
            ElectronicPetTimer*timer = (ElectronicPetTimer*)arg;
            timer->OnClockTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "e_pet_timer",
        .skip_unhandled_events = true
    };
    esp_timer_create(&clock_timer_args, &electromic_prt_timer_);
    esp_timer_start_periodic(electromic_prt_timer_, 1000000); // 1 second
    if(from_web_){
        TimerReadWebTimer();
    }else{
        TimerReadCsvTimer();
    }
}



// 计算最大公约数
long gcd(long a, long b) {
    return b == 0 ? a : gcd(b, a % b);
}

// 计算最小公倍数
long lcm(long a, long b) {
    return (a * b) / gcd(a, b);
}

// 辅助函数：判断闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 辅助函数：获取指定月份的天数
int get_month_days(int year, int month) {
    static const int days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 1 && is_leap_year(year)) 
        return 29;
    return days[month];
}

// 计算下一个触发时间以及重复时间
void calculate_next_trigger(
    int tm_sec, int tm_min, int tm_hour,
    int re_mday, int re_mon, int re_year,
    int re_wday, long* delta_sec, long* interval_sec) {
    
    time_t now;
    time(&now);
    struct tm* current = localtime(&now); // 当前时间

    struct tm next = *current; // 下一个时间

    // 初始化周期参数
    long interval = 1; // 周期时间
    int has_periodic = 0; // 是否周期

    // 处理时间字段
    #define PROCESS_FIELD(field_suffix, var, max, unit_sec) \
        if (!has_periodic &&  var < 0) { \
            interval = lcm(interval, labs(var) * (unit_sec)); \
            has_periodic = 1; \
        } else if (!has_periodic && var >= 0 && var <= max) { \
            next.tm_##field_suffix = var; \
        }

    // 修正调用方式：明确分离结构体字段后缀和变量名
    PROCESS_FIELD(sec, tm_sec, 59, 1)
    PROCESS_FIELD(min, tm_min, 59, 60)
    PROCESS_FIELD(hour, tm_hour, 23, 3600)

    if (re_wday != 0) { // 0表示不设置星期条件
        // 转换输入范围：0-7 → 0-6（周日=0）
        int target_wday = (re_wday < 0) ? labs(re_wday) % 7 : re_wday % 7;
        
        // 处理周期性（负数表示周期）
        if (re_wday < 0) {
            // 计算周周期（绝对值×7天的秒数）
            long week_interval = 604800L; 
            interval = lcm(interval, week_interval);
            has_periodic = 1;
            
            // 设置初始触发日为下一个目标星期几
            int days_to_add = (target_wday - next.tm_wday + 7) % 7;
            days_to_add = (days_to_add == 0) ? 7 : days_to_add; // 确保至少增加1天
            next.tm_mday += days_to_add;
        } 
        // 处理单次指定（正数）
        else if (re_wday > 0) {
            // 计算需要增加的天数（考虑跨周情况）
            int days_diff = (target_wday - next.tm_wday + 7) % 7;
            // 如果当天已过目标星期几，则跳到下周
            if (days_diff == 0 && mktime(&next) <= now) {
                days_diff = 7;
            }
            next.tm_mday += days_diff;
        }
        
    }

    // 处理日期字段
    if (!has_periodic &&  re_mday < 0) {
        interval = lcm(interval, labs(re_mday) * 86400L);
        has_periodic = 1;
    } else if (!has_periodic &&  re_mday > 0) {
        next.tm_mday = re_mday;
    }

    if (!has_periodic &&  re_mon < 0) { // 月周期处理
        if(mktime(&next) < now) {
            next.tm_mon += labs(re_mon);
        }

        int year = next.tm_year + 1900 + next.tm_mon / 12;
        int month = next.tm_mon % 12;

        interval = lcm(interval, labs(re_mon) * get_month_days(year, month) *86400L); // 近似值
        // 调整日期到有效值

        int max_day = get_month_days(year, month);
        if (next.tm_mday > max_day)
            next.tm_mday = max_day;
        has_periodic = 1;
    } else if (!has_periodic &&  re_mon > 0) {
        next.tm_mon = re_mon - 1;
    }

    if (!has_periodic &&  re_year < 0) { // 年周期处理
        int year = next.tm_year + 1900;
        interval = lcm(interval, labs(re_year) * (is_leap_year(year) ? 366 : 365) * 86400L);
        if(mktime(&next) < now) {
            next.tm_year += labs(re_year);
        }
        // 闰年调整
        if (next.tm_mon == 1) { // 二月
            int max_day = is_leap_year(year) ? 29 : 28;
            if (next.tm_mday > max_day)
                next.tm_mday = max_day;
        }
        has_periodic = 1;
    } else if (!has_periodic &&  re_year > 0) {
        next.tm_year = re_year - 1900;
    }

    // 计算初始候选时间
    next.tm_isdst = -1;
    time_t candidate = mktime(&next);
    // 自动调整策略
    while (1) {
        // 处理时间已过的情况
        if (candidate <= now) {
            // printf("Candidate time has passed, adjusting...\n");
            if (has_periodic) {
                candidate += interval;
            } else {
                // 单次事件已过期
                *delta_sec = -1;
                *interval_sec = 0;
                return;
            }
            continue;
        }
        break;
    }

    *delta_sec = candidate - now;
    *interval_sec = has_periodic ? interval : 0;
}
/// @brief 处理单条CSV消息
/// @param csv_info 获取的数据
/// @param delta_sec 返回参数, 第一个触发时间
/// @param interval_sec 返回参数, 周期时间
void ElectronicPetTimer::CalculationNextAndRepeat(timer_info_t *csv_info, long *delta_sec, long *interval_sec){
    calculate_next_trigger(
        csv_info->tm_sec, csv_info->tm_min, csv_info->tm_hour,
        csv_info->re_mday, csv_info->re_mon, csv_info->re_year,
        csv_info->re_wday, delta_sec, interval_sec);
    ESP_LOGI(TAG, "Delta sec: %ld, Interval sec: %ld", *delta_sec, *interval_sec);
}

bool ElectronicPetTimer::DealTimerInfo(timer_info_t *timer_info){
    long delta_sec = 0;
    long interval_sec = 0;
    CalculationNextAndRepeat(timer_info, &delta_sec, &interval_sec);
    if(delta_sec <= 0){
        ESP_LOGE(TAG, "Delta sec is less than 0");
        return false;
    }
    char *message = (char*)malloc(strlen(timer_info->message) + 1);
    if (message == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for message");
        return false;;
    }
    strcpy(message, timer_info->message);
    if(timer_info->function_id == 0){
        // 设置定时器事件
        TimerAddTimerEventRepeat(
            time(nullptr) + delta_sec, E_PET_TIMER_MESSAGE, 
            NULL, 
            (void*)message, interval_sec, timer_info->random_l, timer_info->random_h);
    }else{
        // 设置定时器事件
        ESP_LOGI(TAG, "Function ID: %d", timer_info->function_id);
        TimerAddTimerEventRepeat(
            time(nullptr) + delta_sec, E_PET_TIMER_FUNCTION, 
            (callback_f)callback_catalogue[timer_info->function_id - 1].callback, 
            (void*)callback_catalogue[timer_info->function_id - 1].arg, interval_sec, timer_info->random_l, timer_info->random_h);
    }
    ESP_LOGI(TAG, "Timer event added: %s, delta_sec: %ld, interval_sec: %ld", 
             timer_info->message, delta_sec, interval_sec);
    return true;
}


/// @brief 读取定时器配置文件
void ElectronicPetTimer::TimerReadCsvTimer(){
    const char *file_hello = MOUNT_POINT"/electronic_pet.csv";
    ESP_LOGI(TAG, "Reading file: %s", file_hello);
    // 打开文件
    FILE *f = fopen(file_hello, "r");  // 以只读方式打开文件
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        timer_info_t csv_info;

        int ret = sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", 
            &csv_info.tm_sec, &csv_info.tm_min, &csv_info.tm_hour, 
            &csv_info.re_mday, &csv_info.re_mon, &csv_info.re_year,
            &csv_info.re_wday, &csv_info.random_l, &csv_info.random_h,
            &csv_info.function_id, csv_info.message);
        if (ret == 11) {
            DealTimerInfo(&csv_info);
        } else {
            ESP_LOGE(TAG, "Failed to parse line: %s", line);
        }
    }
}

void ElectronicPetTimer::TimerReadWebTimer(){
    // 读取web定时器
    // 读取web定时器
        // 使用GET请求获取游戏列表信息
        auto http = std::unique_ptr<Http>(Board::GetInstance().CreateHttp());
        std::string url = CONFIG_SERVER_BASE_SERVER_URL "/pets/schedule/?boardID=" + boardID;
        if (!http->Open("GET", url)) {
            ESP_LOGE(TAG, "无法打开HTTP连接获取游戏列表");
            return;
        }
        // 使用cJSON解析返回的物品列表
        std::string data = http->ReadAll();
        // ESP_LOGI(TAG, "定时器列表返回数据: %s", data.c_str());
        cJSON *root = cJSON_Parse(data.c_str());
        if (root == NULL) {
            ESP_LOGE(TAG, "解析定时器列表JSON失败: %s", data.c_str());
            http->Close();
            return;
        }
        cJSON *prompts_data = cJSON_GetObjectItem(root, "data");
        if (!cJSON_IsArray(prompts_data)) {
            ESP_LOGE(TAG, "定时器列表JSON不是数组格式");
            cJSON_Delete(root);
            http->Close();
            return;
        }
        int item_count = cJSON_GetArraySize(prompts_data);
        ESP_LOGI(TAG, "Found %d timers in JSON", item_count);
        for (int i = 0; i < item_count; i++) {
            // 解析每一项日程数据，并转为timer_info_t结构体，调用deal_timer_info
            cJSON *item = cJSON_GetArrayItem(prompts_data, i);
            if (!cJSON_IsObject(item)) continue;
            timer_info_t csv_info;
            memset(&csv_info, 0, sizeof(timer_info_t));
            cJSON *tm_sec = cJSON_GetObjectItem(item, "tm_sec");
            cJSON *tm_min = cJSON_GetObjectItem(item, "tm_min");
            cJSON *tm_hour = cJSON_GetObjectItem(item, "tm_hour");
            cJSON *re_mday = cJSON_GetObjectItem(item, "re_mday");
            cJSON *re_mon = cJSON_GetObjectItem(item, "re_mon");
            cJSON *re_year = cJSON_GetObjectItem(item, "re_year");
            cJSON *re_wday = cJSON_GetObjectItem(item, "re_wday");
            cJSON *random_l = cJSON_GetObjectItem(item, "random_l");
            cJSON *random_h = cJSON_GetObjectItem(item, "random_h");
            cJSON *function_id = cJSON_GetObjectItem(item, "function_id");
            cJSON *message = cJSON_GetObjectItem(item, "message");

            if (tm_sec && cJSON_IsNumber(tm_sec)) csv_info.tm_sec = tm_sec->valueint;
            if (tm_min && cJSON_IsNumber(tm_min)) csv_info.tm_min = tm_min->valueint;
            if (tm_hour && cJSON_IsNumber(tm_hour)) csv_info.tm_hour = tm_hour->valueint;
            if (re_mday && cJSON_IsNumber(re_mday)) csv_info.re_mday = re_mday->valueint;
            if (re_mon && cJSON_IsNumber(re_mon)) csv_info.re_mon = re_mon->valueint;
            if (re_year && cJSON_IsNumber(re_year)) csv_info.re_year = re_year->valueint;
            if (re_wday && cJSON_IsNumber(re_wday)) csv_info.re_wday = re_wday->valueint;
            if (random_l && cJSON_IsNumber(random_l)) csv_info.random_l = random_l->valueint;
            if (random_h && cJSON_IsNumber(random_h)) csv_info.random_h = random_h->valueint;
            if (function_id && cJSON_IsNumber(function_id)) csv_info.function_id = function_id->valueint;
            if (message && cJSON_IsString(message)) {
                strncpy(csv_info.message, message->valuestring, sizeof(csv_info.message) - 1);
                csv_info.message[sizeof(csv_info.message) - 1] = '\0';
            } else {
                csv_info.message[0] = '\0';
            }
            DealTimerInfo(&csv_info);
        }
        
        // 清理资源
        cJSON_Delete(root);
        http->Close();
}

ElectronicPetTimer::~ElectronicPetTimer() {
    if (electromic_prt_timer_ != nullptr) {
        esp_timer_stop(electromic_prt_timer_);
        esp_timer_delete(electromic_prt_timer_);
    }
}



void ElectronicPetTimer::OnClockTimer() {
    std::lock_guard<std::mutex> lock(mutex_);
    TimerEventProcess();
    clock_ticks_++;
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr){
        return;
    }
    ESP_LOGI(TAG, "Clock ticks: %d %d", clock_ticks_, 1000 / CONFIG_FREQUENCE_OF_PET);
    if(clock_ticks_ % (1000 / CONFIG_FREQUENCE_OF_PET) == 0){
        ESP_LOGI(TAG, "Clock ticks: %d", clock_ticks_);
        // 更新一下状态
        int state[E_PET_STATE_NUMBER];
        for(int i = 0; i < E_PET_STATE_NUMBER; i++){
            state[i] = ElectronicPet::state_time_change_[pet->GetAction()][i];
        }
        state[E_PET_STATE_MONEY] *= (state[E_PET_STATE_IQ] / 20); // 智商越高，钱越多
        pet->change_statue(ElectronicPet::state_time_change_[pet->GetAction()]);
        

        if(clock_ticks_ % 1000 == 0){
            pet->UploadState();
        }

        if(pet->isGame() == false){
            pet->StateEventDeal();
        }
    }
    // ESP_LOGI(TAG, "Clock ticks: %d", clock_ticks_);
}



void ElectronicPetTimer::TimerEventSort(){
    std::sort(e_pet_timer_events.begin(), e_pet_timer_events.end(), [](const e_pet_timer_event_t& a, const e_pet_timer_event_t& b) {
        return a.trigger_time < b.trigger_time;
    });
}
/// @brief 设置定时器事件(相对时间)
/// @param seconds 多长时间以后
/// @param type 设置的事件类型
/// @param callback 回调函数
/// @param arg 参数(MESAGEE类型时，传入消息字符串的地址, FUNCTION类型时，传入函数的参数)
void ElectronicPetTimer::TimerAddTimerEventRelative(
    int seconds, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    bool repeat
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    time_t current_time = time(nullptr);
    event.trigger_time = current_time + seconds;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    if(repeat){
        event.repeat_time = seconds;
    }else{
        event.repeat_time = 0;
    }
    e_pet_timer_events.push_back(event);
    TimerEventSort();
    ESP_LOGI(TAG, "Added timer event: %lld, type: %d", event.trigger_time, type);
}

/// @brief 设置定时器事件(绝对时间)
/// @param trigger_time 
/// @param type 
/// @param callback 
/// @param arg 
void ElectronicPetTimer::TimerAddTimerEventAbsolute(
    time_t trigger_time, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    bool repeat
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    event.trigger_time = trigger_time;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    if(repeat){
        time_t current_time = time(nullptr);
        event.repeat_time = trigger_time - current_time;
    }else{
        event.repeat_time = 0;
    }
    e_pet_timer_events.push_back(event);
    TimerEventSort();
}

void ElectronicPetTimer::TimerAddTimerEventRepeat(
    time_t trigger_time, 
    e_pet_timer_type_e type, 
    void (*callback)(void*), 
    void* arg,
    int repeat_time,
    int random_l,
    int random_h
){
    std::lock_guard<std::mutex> lock(mutex_);
    e_pet_timer_event_t event;
    event.trigger_time = trigger_time;
    event.type = type;
    if (type == E_PET_TIMER_FUNCTION) {
        event.function.callback = callback;
        event.function.arg = arg;
    } else {
        event.message = (char*)arg;
    }
    event.repeat_time = repeat_time;
    event.random_l = random_l;
    event.random_h = random_h;
    e_pet_timer_events.push_back(event);
    TimerEventSort();
}

void ElectronicPetTimer::TimerEventProcess(){
    time_t current_time = time(nullptr);
    for (auto it = e_pet_timer_events.begin(); it != e_pet_timer_events.end();) {
        if (it->trigger_time <= current_time) {
            if (it->type == E_PET_TIMER_FUNCTION && it->function.callback != nullptr) {
                it->function.callback(it->function.arg);
            } else {
                ESP_LOGI(TAG, "Message: %s", it->message);
                std::string message = it->message;
                // 发送消息给小智
                auto &app = Application::GetInstance();
                app.SendMessage(message);
            }
            
            if(it->repeat_time > 0){
                if(it->random_l){
                    it->trigger_time += ((rand() % (it->random_h - it->random_l + 1)) + it->random_l) * it->repeat_time;
                }else{
                    it->trigger_time += it->repeat_time;
                }
            }else{
                if(it->type == E_PET_TIMER_FUNCTION){
                    // free(it->function.arg); // 释放函数参数
                }else{
                    free(it->message); // 释放消息字符串
                }

                it = e_pet_timer_events.erase(it); // 删除已处理的事件
            }
        } else {
            break;
        }
    }
    TimerEventSort();
}


