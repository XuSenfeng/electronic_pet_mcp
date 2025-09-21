#include <string>
#include "core/lv_obj_pos.h"
#include "font/lv_symbol_def.h"
#include "lcd_display.h"
#include "display.h"
#include "application.h"
#include "electronic_pet.h"
#include "electronic_mqtt.h"
#define TAG "LcdDisplay"
#include "board.h"

// 清空消息列表的回调函数
static void clear_messages_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 获取LcdDisplay实例
        auto display = Board::GetInstance().GetDisplay();
        if (display != nullptr) {
            // 获取电子宠物实例并清空消息列表
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet != nullptr) {
                // 获取MQTT客户端并清空消息列表
                PMQTT_Clinet* mqtt_client = pet->GetClient();
                if (mqtt_client != nullptr) {
                    mqtt_client->ClearMessageList();
                    // 刷新消息界面
                    display->MessageUI();
                    // 显示新创建的消息界面
                    if (display->screen_game_ != nullptr) {
                        lv_obj_clear_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }
        }
    }
}

// 返回主界面的回调函数
static void back_to_main_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // 获取LcdDisplay实例
        auto display = Board::GetInstance().GetDisplay();
        if (display != nullptr) {
            // 隐藏消息界面，显示主界面
            lv_obj_add_flag(display->screen_game_, LV_OBJ_FLAG_HIDDEN);
            lv_obj_del(display->screen_game_);
            display->screen_game_ = nullptr;
            lv_obj_clear_flag(display->screen_main_, LV_OBJ_FLAG_HIDDEN);
            display->screen_now_ = display->screen_main_;
        }
    }
}

