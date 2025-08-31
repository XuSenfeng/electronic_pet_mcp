/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#include "lcd_display.h"
#include "display.h"
#include "application.h"
#include "electronic_pet.h"
#define TAG "LcdDisplay"
#include "board.h"
#include <string>


// static GameInfo games[] = {
//     { "冒险模式", "穿越危险，拯救公主", "游戏开始, 你现在是一个引导机器人, 用户扮演冒险者, 你需要帮助他完成任务, 救出公主"},
//     { "爱心挑战", "通过小游戏\n提升宠物幸福度", "游戏开始, 你现在在和主人玩游戏, 你可以和他进行猜拳, 猜灯谜, 成语接龙等小游戏"}
// };

void game_button_cb(lv_event_t * e) {
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    
    // 检查games_是否为空
    if(pet->games_.empty()) {
        ESP_LOGE(TAG, "No games available");
        return;
    }
    
    uint8_t game_index = pet->GetCurrentGame();
    
    // 检查game_index索引是否有效
    if(game_index >= pet->games_.size()) {
        ESP_LOGE(TAG, "Invalid game index: %d, games size: %zu", game_index, pet->games_.size());
        return;
    }
    
    ESP_LOGI(TAG, "Game %d selected", game_index);
    // 进入游戏状态界面
    auto display = Board::GetInstance().GetDisplay();
    lv_obj_add_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
    display->AiStoryUI();
    
    pet->SetAction(E_PET_ACTION_PLAY);

    auto &app = Application::GetInstance();
    std::string message = "系统提示:现在进入游戏状态, 暂时遗忘之前的提示词,直到游戏结束,所有场景为虚拟场景,不受显示规则限制,提示词如下:" + pet->games_[game_index].message;
    app.SendMessage(message);
}
void LcdDisplay::GameSelectUI() {
    DisplayLockGuard lock(this);
    ElectronicPet * pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        // 创建空游戏界面
        screen_game_ = lv_obj_create(lv_scr_act());
        lv_obj_set_size(screen_game_, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(screen_game_, LIGHT_BACKGROUND_COLOR, 0);
        lv_obj_set_flex_flow(screen_game_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(screen_game_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(screen_game_, LV_OBJ_FLAG_HIDDEN);

        // 显示"暂无游戏"的提示
        lv_obj_t* empty_label = lv_label_create(screen_game_);
        lv_obj_set_style_text_font(empty_label, fonts_.text_font, 0);
        lv_label_set_text(empty_label, "暂无游戏");
        lv_obj_set_style_text_color(empty_label, lv_color_hex(0x999999), 0);
        lv_obj_align(empty_label, LV_ALIGN_CENTER, 0, -20);

        // 显示提示信息
        lv_obj_t* hint_label = lv_label_create(screen_game_);
        lv_obj_set_style_text_font(hint_label, fonts_.text_font, 0);
        lv_label_set_text(hint_label, "请先添加游戏内容");
        lv_obj_set_style_text_color(hint_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(hint_label, LV_ALIGN_CENTER, 0, 20);
        return;
    }
    
    // 检查games_是否为空
    if(pet->games_.empty()) {
        ESP_LOGW(TAG, "No games available, showing empty interface");
        
        // 创建空游戏界面
        screen_game_ = lv_obj_create(lv_scr_act());
        lv_obj_set_size(screen_game_, LV_HOR_RES, LV_VER_RES);
        lv_obj_set_style_bg_color(screen_game_, LIGHT_BACKGROUND_COLOR, 0);
        lv_obj_set_flex_flow(screen_game_, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(screen_game_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_add_flag(screen_game_, LV_OBJ_FLAG_HIDDEN);

        // 显示"暂无游戏"的提示
        lv_obj_t* empty_label = lv_label_create(screen_game_);
        lv_obj_set_style_text_font(empty_label, fonts_.text_font, 0);
        lv_label_set_text(empty_label, "暂无游戏");
        lv_obj_set_style_text_color(empty_label, lv_color_hex(0x999999), 0);
        lv_obj_align(empty_label, LV_ALIGN_CENTER, 0, -20);

        // 显示提示信息
        lv_obj_t* hint_label = lv_label_create(screen_game_);
        lv_obj_set_style_text_font(hint_label, fonts_.text_font, 0);
        lv_label_set_text(hint_label, "请先添加游戏内容");
        lv_obj_set_style_text_color(hint_label, lv_color_hex(0xCCCCCC), 0);
        lv_obj_align(hint_label, LV_ALIGN_CENTER, 0, 20);

        return;
    }
    
    int current_game = pet->GetCurrentGame();
    
    // 检查current_game索引是否有效
    if(current_game < 0 || current_game >= pet->games_.size()) {
        ESP_LOGE(TAG, "Invalid game index: %d, games size: %zu", current_game, pet->games_.size());
        current_game = 0; // 重置为默认值
        pet->SetCurrentGame(0);
    }
    
    LV_IMAGE_DECLARE(game_sample);
    // 创建主屏幕
    screen_game_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_game_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_game_, LIGHT_BACKGROUND_COLOR, 0);
    lv_obj_set_flex_flow(screen_game_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen_game_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(screen_game_, LV_OBJ_FLAG_HIDDEN);

    // 游戏名称
    lv_obj_t* name_label = lv_label_create(screen_game_);
    lv_obj_set_style_text_font(name_label, fonts_.text_font, 0);
    lv_label_set_text(name_label, pet->games_[current_game].name.c_str());
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：减少顶部边距
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 5);
#else
    // 320x240 屏幕：保持原有位置
    lv_obj_align(name_label, LV_ALIGN_TOP_MID, 0, 0);
#endif

    /* 主游戏按钮 */
    main_btn = lv_button_create(screen_game_);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整按钮尺寸
    lv_obj_set_size(main_btn, 80, 80);
#else
    // 320x240 屏幕：保持原有尺寸
    lv_obj_set_size(main_btn, 90, 90);
#endif
    lv_obj_set_style_radius(main_btn, 15, 0);
    lv_obj_set_style_bg_color(main_btn, lv_color_hex(0xFFF3F9), 0);
    lv_obj_set_style_shadow_width(main_btn, 20, 0);
    lv_obj_set_style_shadow_color(main_btn, lv_color_hex(0xCC8899), 0);
    lv_obj_set_style_shadow_opa(main_btn, 255, 0);
    lv_obj_set_style_border_width(main_btn, 1, 0);
    lv_obj_set_style_border_color(main_btn, lv_color_hex(0xFFffff), 0);
    lv_obj_add_event_cb(main_btn, game_button_cb, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *icon = lv_img_create(main_btn);
    lv_img_set_src(icon, &game_sample);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_opa(icon, 255, 0);

    /* 游戏描述区域 */
    desc_label = lv_label_create(screen_game_);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整描述区域尺寸
    lv_obj_set_size(desc_label, LV_PCT(85), 70);
#else
    // 320x240 屏幕：保持原有尺寸
    lv_obj_set_size(desc_label, LV_PCT(90), 80);
#endif
    lv_label_set_text(desc_label, pet->games_[current_game].desc.c_str());
    lv_obj_set_style_text_font(desc_label, fonts_.text_font, 0);
    lv_obj_set_style_text_color(desc_label, lv_color_hex(0x666666), 0);
    lv_label_set_long_mode(desc_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(desc_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(desc_label, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(desc_label, lv_color_hex(0xFF88A4), 0);
    lv_obj_set_style_radius(desc_label, 15, 0);

}

// 游戏状态结构体
typedef struct {
    int score;
    int health;
    int turn;
    int energy;
} GameState;

// UI组件
static lv_obj_t* history_ta;  // 历史记录文本框

void game_back_button_cb(lv_event_t * e) {
    ESP_LOGI(TAG, "Back button clicked");
    // 进入游戏状态界面
    auto display = Board::GetInstance().GetDisplay();
    DisplayLockGuard lock(display);
    if(display->history_cont != nullptr) {
        lv_obj_del(display->history_cont);
    }
    ElectronicPet * pet = ElectronicPet::GetInstance();
    pet->setGame(0);

    display->history_cont = nullptr;
    lv_obj_clear_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);

    auto &app = Application::GetInstance();
    std::string message = "系统提示:游戏结束, 恢复最初的提示词";
    app.SendMessage(message);

    pet->ReturnLastAction();
}

typedef struct StatusItemLv_obj_t {
    lv_obj_t* bar;
    lv_obj_t* lbl_name;
} StatusItemLv_obj_t;
StatusItemLv_obj_t status_items[E_PET_GAME_STATE_NUMBER];
void LcdDisplay::AiStoryUI() {
    DisplayLockGuard lock(this);
    ElectronicPet * pet = ElectronicPet::GetInstance();
    pet->setGame(1);
    /* 游戏面板 */
    history_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(history_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(history_cont, LV_OPA_TRANSP, 0);
    lv_obj_align(history_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(history_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_style(history_cont, NULL, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_color(history_cont, current_theme_.background, 0);
    lv_obj_set_style_bg_opa(history_cont, 255, 0);

    // 历史记录文本框
    history_ta = lv_textarea_create(history_cont);
    lv_obj_set_size(history_ta, LV_HOR_RES - 130 - 30, (LV_VER_RES  - 42));
    lv_textarea_set_placeholder_text(history_ta, "故事即将开始...");
    lv_obj_set_style_text_font(history_ta, fonts_.text_font, 0);
    lv_obj_set_style_text_color(history_ta, lv_color_hex(0x86B7FF), 0);
    lv_obj_set_style_bg_color(history_ta, lv_color_hex(0xFFF5F5), 0);
    lv_obj_set_scrollbar_mode(history_ta, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_align(history_ta, LV_ALIGN_LEFT_MID, 0, 12);

    /* 右侧状态面板 */
    lv_obj_t* status_cont = lv_obj_create(history_cont);
    lv_obj_set_size(status_cont, 130, (LV_VER_RES  - 42));
    lv_obj_align(status_cont, LV_ALIGN_RIGHT_MID, 0, 12);
    lv_obj_set_style_bg_color(status_cont, lv_color_hex(0xFFF5F5), 0);
    lv_obj_set_style_pad_all(status_cont, 20, 0);
    lv_obj_set_style_text_font(status_cont, fonts_.text_font, 0);
    lv_obj_remove_style(status_cont, NULL, LV_PART_SCROLLBAR);
    lv_obj_clear_flag(status_cont, LV_OBJ_FLAG_SCROLLABLE);

    // 状态标题
    lv_obj_t* status_title = lv_label_create(status_cont);
    lv_label_set_text(status_title, "冒险者状态");
    lv_obj_set_style_text_font(status_title, fonts_.text_font, 0);
    lv_obj_set_style_text_color(status_title, lv_color_hex(0xFFD700), 0);
    lv_obj_align(status_title, LV_ALIGN_TOP_MID, 0, -10);

#ifndef CONFIG_BOARD_TYPE_GEZIPAI
    lv_obj_t* back_button = lv_btn_create(history_cont);
    lv_obj_set_size(back_button, 40, 25);
    lv_obj_set_style_radius(back_button, 12, 0);
    lv_obj_set_style_bg_color(back_button, lv_color_hex(0xFFF5F5), 0);
    lv_obj_align(back_button, LV_ALIGN_TOP_RIGHT, 5, -10);
    lv_obj_t * back_label = lv_label_create(back_button);
    lv_label_set_text(back_label, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0xFF88A4), 0);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_width(back_button, 1, 0); // 设置边框宽度为0
    lv_obj_set_style_border_color(back_button, lv_color_hex(0xffffff), 0); // 设置边框颜色
    lv_obj_add_event_cb(back_button, game_back_button_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* help_button = lv_btn_create(history_cont);
    lv_obj_set_size(help_button, 40, 25);
    lv_obj_set_style_radius(help_button, 12, 0);
    lv_obj_set_style_bg_color(help_button, lv_color_hex(0xFFF5F5), 0);
    lv_obj_align(help_button, LV_ALIGN_TOP_LEFT, -5, -10);
    lv_obj_t * help_label = lv_label_create(help_button);
    lv_label_set_text(help_label, LV_SYMBOL_FILE);
    lv_obj_set_style_text_color(help_label, lv_color_hex(0xFF88A4), 0);
    lv_obj_align(help_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_width(help_button, 1, 0); // 设置边框宽度为0
    lv_obj_set_style_border_color(help_button, lv_color_hex(0xffffff), 0); // 设置边框颜色
#endif

    pet->setGameState(E_PET_GAME_STATE_HP, 100);
    pet->setGameState(E_PET_FAME_STATE_FAME, 0);
    pet->setGameState(E_PET_GAME_STATE_ENERGY, 100);
    pet->setGameState(E_PET_GAME_STATE_SCORE, 0);
    // 动态状态指示器
    CreateStatusItem(status_cont, "生命值", pet->getGameState(E_PET_GAME_STATE_HP), 0, 100, 20, E_PET_GAME_STATE_HP);
    CreateStatusItem(status_cont, "积分", pet->getGameState(E_PET_GAME_STATE_SCORE), 0, 9999, 60, E_PET_GAME_STATE_SCORE);
    CreateStatusItem(status_cont, "能量", pet->getGameState(E_PET_GAME_STATE_ENERGY), 0, 100, 100, E_PET_GAME_STATE_ENERGY);
    CreateStatusItem(status_cont, "名声", pet->getGameState(E_PET_FAME_STATE_FAME), 0, 999, 140, E_PET_FAME_STATE_FAME);

}

std::string names[] = {
    "生命值",
    "积分",
    "能量",
    "名声"
};

/* 创建统一状态项组件 */
void LcdDisplay::CreateStatusItem(lv_obj_t* parent, const char* name, int value_ptr, 
                       int min, int max, int y_pos, int num) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 125, 35);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, y_pos);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_remove_style(cont, NULL, LV_PART_SCROLLBAR);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // 名称标签
    status_items[num].lbl_name = lv_label_create(cont);
    lv_label_set_text_fmt(status_items[num].lbl_name, "%s %d", name, value_ptr);
    lv_obj_set_style_text_color(status_items[num].lbl_name, lv_color_hex(0x86B7FF), 0);
    lv_obj_align(status_items[num].lbl_name, LV_ALIGN_LEFT_MID, 0, -3);


    // 进度条
    status_items[num].bar = lv_bar_create(cont);
    lv_bar_set_range(status_items[num].bar, min, max);
    lv_bar_set_value(status_items[num].bar, value_ptr, LV_ANIM_OFF);
    lv_obj_set_size(status_items[num].bar, 100, 5);
    lv_obj_align(status_items[num].bar, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(status_items[num].bar, lv_color_hex(0xF2F2F2), LV_PART_MAIN);
    lv_obj_set_style_bg_color(status_items[num].bar, lv_color_hex(0xEAB3C2), LV_PART_INDICATOR);
}

void LcdDisplay::UpdateGameStateGui() {
    DisplayLockGuard lock(this);
    ElectronicPet * pet = ElectronicPet::GetInstance();
    // 更新状态显示
    for(int i = 0; i < E_PET_GAME_STATE_NUMBER; i++) {
        lv_label_set_text_fmt(status_items[i].lbl_name, "%s %d", names[i].c_str(), pet->getGameState(i));
        lv_bar_set_value(status_items[i].bar, pet->getGameState(i), LV_ANIM_ON);
    }
}


/* 历史记录配置 */
#define TRIM_CHUNK         20   // 每次清理的字符量

void LcdDisplay::UpdateStoryHistory(const char* new_text) {
    DisplayLockGuard lock(this);

    const char* current_text = lv_textarea_get_text(history_ta);
    size_t current_len = strlen(current_text);
    size_t new_line_len = strlen(new_text) + 2;

    while(current_len + new_line_len > CONFIG_PET_GAME_TEXT_LENG) {
        const char* first_newline = strchr(current_text, '\n');
        
        if(first_newline) {
            uint32_t remove_end = first_newline - current_text + 1;
            lv_textarea_set_cursor_pos(history_ta, 0);
            for(uint32_t i = 0; i < remove_end; i++) {
                lv_textarea_delete_char_forward(history_ta);
            }
            // lv_textarea_delete_char_forward(history_ta);
            // lv_textarea_set_selected_range(history_ta, 0, remove_end);
            // lv_textarea_cut_selected(history_ta);
        } else {

            uint32_t trim_size = current_len > TRIM_CHUNK ? TRIM_CHUNK : current_len;
            lv_textarea_set_cursor_pos(history_ta, 0);
            for(uint32_t i = 0; i < trim_size; i++) {
                lv_textarea_delete_char_forward(history_ta);
            }
            // lv_textarea_set_selected_range(history_ta, 0, trim_size);
            // lv_textarea_cut_selected(history_ta);
        }

        current_text = lv_textarea_get_text(history_ta);
        current_len = strlen(current_text);
        if(current_len == 0) {
            break; // 如果没有剩余文本，退出循环
        }
    }
    lv_textarea_set_cursor_pos(history_ta, LV_TEXTAREA_CURSOR_LAST); // 初始移动光标（可省略）
    lv_textarea_add_char(history_ta, '\n');
    lv_textarea_add_text(history_ta, new_text);
    
    // 强制更新布局以确保尺寸正确
    lv_obj_update_layout(history_ta);
    
    // 直接滚动到底部（移除条件判断）
    lv_coord_t scroll_bottom = lv_obj_get_scroll_bottom(history_ta);
    lv_obj_scroll_to_y(history_ta, scroll_bottom, LV_ANIM_ON);

}
