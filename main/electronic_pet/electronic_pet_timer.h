/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once

#include "time.h"
#include <esp_timer.h>
#include <atomic>
#include <vector>
#include "electronic_config.h"
typedef enum {
    E_PET_TIMER_MESSAGE = 0, // 消息
    E_PET_TIMER_FUNCTION, // 函数
}e_pet_timer_type_e;


typedef struct {  
    void (*callback)(void*);  
    void* arg;  
}e_pet_timer_function;

typedef struct{
    time_t trigger_time;
    e_pet_timer_type_e type;
    int repeat_time;
    union {
        e_pet_timer_function function;
        char *message;
    };
}e_pet_timer_event_t;

typedef struct{
    int tm_sec; // 秒
    int tm_min; // 分
    int tm_hour; // 时
    int re_mday; // 日
    int re_mon; // 月
    int re_year; // 年
    int re_wday; // 星期几
    int random_l;
    int random_h;
    int function_id;
    char message[100];
}timer_info_t;



class ElectronicPetTimer {
private:
    int clock_ticks_;  // 时钟
    std::mutex mutex_;
    esp_timer_handle_t electromic_prt_timer_ = nullptr;
    std::vector<e_pet_timer_event_t> e_pet_timer_events; // 处理事件列表
public:
    ElectronicPetTimer();
    ~ElectronicPetTimer();
    void timer_read_csv_timer();
    void OnClockTimer();
    void timer_event_sort();
    void timer_add_timer_event_relative(int seconds, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_absolute(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, bool repeat);
    void timer_add_timer_event_repeat(time_t trigger_time, e_pet_timer_type_e type, void (*callback)(void*), void* arg, int repeat_time);
    void deal_one_csv_message(timer_info_t *csv_info, long *delta_sec, long *interval_sec);
    void timer_event_process();
    bool deal_timer_info(timer_info_t *timer_info);
};


#define SECOND_ONE_SECOND 1 
#define SECOND_ONE_MINUTE 60
#define SECOND_ONE_HOUR 3600
#define SECOND_ONE_DAY 86400
#define SECOND_ONE_WEEK 604800
#define SECOND_ONE_MONTH 2592000
#define SECOND_ONE_YEAR 31536000