void LcdDisplay::MessageUI()
{
    DisplayLockGuard lock(this);
    
    // 如果消息界面已存在，先删除
    if (screen_game_ != nullptr) {
        lv_obj_del(screen_game_);
        screen_game_ = nullptr;
    }
    
    // 创建消息界面屏幕
    screen_game_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_game_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_game_, CUTE_WHITE, 0);
    lv_obj_clear_flag(screen_game_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(screen_game_, LV_OBJ_FLAG_HIDDEN);

    // 创建主容器（垂直布局）
    lv_obj_t* main_cont = lv_obj_create(screen_game_);
    lv_obj_set_size(main_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
#ifdef CONFIG_BOARD_TYPE_XVSENFAI
    // 240x280 屏幕：减少边框，最大化内容区域
    lv_obj_set_style_pad_all(main_cont, CONTAINER_PADDING_SMALL, 0);
#else
    // 320x240 屏幕：保持原有内边距
    lv_obj_set_style_pad_all(main_cont, 15, 0);
#endif
    lv_obj_set_style_pad_row(main_cont, 8, 0);
    lv_obj_remove_style(main_cont, NULL, LV_PART_SCROLLBAR);
    lv_obj_set_style_bg_opa(main_cont, LV_OPA_TRANSP, 0);

    // 创建标题栏
    lv_obj_t* title_bar = lv_obj_create(main_cont);
    lv_obj_set_size(title_bar, LV_PCT(100), 45);
    lv_obj_set_style_radius(title_bar, 20, 0);
    lv_obj_set_style_bg_color(title_bar, CUTE_PINK_LIGHT, 0);
    lv_obj_set_style_text_color(title_bar, CUTE_PINK_DARK, 0);
    lv_obj_set_style_text_font(title_bar, fonts_.text_font, 0);
    lv_obj_set_style_pad_all(title_bar, 10, 0);
    lv_obj_set_style_shadow_width(title_bar, 10, 0);
    lv_obj_set_style_shadow_color(title_bar, CUTE_PINK_PRIMARY, 0);
    lv_obj_set_style_shadow_opa(title_bar, 80, 0);
    lv_obj_set_flex_flow(title_bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(title_bar, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(title_bar, LV_SCROLLBAR_MODE_OFF);
    // 清空按钮
    lv_obj_t* clear_btn = lv_btn_create(title_bar);
    lv_obj_set_size(clear_btn, 35, 35);
    lv_obj_set_style_radius(clear_btn, 18, 0);
    lv_obj_set_style_bg_color(clear_btn, CUTE_WHITE, 0);
    lv_obj_set_style_shadow_width(clear_btn, 8, 0);
    lv_obj_set_style_shadow_color(clear_btn, CUTE_PINK_PRIMARY, 0);
    lv_obj_set_style_shadow_opa(clear_btn, 100, 0);
    lv_obj_add_event_cb(clear_btn, clear_messages_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* clear_label = lv_label_create(clear_btn);
    lv_label_set_text(clear_label, LV_SYMBOL_TRASH);
    lv_obj_set_style_text_color(clear_label, CUTE_PINK_DARK, 0);
    lv_obj_align(clear_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(clear_label, fonts_.text_font, 0);
    lv_label_set_text(clear_label, "清空");

    // 标题
    lv_obj_t* title_label = lv_label_create(title_bar);
    lv_label_set_text(title_label, "消息通知");
    lv_obj_set_style_text_color(title_label, CUTE_PINK_DARK, 0);
    lv_obj_set_style_text_font(title_label, fonts_.text_font, 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);

    // 返回按钮
    lv_obj_t* back_btn = lv_btn_create(title_bar);
    lv_obj_set_size(back_btn, 35, 35);
    lv_obj_set_style_radius(back_btn, 18, 0);
    lv_obj_set_style_bg_color(back_btn, CUTE_WHITE, 0);
    lv_obj_set_style_shadow_width(back_btn, 8, 0);
    lv_obj_set_style_shadow_color(back_btn, CUTE_PINK_PRIMARY, 0);
    lv_obj_set_style_shadow_opa(back_btn, 100, 0);
    lv_obj_add_event_cb(back_btn, back_to_main_cb, LV_EVENT_CLICKED, this);

    lv_obj_t* back_label = lv_label_create(back_btn);
    lv_obj_set_style_text_color(back_label, CUTE_PINK_DARK, 0);
    lv_obj_align(back_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(back_label, fonts_.text_font, 0);
    lv_label_set_text(back_label, "返回");

    // 创建消息列表滚动容器
    lv_obj_t* scroll_cont = lv_obj_create(main_cont);
#ifdef CONFIG_BOARD_TYPE_XVSENFAI
    // 240x280 屏幕：调整滚动容器大小
    lv_obj_set_size(scroll_cont, LV_PCT(100), LV_PCT(85));
#else
    // 320x240 屏幕：保持原有大小
    lv_obj_set_size(scroll_cont, LV_PCT(100), LV_PCT(80));
#endif
    lv_obj_set_style_bg_opa(scroll_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(scroll_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scroll_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(scroll_cont, 8, 0);
    lv_obj_set_style_pad_row(scroll_cont, 10, 0);
    // 启用滚动
    lv_obj_set_scrollbar_mode(scroll_cont, LV_SCROLLBAR_MODE_AUTO);

    // 获取消息列表并创建消息卡片
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if (pet != nullptr) {
        PMQTT_Clinet* mqtt_client = pet->GetClient();
        if (mqtt_client != nullptr && mqtt_client->message_list_.size() > 0) {
            // 获取真实的消息列表
            const std::vector<std::string>& messages = mqtt_client->message_list_;
            
            // 创建消息卡片
            for (size_t i = 0; i < messages.size(); i++) {
                const std::string& message = messages[i];
                
                // 解析消息格式 "用户名:消息内容"
                size_t colon_pos = message.find(':');
                std::string username = "未知用户";
                std::string message_content = message;
                
                if (colon_pos != std::string::npos) {
                    username = message.substr(0, colon_pos);
                    message_content = message.substr(colon_pos + 1);
                }
                
                // 创建消息卡片容器
                lv_obj_t* message_card = lv_obj_create(scroll_cont);
#ifdef CONFIG_BOARD_TYPE_XVSENFAI
                lv_obj_set_size(message_card, LV_HOR_RES - 30, 50);
#else
                lv_obj_set_size(message_card, LV_HOR_RES - 30, 60);
#endif
                lv_obj_set_style_bg_color(message_card, CUTE_WHITE, 0);
                lv_obj_set_style_bg_opa(message_card, 255, 0);
                lv_obj_set_style_radius(message_card, 20, 0);
                lv_obj_set_style_shadow_width(message_card, 12, 0);
                lv_obj_set_style_shadow_color(message_card, CUTE_PINK_LIGHT, 0);
                lv_obj_set_style_shadow_opa(message_card, 100, 0);
                lv_obj_set_style_border_width(message_card, 2, 0);
                lv_obj_set_style_border_color(message_card, CUTE_PINK_LIGHT, 0);
                lv_obj_set_flex_flow(message_card, LV_FLEX_FLOW_ROW);
                lv_obj_set_flex_align(message_card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
                lv_obj_set_style_pad_all(message_card, 12, 0);
                lv_obj_set_scrollbar_mode(message_card, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_width(message_card, LV_PCT(95));
                           

                // 消息内容容器
                lv_obj_t* content_cont = lv_obj_create(message_card);
                lv_obj_set_flex_grow(content_cont, 1);
                lv_obj_set_style_bg_opa(content_cont, LV_OPA_TRANSP, 0);
                lv_obj_set_style_border_width(content_cont, 0, 0);
                lv_obj_set_style_pad_all(content_cont, 0, 0);
                lv_obj_set_style_pad_left(content_cont, 10, 0);
                // 禁用滚动条
                lv_obj_set_scrollbar_mode(content_cont, LV_SCROLLBAR_MODE_OFF);
                // 不可以滚动
                lv_obj_clear_flag(content_cont, LV_OBJ_FLAG_SCROLLABLE);
                

                // 用户名，放在最上边一行
                lv_obj_t* name_label = lv_label_create(content_cont);
                lv_label_set_text(name_label, username.c_str());
                lv_obj_set_style_text_color(name_label, CUTE_PINK_DARK, 0);
                lv_obj_set_style_text_font(name_label, fonts_.text_font, 0);
                lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_LEFT, 0);
                lv_obj_align(name_label, LV_ALIGN_TOP_LEFT, 0, 43); // 设置在最上边一行
                

                // 消息内容
                lv_obj_t* message_label = lv_label_create(content_cont);
                lv_label_set_text(message_label, message_content.c_str());
                lv_obj_set_style_text_color(message_label, current_theme_.text, 0);
                lv_obj_set_style_text_font(message_label, fonts_.text_font, 0);
                lv_obj_set_style_text_align(message_label, LV_TEXT_ALIGN_LEFT, 0);
                lv_label_set_long_mode(message_label, LV_LABEL_LONG_WRAP);
                lv_obj_align_to(message_label, name_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5); // 设置在用户名下边
            }
        } else {
            // 如果没有消息，显示空状态
            lv_obj_t* empty_label = lv_label_create(scroll_cont);
            lv_label_set_text(empty_label, "暂无消息通知\n\n等待其他用户发送消息...");
            lv_obj_set_style_text_color(empty_label, CUTE_PURPLE, 0);
            lv_obj_set_style_text_font(empty_label, fonts_.text_font, 0);
            lv_obj_set_style_text_align(empty_label, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(empty_label, LV_ALIGN_CENTER, 0, 0);
        }
    } else {
        // 如果无法获取宠物实例，显示错误状态
        lv_obj_t* error_label = lv_label_create(scroll_cont);
        lv_label_set_text(error_label, "无法加载消息\n\n请检查系统状态");
        lv_obj_set_style_text_color(error_label, CUTE_PINK_DARK, 0);
        lv_obj_set_style_text_font(error_label, fonts_.text_font, 0);
        lv_obj_set_style_text_align(error_label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(error_label, LV_ALIGN_CENTER, 0, 0);
    }
}
