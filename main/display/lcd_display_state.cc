/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "lcd_display.h"
#include "lvgl.h"
#include "electronic_pet.h"
#define TAG "LcdDisplay"
void LcdDisplay::StateUI(){
    DisplayLockGuard lock(this);
    screen_state_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_state_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_state_, LIGHT_BACKGROUND_COLOR, 0);
    lv_obj_clear_flag(screen_state_, LV_OBJ_FLAG_SCROLLABLE);

    // 创建主容器（垂直布局）
    lv_obj_t* main_cont = lv_obj_create(screen_state_);
    lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(main_cont, 20, 0);
    lv_obj_set_style_pad_row(main_cont, ITEM_SPACING, 0);
    lv_obj_remove_style(main_cont, NULL, LV_PART_SCROLLBAR);

    // 设置状态栏
    lv_obj_t * action_bar_ = lv_obj_create(main_cont);
    lv_obj_set_size(action_bar_, LV_PCT(100), 40);
    lv_obj_set_style_radius(action_bar_, 0, 0);
    lv_obj_set_style_bg_color(action_bar_, lv_color_hex(0xE0E0E0), 0);
    lv_obj_set_style_text_color(action_bar_, lv_color_hex(0x666666), 0);
    lv_obj_set_style_text_font(action_bar_, fonts_.text_font, 0);
    lv_obj_set_style_pad_all(action_bar_, 0, 0);
    lv_obj_set_style_pad_column(action_bar_, 0, 0);
    
    action_label_ = lv_label_create(action_bar_);
    lv_label_set_text(action_label_, "");
    lv_obj_set_style_text_align(action_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(action_label_, LV_ALIGN_CENTER, 0, 0); // 居中对齐

    // 创建每个状态项
    for(int i = 0; i < E_PET_STATE_NUMBER + 2; i++) {
        StateItemCreate(main_cont, i);
    }

    lv_obj_add_flag(screen_state_, LV_OBJ_FLAG_HIDDEN);
}

void LcdDisplay::StateItemCreate(lv_obj_t *parent, int i){
    // 状态项容器
    lv_obj_t* item = lv_obj_create(parent);
    lv_obj_set_size(item, LV_PCT(100), ITEM_HEIGHT);
    lv_obj_set_style_radius(item, 15, 0);
    lv_obj_set_style_bg_color(item, lv_color_hex(0xFFF3F9), 0);
    lv_obj_set_style_shadow_width(item, 15, 0);
    lv_obj_set_style_shadow_color(item, lv_color_hex(0xCCCCCC), 0);
    lv_obj_remove_style(item, NULL, LV_PART_SCROLLBAR);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);

    
    // 图标部分
    lv_obj_t* icon = lv_label_create(item);
    // lv_label_set_text(icon, get_state_icon(i)); // 需要实现图标获取函数
    lv_obj_set_style_text_color(icon, lv_color_hex(0xFF88A4), 0);
    // lv_obj_set_style_text_font(icon, ICON_FONT, 0);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 15, 0);

    // 状态名称和数值
    lv_obj_t* name = lv_label_create(item);
    // lv_label_set_text_fmt(name, "%s: %d%%", pet->GetStateName(i), pet->GetState(i));
    lv_obj_set_style_text_font(name, fonts_.text_font, 0);
    lv_obj_align(name, LV_ALIGN_LEFT_MID, 32 + 25, -10);

    // 进度条
    lv_obj_t* bar = lv_bar_create(item);
    if(i < E_PET_STATE_DIVIDING_LINE + 2)
        lv_bar_set_range(bar, 0, 100);
    else
        lv_bar_set_range(bar, 0, 5000);
    // lv_bar_set_value(bar, pet->GetState(i), LV_ANIM_ON);
    lv_obj_set_size(bar, PROGRESS_WIDTH, 15);
    lv_obj_align(bar, LV_ALIGN_LEFT_MID, 32 + 25, 15);
    lv_obj_set_style_radius(bar, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xFFE4EB), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x00facc), LV_PART_INDICATOR);
    
    state_items[i] = item; // 保存对象指针方便后续更新
}

// 添加状态更新函数
void LcdDisplay::UpdateStateGui() {
    DisplayLockGuard lock(this);
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    char * action_now = pet->GetActionName();
    if(action_now == nullptr) {
        ESP_LOGE(TAG, "GetActionName returned null");
        return;
    }
    // ESP_LOGI(TAG, "action_now: %s", action_now);
    lv_label_set_text_fmt(action_label_, "%s", action_now);
    int level = pet->getLevel();
    int experience = pet->getExperience();
    lv_obj_t* item = state_items[0];
    // 更新数值显示
    lv_label_set_text_fmt(lv_obj_get_child(item, 1), 
                        "%s: %d", "等级", level);
    
    // 更新进度条
    lv_obj_t* bar = (lv_obj_t*)lv_obj_get_child(item, 2);
    lv_bar_set_value(bar, level, LV_ANIM_ON);
    
    // 动态颜色（示例：根据数值改变进度条颜色）
    lv_color_t color = lv_color_hex(0x0099ff);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);

    item = state_items[1];
    // 更新数值显示
    lv_label_set_text_fmt(lv_obj_get_child(item, 1), 
                        "%s: %d", "经验", experience);
    
    // 更新进度条
    bar = (lv_obj_t*)lv_obj_get_child(item, 2);
    lv_bar_set_range(bar, 0, level * level);
    lv_bar_set_value(bar, experience > level * level ? experience : level * level, LV_ANIM_ON);
    
    // 动态颜色（示例：根据数值改变进度条颜色）
    color = lv_color_hex(0x0099ff);
    lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);


    for(int i = 2; i < E_PET_STATE_NUMBER + 2; i++) {
        item = state_items[i];
        // 更新数值显示
        lv_label_set_text_fmt(lv_obj_get_child(item, 1), 
                            "%s: %d", pet->GetStateName(i - 2), pet->GetState(i - 2));
        
        // 更新进度条
        bar = (lv_obj_t*)lv_obj_get_child(item, 2);
        lv_bar_set_value(bar, pet->GetState(i - 2), LV_ANIM_ON);
        
        // 动态颜色（示例：根据数值改变进度条颜色）
        color = pet->GetState(i - 2) > 30 ? 
            lv_color_hex(0x0099ff) : lv_color_hex(0xff6699);
        lv_obj_set_style_bg_color(bar, color, LV_PART_INDICATOR);
    }


}