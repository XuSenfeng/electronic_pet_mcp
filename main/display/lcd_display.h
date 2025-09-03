#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "display.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <font_emoji.h>

#include <atomic>

// Theme color structure
struct ThemeColors {
    lv_color_t background;
    lv_color_t text;
    lv_color_t chat_background;
    lv_color_t user_bubble;
    lv_color_t assistant_bubble;
    lv_color_t system_bubble;
    lv_color_t system_text;
    lv_color_t border;
    lv_color_t low_battery;
};
#define ITEM_SPACING        15
#define ITEM_HEIGHT         80
#define ITEM_WIDTH          (LV_HOR_RES - 2*20)
#define ICON_SIZE           48
#define PROGRESS_WIDTH      150

// 屏幕尺寸相关的布局常量
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕的布局参数
    #define ITEM_SPACING_SMALL     12
    #define ITEM_HEIGHT_SMALL      70
    #define ITEM_WIDTH_SMALL       (LV_HOR_RES - 2*10)
    #define ICON_SIZE_SMALL        40
    #define PROGRESS_WIDTH_SMALL   100
    #define BUTTON_HEIGHT_SMALL    35
    #define TITLE_OFFSET_SMALL     -2
    #define LIST_OFFSET_SMALL      -8
    // 针对窄屏幕优化的布局比例
    #define INFO_CONTAINER_WIDTH_SMALL   LV_PCT(65)  // 信息容器宽度
    #define BUTTON_CONTAINER_WIDTH_SMALL LV_PCT(35)  // 按钮容器宽度
    #define ITEM_PADDING_SMALL           6           // 物品项内边距
    // 边框优化常量
    #define CONTAINER_PADDING_SMALL     10           // 主容器内边距
    #define CARD_PADDING_SMALL          8            // 卡片内边距
    #define SCROLL_PADDING_SMALL        8            // 滚动容器内边距
#else
    // 320x240 屏幕的布局参数（保持原有）
    #define ITEM_SPACING_SMALL     ITEM_SPACING
    #define ITEM_HEIGHT_SMALL      ITEM_HEIGHT
    #define ITEM_WIDTH_SMALL       ITEM_WIDTH
    #define ICON_SIZE_SMALL        ICON_SIZE
    #define PROGRESS_WIDTH_SMALL   PROGRESS_WIDTH
    #define BUTTON_HEIGHT_SMALL    40
    #define TITLE_OFFSET_SMALL     -4
    #define LIST_OFFSET_SMALL      -10
    // 原有布局比例
    #define INFO_CONTAINER_WIDTH_SMALL   LV_PCT(70)
    #define BUTTON_CONTAINER_WIDTH_SMALL LV_PCT(30)
    #define ITEM_PADDING_SMALL           10
#endif

// 说明卡片结构体
typedef struct {
    lv_obj_t* card;
    lv_obj_t* icon;
    lv_obj_t* title;
    lv_obj_t* content;
} HelpCard;
class LcdDisplay : public Display {
protected:
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    
    lv_draw_buf_t draw_buf_;
    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;
    lv_obj_t* preview_image_ = nullptr;
    lv_obj_t* action_label_ = nullptr;
    lv_obj_t* main_btn;
    lv_obj_t* desc_label;
    HelpCard cards[9]; // 根据实际版块数量调整

    
    DisplayFonts fonts_;
    lv_obj_t** item_cont; // 保存物品容器
    ThemeColors current_theme_;

    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

protected:
    // 添加protected构造函数
    LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, DisplayFonts fonts, int width, int height);
    
