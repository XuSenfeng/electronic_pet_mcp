/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once

#include <cstdint>
#include <functional>

#include <driver/gpio.h>
#include <esp_timer.h>


class Backlight {
public:
    Backlight();
    ~Backlight();

    void RestoreBrightness();
    void SetBrightness(uint8_t brightness, bool permanent = false, bool lock_screen = false);
    inline uint8_t brightness() const { return brightness_; }

    void DisplayBrightnessReset(void);
    int DisplayBrightnessGetDefalutTime(void);
    void DisplayBrightnessKeep(void);
    esp_timer_handle_t my_timer;
    int brightness_time = -1;  // 初识亮屏时间

protected:
    void OnTransitionTimer();
    virtual void SetBrightnessImpl(uint8_t brightness) = 0;

    esp_timer_handle_t transition_timer_ = nullptr;
    uint8_t brightness_ = 0;
    uint8_t target_brightness_ = 0;
    uint8_t step_ = 1;

    int last_light = 0;  // 记录上次亮度
    int default_sleep_time = 30;    // 默认休眠时间
};


class PwmBacklight : public Backlight {
public:
    PwmBacklight(gpio_num_t pin, bool output_invert = false);
    ~PwmBacklight();

    void SetBrightnessImpl(uint8_t brightness) override;
};
