#include "lcd_display.h"

#include <vector>
#include <font_awesome_symbols.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_lvgl_port.h>
#include "assets/lang_config.h"
#include <cstring>
#include "settings.h"
#include "application.h"
#include "board.h"
#include "electronic_pet.h"

#define TAG "LcdDisplay"


void LcdDisplay::HelpUI() {
    DisplayLockGuard lock(this);
    screen_description_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_description_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_description_, lv_color_hex(0xFFF5F5), 0);
    lv_obj_add_flag(screen_description_, LV_OBJ_FLAG_HIDDEN);
    
    // 标题栏
    lv_obj_t* title = lv_label_create(screen_description_);
    lv_label_set_text(title, "宠物指南");
    lv_obj_set_style_text_font(title, fonts_.text_font, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFF88A4), 0);
    
    // 滚动容器
    lv_obj_t* scroll_cont = lv_obj_create(screen_description_);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：减少边框，最大化内容区域
    lv_obj_set_size(scroll_cont, LV_PCT(100), LV_PCT(88));
    lv_obj_set_style_pad_all(scroll_cont, 8, 0);
    lv_obj_set_style_pad_row(scroll_cont, 12, 0);
#else
    // 320x240 屏幕：保持原有设置
    lv_obj_set_size(scroll_cont, LV_PCT(100), LV_PCT(85));
    lv_obj_set_style_pad_all(scroll_cont, 15, 0);
    lv_obj_set_style_pad_row(scroll_cont, 20, 0);