public:
    ~LcdDisplay();
    virtual void SetEmotion(const char* emotion) override;
    virtual void SetIcon(const char* icon) override;
    virtual void SetPreviewImage(const lv_img_dsc_t* img_dsc) override; 



    void SetupUI() override;
    void StateUI() override;
    void ItemUI() override;
    void HelpUI() override;
    void AIPlayGameUI() override;
    void GameSelectUI() override;
    void MessageUI() override;

    void UpdateStateGui() override;
    void UpdateItemUI() override;
    void AiStoryUI() override;
    void CreateItem(lv_obj_t* parent, int index);
    void UpdateThingsCount(int index);
    void CreateCard(lv_obj_t* parent, int index, const char* icon, 
        const char* title, const char* content);
    void CreateStatusItem(lv_obj_t* parent, const char* name, int value_ptr, 
        int min, int max, int y_pos, int num) override;
    void UpdateStoryHistory(const char* new_text) override;
    void UpdateGameStateGui() override;
    void StateItemCreate(lv_obj_t *parent, int i);
    void UpdataUILevel(int level) override;

};

// RGB LCD显示器
class RgbLcdDisplay : public LcdDisplay {
public:
    RgbLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

// MIPI LCD显示器
class MipiLcdDisplay : public LcdDisplay {
public:
    MipiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

// // SPI LCD显示器
class SpiLcdDisplay : public LcdDisplay {
public:
    SpiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                  int width, int height, int offset_x, int offset_y,
                  bool mirror_x, bool mirror_y, bool swap_xy,
                  DisplayFonts fonts);
};

// QSPI LCD显示器
class QspiLcdDisplay : public LcdDisplay {
public:
    QspiLcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                   int width, int height, int offset_x, int offset_y,
                   bool mirror_x, bool mirror_y, bool swap_xy,
                   DisplayFonts fonts);
};

// MCU8080 LCD显示器
class Mcu8080LcdDisplay : public LcdDisplay {
public:
    Mcu8080LcdDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                      int width, int height, int offset_x, int offset_y,
                      bool mirror_x, bool mirror_y, bool swap_xy,
                      DisplayFonts fonts);
};



// Color definitions for light theme - 可爱桌宠风格
#define LIGHT_BACKGROUND_COLOR       lv_color_hex(0xFFF0F5)     // 浅粉色背景
#define LIGHT_TEXT_COLOR             lv_color_hex(0x4A4A4A)     // 深灰色文字
#define LIGHT_CHAT_BACKGROUND_COLOR  lv_color_hex(0xFFF8FA)     // 更浅的粉色背景
#define LIGHT_USER_BUBBLE_COLOR      lv_color_hex(0xFFB6C1)     // 浅粉色气泡
#define LIGHT_ASSISTANT_BUBBLE_COLOR lv_color_hex(0xFFFFFF)     // 白色气泡
#define LIGHT_SYSTEM_BUBBLE_COLOR    lv_color_hex(0xFFE4E1)     // 浅珊瑚色
#define LIGHT_SYSTEM_TEXT_COLOR      lv_color_hex(0x8B7D7B)     // 深灰色文字
#define LIGHT_BORDER_COLOR           lv_color_hex(0xFFC0CB)     // 浅粉色边框
#define LIGHT_LOW_BATTERY_COLOR      lv_color_hex(0xFF6B9D)     // 粉色警告

// 可爱桌宠风格专用颜色
#define CUTE_PINK_PRIMARY           lv_color_hex(0xFF88A4)      // 主粉色
#define CUTE_PINK_LIGHT             lv_color_hex(0xFFB6C1)      // 浅粉色
#define CUTE_PINK_DARK              lv_color_hex(0xFF69B4)      // 深粉色
#define CUTE_PURPLE                 lv_color_hex(0xDDA0DD)      // 浅紫色
#define CUTE_BLUE                   lv_color_hex(0x87CEEB)      // 天蓝色
#define CUTE_GREEN                  lv_color_hex(0x98FB98)      // 浅绿色
#define CUTE_YELLOW                 lv_color_hex(0xFFFACD)      // 浅黄色
#define CUTE_ORANGE                 lv_color_hex(0xFFB347)      // 橙色
#define CUTE_WHITE                  lv_color_hex(0xFFF8FA)      // 暖白色
#define CUTE_GRAY                   lv_color_hex(0xF5F5F5)      // 浅灰色
#endif // LCD_DISPLAY_H
