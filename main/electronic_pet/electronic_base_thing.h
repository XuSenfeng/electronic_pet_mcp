/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once
#include <string>
#include "lvgl.h"
#include "electronic_config.h"
#include <atomic>
typedef enum{
    E_THING_TYPE_FOOD = 0,
    E_THING_TYPE_TOY,
    E_THING_TYPE_CLOTHES,
}thing_type_e;

class BaseThing{
public:
    BaseThing(const char * name, lv_image_dsc_t thing_pic, const char *thing_description, thing_type_e thing_type, int num, int *state)
        : thing_pic(thing_pic), thing_type(thing_type), num_(num){
            name_ = name;
            thing_description_ = thing_description;
            vigor_ = state[E_PET_STATE_VIGIR]; // 金钱
            satiety_ = state[E_PET_STATE_SATITY]; // 精神状态
            happiness_ = state[E_PET_STATE_HAPPINESS]; // 快乐度
            money_ = state[E_PET_STATE_MONEY]; // 金钱
            iq_ = state[E_PET_STATE_IQ]; // 智商
        };

    virtual ~BaseThing() = default;

    virtual void Use(){}; // 使用物品
    int GetNum() const { return num_; }
    void SetNum(int num);
    int GetVigor() const { return vigor_; }
    void SetVigor(int vigor) { vigor_ = vigor; }
    int GetSatiety() const { return satiety_; }
    void SetSatiety(int satiety) { satiety_ = satiety; }
    int GetHappiness() const { return happiness_; }
    void SetHappiness(int happiness) { happiness_ = happiness; }
    int GetMoney() const { return money_; }
    void SetMoney(int money) { money_ = money; }
    int GetIq() const { return iq_; }
    void SetIq(int iq) { iq_ = iq; }
    const char *GetName() const { return name_.c_str(); }
    const char *GetDescription() const { return thing_description_.c_str(); }
    
protected:
    std::mutex mutex_;
    std::string name_;
    lv_image_dsc_t thing_pic;
    std::string thing_description_; // 使用之后发送给小智
    thing_type_e thing_type;
    int num_;
    int money_; // 金钱
    int vigor_; // 精神状态
    int satiety_; // 饱食度
    int happiness_; // 快乐度
    int iq_; // 智商
};