#endif
    lv_obj_align(scroll_cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_flex_flow(scroll_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_remove_style(scroll_cont, NULL, LV_PART_SCROLLBAR);

    // 创建说明卡片
    CreateCard(scroll_cont, 0, LV_SYMBOL_HOME, "欢迎来到宠物世界", 
        "这里住着可爱的电子伙伴\n"
        "通过互动培养你们的感情\n"
        "保持它的健康快乐成长吧~");
    
    CreateCard(scroll_cont, 1, LV_SYMBOL_AUDIO, "核心玩法", 
        "• 喂食  保持饱食度\n"
        "• 游戏  提升快乐指数\n"
        "• 商店  购买道具物品\n"
        "• 升级  提升宠物的属性\n");
    
    CreateCard(scroll_cont, 2, LV_SYMBOL_EYE_OPEN, "状态说明",
        "经验值：在经验值达到一定阈值以后可以升级, 升级会提示宠物的状态\n"
        "心情值：互动频率影响\n"
        "饱食度：随时间自然消耗, 为0的时候会寄寄\n"
        "金钱：打工获取用于购买\n"
        "IQ：可以提升工作的工资");
    
    CreateCard(scroll_cont, 3, LV_SYMBOL_KEYBOARD, "操作指南",
        "← 状态界面\n"
        "→ 商店界面\n"
        "↓ 说明界面\n"
        "↑ 设置界面");

    CreateCard(scroll_cont, 4, LV_SYMBOL_KEYBOARD, "主要行为",
        "• 空闲:各项数值缓慢下降\n"
        "• 玩耍:游戏时候处于的状态,不可以主动切换\n"
        "• 睡觉:精力恢复,饱食度缓慢下降\n"
        "• 走路:精力饱食度下降,心情变好\n"
        "• 工作:精力心情快速下降,饱食度下降,金钱增加\n"
        "• 洗澡:精力饱食度缓慢下降,心情变好\n"
        "• 听歌:精力饱食度缓慢下降,心情变好\n");

    CreateCard(scroll_cont, 5, LV_SYMBOL_KEYBOARD, "升级",
        "数值大于75的时候经验会缓慢增长,"
        "当经验的数值达到一定阈值的时候可以对话触发升级事件"
        "不同等级的时候会有不同的升级任务"
        "实现任务以后可以升级\n");

    CreateCard(scroll_cont, 6, LV_SYMBOL_KEYBOARD, "作者",
        "• github: XuSenfeng\n"
        "• B站: XvSenfeng\n"
        "• 邮箱: helloworldjiao@163.com\n"
        "• 注: 当前为免费测试版本,在粉丝群内部测试\n");

}



/* 创建统一风格的说明卡片 */
void LcdDisplay::CreateCard(lv_obj_t* parent, int index, const char* icon, 
    const char* title, const char* content) 
{
// 卡片容器
cards[index].card = lv_obj_create(parent);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：减少边框，最大化内容区域
    lv_obj_set_size(cards[index].card, (LV_HOR_RES - 20), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(cards[index].card, 8, 0);
#else
    // 320x240 屏幕：保持原有设置
    lv_obj_set_size(cards[index].card, (LV_HOR_RES - 60), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(cards[index].card, 12, 0);
#endif
lv_obj_set_style_radius(cards[index].card, 15, 0);
lv_obj_set_style_bg_color(cards[index].card, lv_color_hex(0xFFFFFF), 0);
lv_obj_set_style_shadow_width(cards[index].card, 15, 0);
lv_obj_set_flex_flow(cards[index].card, LV_FLEX_FLOW_ROW);

// 图标区域
lv_obj_t* icon_cont = lv_obj_create(cards[index].card);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整图标尺寸
    lv_obj_set_size(icon_cont, 30, 30);
#else
    // 320x240 屏幕：保持原有尺寸
    lv_obj_set_size(icon_cont, 35, 35);
#endif
lv_obj_set_style_bg_opa(icon_cont, LV_OPA_TRANSP, 0);
lv_obj_remove_style(icon_cont, NULL, LV_PART_SCROLLBAR);

cards[index].icon = lv_label_create(icon_cont);
lv_label_set_text(cards[index].icon, icon);
// lv_obj_set_style_text_font(cards[index].icon, fonts_.text_font, 0);
lv_obj_set_style_text_color(cards[index].icon, lv_color_hex(0xFF88A4), 0);
lv_obj_center(cards[index].icon);

// 内容区域
lv_obj_t* text_cont = lv_obj_create(cards[index].card);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整内容区域宽度
    lv_obj_set_size(text_cont, LV_PCT(85), LV_SIZE_CONTENT);
#else
    // 320x240 屏幕：保持原有宽度
    lv_obj_set_size(text_cont, LV_PCT(80), LV_SIZE_CONTENT);
#endif
lv_obj_set_flex_flow(text_cont, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_all(text_cont, 10, 0);

// 标题
cards[index].title = lv_label_create(text_cont);
lv_label_set_text(cards[index].title, title);
lv_obj_set_style_text_font(cards[index].title, fonts_.text_font, 0);

// 分隔线
lv_obj_t* line = lv_line_create(text_cont);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整分隔线长度
    static lv_point_precise_t points_small[] = {{0,0}, {120,0}};
    lv_line_set_points(line, points_small, 2);
#else
    // 320x240 屏幕：保持原有长度
    static lv_point_precise_t points[] = {{0,0}, {160,0}};
    lv_line_set_points(line, points, 2);
#endif
lv_obj_set_style_line_width(line, 2, 0);
lv_obj_set_style_line_color(line, lv_color_hex(0xFFE4EB), 0);

// 内容
cards[index].content = lv_label_create(text_cont);
lv_label_set_text(cards[index].content, content);
lv_obj_set_style_text_color(cards[index].content, lv_color_hex(0x666666), 0);
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整内容宽度
    lv_obj_set_width(cards[index].content, LV_PCT(85));
#else
    // 320x240 屏幕：保持原有宽度
    lv_obj_set_width(cards[index].content, LV_PCT(80));
#endif
lv_label_set_long_mode(cards[index].content, LV_LABEL_LONG_WRAP);
lv_obj_set_style_text_font(cards[index].content, fonts_.text_font, 0);
}
