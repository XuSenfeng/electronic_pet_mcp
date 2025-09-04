/*
 * @Author: XvSenfeng
 * @Email: helloworldjiao@163.com
 * @Date: 2025-09-02 12:17:41
 * @LastEditTime: 2025-09-04 11:07:34
 * @FilePath: /xiaozhi-esp32/main/display/lcd_display_catalogue.cc
 */
 #include "lcd_display.h"

 #define TAG "LcdDisplay"

 #include <esp_log.h>
 #include <esp_lvgl_port.h>
 #include "assets/lang_config.h"
 #include <cstring>
 #include "settings.h"
 #include "application.h"
 #include "board.h"
 #include "electronic_pet.h"

 void game_select_button_cb(lv_event_t * e) {
    ESP_LOGI(TAG, "Game select button clicked");
    int index = (int)lv_event_get_user_data(e);
    auto display = Board::GetInstance().GetDisplay();
    DisplayLockGuard lock(display);
    
    // 将 Display 指针转换为 LcdDisplay 指针
    LcdDisplay* lcd_display = static_cast<LcdDisplay*>(display);
    if (lcd_display && index < lcd_display->game_screens_.size()) {
        (lcd_display->*(lcd_display->game_screens_[index].UIFunc))();
    }
    
    if (display->screen_game_ != nullptr) {
        lv_obj_clear_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
    }
    display->screen_now_ = display->screen_game_;
    // switch (index) {
    //     case 0:
    //         display->AIPlayGameUI();
    //         // 显示新创建的游戏界面
    //         if (display->screen_game_ != nullptr) {
    //             lv_obj_clear_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
    //         }
    //         display->screen_now_ = display->screen_game_;
    //         break;
    //     case 1:
    //         display->MessageUI();
    //         // 显示新创建的游戏界面
    //         if (display->screen_game_ != nullptr) {
    //             lv_obj_clear_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
    //         }
    //         display->screen_now_ = display->screen_game_;
    //         break;
    //     case 2:
    //         break;
    //     case 3:
    //         break;
    //     case 4:
    //         break;
    //     default:
    //         break;
    // }
}

 void LcdDisplay::GameSelectUI() {
    DisplayLockGuard lock(this);
    
    // 创建可爱风格游戏选择主屏幕
    screen_game_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_game_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_game_, CUTE_WHITE, 0);
    lv_obj_set_flex_flow(screen_game_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen_game_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(screen_game_, LV_OBJ_FLAG_HIDDEN);

    // 可爱风格标题
    lv_obj_t* title_label = lv_label_create(screen_game_);
    lv_obj_set_style_text_font(title_label, fonts_.text_font, 0);
    lv_label_set_text(title_label, "游戏选择");
    lv_obj_set_style_text_color(title_label, CUTE_PINK_PRIMARY, 0);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 8);
#else
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 15);
#endif

    // 创建滚动容器来容纳游戏列表
    lv_obj_t* scroll_cont = lv_obj_create(screen_game_);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    lv_obj_set_size(scroll_cont, LV_HOR_RES - 18, LV_VER_RES - 80);
    lv_obj_align(scroll_cont, LV_ALIGN_TOP_MID, 0, 50);
#else
    lv_obj_set_size(scroll_cont, LV_HOR_RES - 30, LV_VER_RES - 100);
    lv_obj_align(scroll_cont, LV_ALIGN_TOP_MID, 0, 60);
#endif
    lv_obj_set_style_bg_opa(scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(scroll_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(scroll_cont, 10, 0);
    lv_obj_set_style_pad_row(scroll_cont, 12, 0);
    // 不显示滚动条
    lv_obj_set_scrollbar_mode(scroll_cont, LV_SCROLLBAR_MODE_OFF);

    for (int i = 0; i < game_screens_.size(); i++) {
        // 创建游戏卡片容器
        lv_obj_t* game_card = lv_obj_create(scroll_cont);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
        lv_obj_set_size(game_card, LV_HOR_RES - 44, 70);
#else
        lv_obj_set_size(game_card, LV_HOR_RES - 50, 80);
#endif
        lv_obj_set_style_bg_color(game_card, CUTE_WHITE, 0);
        lv_obj_set_style_bg_opa(game_card, 255, 0);
        lv_obj_set_style_radius(game_card, 20, 0);
        lv_obj_set_style_shadow_width(game_card, 15, 0);
        lv_obj_set_style_shadow_color(game_card, CUTE_PINK_LIGHT, 0);
        lv_obj_set_style_shadow_opa(game_card, 120, 0);
        lv_obj_set_style_border_width(game_card, 2, 0);
        lv_obj_set_style_border_color(game_card, CUTE_PINK_LIGHT, 0);
        lv_obj_set_flex_flow(game_card, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(game_card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(game_card, 15, 0);

        // 左侧选择按钮
        lv_obj_t* select_btn = lv_btn_create(game_card);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
        lv_obj_set_size(select_btn, 35, 35);
#else
        lv_obj_set_size(select_btn, 40, 40);
#endif
        lv_obj_set_style_radius(select_btn, 20, 0);
        lv_obj_set_style_bg_color(select_btn, CUTE_PINK_PRIMARY, 0);
        lv_obj_set_style_shadow_width(select_btn, 8, 0);
        lv_obj_set_style_shadow_color(select_btn, CUTE_PINK_DARK, 0);
        lv_obj_set_style_shadow_opa(select_btn, 150, 0);
        lv_obj_add_event_cb(select_btn, game_select_button_cb, LV_EVENT_CLICKED, (void*)i);

        lv_obj_t * game_name = lv_label_create(game_card);
        lv_obj_set_style_text_font(game_name, fonts_.text_font, 0);
        lv_label_set_text(game_name, game_screens_[i].title.c_str());
        lv_obj_set_style_text_color(game_name, CUTE_PINK_DARK, 0);
        lv_obj_align(game_name, LV_ALIGN_LEFT_MID, 0, 0);

    }

    // 底部提示信息
    lv_obj_t* hint_label = lv_label_create(screen_game_);
    lv_obj_set_style_text_font(hint_label, fonts_.text_font, 0);
    lv_label_set_text(hint_label, "点击游戏卡片开始冒险吧！");
    lv_obj_set_style_text_color(hint_label, CUTE_PINK_LIGHT, 0);
    lv_obj_set_style_text_font(hint_label, fonts_.text_font, 0);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -15);
#else
    lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -20);
#endif
}
