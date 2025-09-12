#include "backlight.h"
#include "settings.h"

#include <esp_log.h>
#include <driver/ledc.h>
#include "lvgl.h"
#include "board.h"
#include "display.h"
#define TAG "Backlight"
void DisplayBrightnessTask(void *arg){
    auto backlight = Board::GetInstance().GetBacklight();
    if (!backlight) {
        ESP_LOGE(TAG, "Can't find brightness instance");
        return;
    }
    // ESP_LOGI(TAG, "DisplayBrightnessTask %d", backlight->brightness_time);

    if (backlight->brightness_time == -1)
    {
        // 不需要变化状态
        return;
    }
    else if(backlight->brightness_time == 0)
    {
        // 时间到关灯
        ESP_LOGI(TAG, "Turn off the backlight");
        backlight->SetBrightness(0, true, true);
    }
    backlight->brightness_time--;
}

Backlight::Backlight() {
    // 创建背光渐变定时器
    const esp_timer_create_args_t timer_args = {
        .callback = [](void* arg) {
            auto self = static_cast<Backlight*>(arg);
            self->OnTransitionTimer();
        },
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "backlight_timer",
        .skip_unhandled_events = true,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &transition_timer_));

    // 定义一个自动息屏定时器结构体
    esp_timer_create_args_t timer_args_bl = {	
        .callback = &DisplayBrightnessTask,	//设置回调函数
        .arg = this,	// 设置回调函数参数
        // 可以添加其他参数
    };
    esp_timer_create(&timer_args_bl, &my_timer);	//将创建的定时器句柄存储到 my_timer 中。
    esp_timer_start_periodic(my_timer, 1000000);	//启动定时器，周期为 1s
}

Backlight::~Backlight() {
    if (transition_timer_ != nullptr) {
        esp_timer_stop(transition_timer_);
        esp_timer_delete(transition_timer_);
    }
    if (my_timer != nullptr) {
        esp_timer_stop(my_timer);
        esp_timer_delete(my_timer);
    }
}

void Backlight::RestoreBrightness() {
    // Load brightness from settings
    Settings settings("display");  
    int saved_brightness = settings.GetInt("brightness", 75);
    
    // 检查亮度值是否为0或过小，设置默认值
    if (saved_brightness <= 0) {
        ESP_LOGW(TAG, "Brightness value (%d) is too small, setting to default (10)", saved_brightness);
        saved_brightness = 10;  // 设置一个较低的默认值
    }
    
    SetBrightness(saved_brightness);
}

void Backlight::SetBrightness(uint8_t brightness, bool permanent, bool lock_screen) {
    if (brightness > 100) {
        brightness = 100;
    }

    if (brightness_ == brightness) {
        return;
    }

    if (permanent) {
        Settings settings("display", true);
        settings.SetInt("brightness", brightness);
    }

    if(lock_screen){
        last_light = brightness_;
        ESP_LOGI(TAG, "last_light = %d", last_light);
    }else{
        last_light = brightness;
    }

    target_brightness_ = brightness;
    step_ = (target_brightness_ > brightness_) ? 1 : -1;

    if (transition_timer_ != nullptr) {
        // 启动定时器，每 5ms 更新一次
        esp_timer_start_periodic(transition_timer_, 5 * 1000);
    }
    ESP_LOGI(TAG, "Set brightness to %d", brightness);
}

void Backlight::OnTransitionTimer() {
    if (brightness_ == target_brightness_) {
        esp_timer_stop(transition_timer_);
        return;
    }

    brightness_ += step_;
    SetBrightnessImpl(brightness_);

    if (brightness_ == target_brightness_) {
        esp_timer_stop(transition_timer_);
    }
}

PwmBacklight::PwmBacklight(gpio_num_t pin, bool output_invert) : Backlight() {
    const ledc_timer_config_t backlight_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 25000, //背光pwm频率需要高一点，防止电感啸叫
        .clk_cfg = LEDC_AUTO_CLK,
        .deconfigure = false
    };
    ESP_ERROR_CHECK(ledc_timer_config(&backlight_timer));

    // Setup LEDC peripheral for PWM backlight control
    const ledc_channel_config_t backlight_channel = {
        .gpio_num = pin,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
        .flags = {
            .output_invert = output_invert,
        }
    };
    ESP_ERROR_CHECK(ledc_channel_config(&backlight_channel));
}

PwmBacklight::~PwmBacklight() {
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
}

void PwmBacklight::SetBrightnessImpl(uint8_t brightness) {
    // LEDC resolution set to 10bits, thus: 100% = 1023
    uint32_t duty_cycle = (1023 * brightness) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void Backlight::DisplayBrightnessReset()
{
    if(brightness_time == -1)
    {
        // 亮屏
        ESP_LOGI(TAG, "Turn on the backlight %d", last_light);
        SetBrightness(last_light);
    }
    brightness_time = default_sleep_time;
    bright_keep = false;
}

void Backlight::DisplayBrightnessKeep(void)
{
    ESP_LOGI(TAG, "Keep the backlight on %d", last_light);
    SetBrightness(last_light);
    brightness_time = -1;
    bright_keep = true;
}

int Backlight::DisplayBrightnessGetDefalutTime(void)
{
    return default_sleep_time;
}