#include "lvgl.h"
#include "display.h"
#include "lcd_display.h"
#include "application.h"
#include "board.h"
#define TAG "LCDDisplay"
/* 物品界面创建函数 */
void LcdDisplay::ItemUI() {
    DisplayLockGuard lock(this);
    int num_of_things = ElectronicPet::GetInstance()->GetThingsNum();
    if(num_of_things == 0) {
        ESP_LOGE(TAG, "No things available");
        return;
    }
    item_cont = (lv_obj_t**)malloc(num_of_things * sizeof(lv_obj_t*));
    if(item_cont == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for item containers");
        return;
    }
    // 创建物品界面
    screen_things_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screen_things_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(screen_things_, lv_color_hex(0xFFF0F5), 0);
    lv_obj_add_flag(screen_things_, LV_OBJ_FLAG_HIDDEN);
    
    // 标题栏
    lv_obj_t* title = lv_label_create(screen_things_);
    lv_obj_set_style_text_font(title, fonts_.text_font, 0);
    lv_label_set_text(title, "宠物商店");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, -2);
    
    // 物品列表容器
    lv_obj_t* list = lv_obj_create(screen_things_);
    lv_obj_set_size(list, LV_PCT(95), LV_PCT(85));
    lv_obj_align(list, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(list, 10, 0);
    lv_obj_set_style_pad_row(list, ITEM_SPACING, 0);
    // lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLLABLE);
    
    ESP_LOGI(TAG, "商店 list: %d", num_of_things);
    // // 创建物品项
    for(int i=0; i<num_of_things; i++) {
        CreateItem(list, i);
    }
}


void use_btn_event_cb(lv_event_t* e) {
    // lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);
    if (index < 0) {
        ESP_LOGE(TAG, "Invalid index for use button");
        return;
    }
    ESP_LOGI(TAG, "Use button clicked for item %d", index);
    // 处理按钮点击事件
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    
    BaseThing *it = pet->GetThing(index);
    if(it == nullptr) {
        ESP_LOGE(TAG, "GetThing returned null");
        return;
    }
    if(it->GetNum() <= 0) {
    // 发送消息给小智
        std::string message = "商店提示:" + std::string(it->GetName()) + "数量不足";
        // 发送消息给小智的逻辑
        auto& app = Application::GetInstance();
        app.SendMessage(message);
        return;
    }
    
    // 使用物品
    it->Use();
    // 更新物品数量
    it->SetNum(it->GetNum() - 1);

    auto display = Board::GetInstance().GetDisplay();
    if(display == nullptr) {
        ESP_LOGE(TAG, "Display instance is null");
        return;
    }
    display->UpdateItemUI(); // 更新物品界面
}

void buy_btn_event_cb(lv_event_t* e) {
    // lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);
    if (index < 0) {
        ESP_LOGE(TAG, "Invalid index for buy button");
        return;
    }
    ESP_LOGI(TAG, "Buy button clicked for item %d", index);
    // 处理按钮点击事件
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    
    BaseThing *it = pet->GetThing(index);
    if(it == nullptr) {
        ESP_LOGE(TAG, "GetThing returned null");
        return;
    }
    if(pet->GetState(E_PET_STATE_MONEY) < it->GetMoney()) {
        // 发送消息给小智
        std::string message = "商店提示: 您想购买的" + std::string(it->GetName()) + "金钱不足";
        // 发送消息给小智的逻辑
        auto& app = Application::GetInstance();
        app.SendMessage(message);
        return;
    }
    // 购买物品
    pet->SetState(E_PET_STATE_MONEY, pet->GetState(E_PET_STATE_MONEY) - it->GetMoney());

    // 更新物品数量
    it->SetNum(it->GetNum() + 1);
    
    // 发送消息给小智
    std::string message = "商店提示:" + std::string(it->GetName()) + "购买成功, 已经放入背包";
    // 发送消息给小智的逻辑
    auto& app = Application::GetInstance();
    app.SendMessage(message);

    auto display = Board::GetInstance().GetDisplay();
    if(display == nullptr) {
        ESP_LOGE(TAG, "Display instance is null");
        return;
    }
    display->UpdateItemUI(); // 更新物品界面
}

void sell_btn_event_cb(lv_event_t* e) {
    // lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    int index = (int)lv_event_get_user_data(e);
    if (index < 0) {
        ESP_LOGE(TAG, "Invalid index for sell button");
        return;
    }
    ESP_LOGI(TAG, "Sell button clicked for item %d", index);
    // 处理按钮点击事件
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    
    BaseThing *it = pet->GetThing(index);
    if(it == nullptr) {
        ESP_LOGE(TAG, "GetThing returned null");
        return;
    }
    
    // 更新物品数量
    it->SetNum(it->GetNum() - 1);
    // 金钱增加, 但是只增加80%
    pet->SetState(E_PET_STATE_MONEY, pet->GetState(E_PET_STATE_MONEY) + it->GetMoney() * 0.8);
    
    // 发送消息给小智
    std::string message = "商店提示:" + std::string(it->GetName()) + "卖出成功";
    // 发送消息给小智的逻辑
    auto& app = Application::GetInstance();
    app.SendMessage(message);

    auto display = Board::GetInstance().GetDisplay();
    if(display == nullptr) {
        ESP_LOGE(TAG, "Display instance is null");
        return;
    }
    display->UpdateItemUI(); // 更新物品界面
}


/* 创建单个物品项（Flex布局版本） */
void LcdDisplay::CreateItem(lv_obj_t* parent, int index) {
    // Item* it = &items[index];
    ElectronicPet* pet = ElectronicPet::GetInstance();
    BaseThing *it = pet->GetThing(index);
    if(it == nullptr) {
        ESP_LOGE(TAG, "GetThing returned null");
        return;
    }
    
    // 主容器（横向Flex布局）
    item_cont[index] = lv_obj_create(parent);
    lv_obj_set_size(item_cont[index], LV_PCT(100), LV_SIZE_CONTENT); // 高度自适应
    lv_obj_set_flex_flow(item_cont[index], LV_FLEX_FLOW_ROW);       // 横向排列
    lv_obj_set_style_pad_all(item_cont[index], 10, 0);              // 统一内边距
    lv_obj_set_style_radius(item_cont[index], 15, 0);
    lv_obj_set_style_bg_color(item_cont[index], lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_shadow_width(item_cont[index], 10, 0);

    // 左侧信息容器（纵向Flex）
    lv_obj_t* info_cont = lv_obj_create(item_cont[index]);
    lv_obj_set_size(info_cont, LV_PCT(70), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(info_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(info_cont, 5, 0); // 行间距
    lv_obj_set_style_bg_opa(info_cont, LV_OPA_TRANSP, 0);

    // 名称（自动换行）
    lv_obj_t* name = lv_label_create(info_cont);
    lv_obj_set_style_text_font(name, fonts_.text_font, 0);
    lv_label_set_text_fmt(name, "%s ×%d", it->GetName(), it->GetNum());
    lv_obj_set_width(name, LV_PCT(100));
    lv_obj_set_style_text_align(name, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(name, LV_LABEL_LONG_WRAP);

    // 作用描述（自动换行）
    lv_obj_t* effect = lv_label_create(info_cont);
    lv_label_set_text(effect, it->GetDescription());
    lv_obj_set_width(effect, LV_PCT(100));
    lv_obj_set_style_text_font(effect, fonts_.text_font, 0);
    lv_label_set_long_mode(effect, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(effect, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(effect, lv_color_hex(0x666666), 0);

    // 数值容器
    lv_obj_t* value_cont = lv_obj_create(info_cont);
    lv_obj_set_size(value_cont, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(value_cont, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(value_cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_set_style_pad_column(value_cont, 5, 0);
    lv_obj_set_style_pad_row(info_cont, 5, 0); // 行间距

    // 价格
    lv_obj_t* price = lv_label_create(value_cont);
    lv_obj_set_style_text_font(price, fonts_.text_font, 0);
    lv_label_set_text_fmt(price, "价格：%dG", it->GetMoney());
    lv_obj_set_style_text_color(price, lv_color_hex(0xFF88A4), 0);
    lv_obj_set_style_text_align(price, LV_TEXT_ALIGN_CENTER, 0);

    // 属性值
    lv_obj_t* stats = lv_label_create(value_cont);
    lv_obj_set_style_text_font(value_cont, fonts_.text_font, 0);
    lv_label_set_text_fmt(stats, "精神 %d \n饱食度 %d\n 心情 %d\nIQ %d", it->GetVigor(), it->GetSatiety(), it->GetHappiness(), it->GetIq());
    lv_obj_set_style_text_color(stats, lv_color_hex(0xFF88A4), 0);
    lv_obj_set_style_text_align(stats, LV_TEXT_ALIGN_CENTER, 0);

    // 右侧按钮容器（纵向Flex）
    lv_obj_t* btn_cont = lv_obj_create(item_cont[index]);
    lv_obj_set_size(btn_cont, LV_PCT(30), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(btn_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn_cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(btn_cont, 10, 0); // 按钮间距

    // 购买按钮
    lv_obj_t* buy_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(buy_btn, LV_PCT(100), 40); // 宽度充满容器
    lv_obj_set_style_bg_color(buy_btn, lv_color_hex(0x98FB98), 0);
    lv_obj_set_style_radius(buy_btn, 20, 0);
    lv_obj_t* buy_label = lv_label_create(buy_btn);
    lv_obj_set_style_text_font(buy_label, fonts_.text_font, 0);
    lv_label_set_text(buy_label, "买入");
    lv_obj_align(buy_label, LV_ALIGN_CENTER, 0, 0);

    // 出售按钮
    lv_obj_t* sell_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(sell_btn, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(sell_btn, lv_color_hex(0xFFB6C1), 0);
    lv_obj_set_style_radius(sell_btn, 20, 0);
    lv_obj_t* sell_label = lv_label_create(sell_btn);
    lv_obj_set_style_text_font(sell_label, fonts_.text_font, 0);
    lv_label_set_text(sell_label, "售出");
    lv_obj_align(sell_label, LV_ALIGN_CENTER, 0, 0);

    // 使用按钮
    lv_obj_t* use_btn = lv_btn_create(btn_cont);
    lv_obj_set_size(use_btn, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(use_btn, lv_color_hex(0xdbeafe), 0);
    lv_obj_set_style_radius(use_btn, 20, 0);
    lv_obj_t* use_label = lv_label_create(use_btn);
    lv_obj_set_style_text_font(use_label, fonts_.text_font, 0);
    lv_label_set_text(use_label, "使用");
    lv_obj_align(use_label, LV_ALIGN_CENTER, 0, 0);

    lv_obj_add_event_cb(use_btn, use_btn_event_cb, LV_EVENT_CLICKED, (void*)index);
    lv_obj_add_event_cb(buy_btn, buy_btn_event_cb, LV_EVENT_CLICKED, (void*)index);
    lv_obj_add_event_cb(sell_btn, sell_btn_event_cb, LV_EVENT_CLICKED, (void*)index);
}

void LcdDisplay::UpdateThingsCount(int index) {
    lv_obj_t* cont = item_cont[index];
    lv_obj_t* name_label = lv_obj_get_child(lv_obj_get_child(cont, 0), 0);
    ElectronicPet* pet = ElectronicPet::GetInstance();
    ESP_LOGI(TAG, "UpdateThingsCount: %d %s, %d", index, pet->GetThing(index)->GetName(), pet->GetThing(index)->GetNum());
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    lv_label_set_text_fmt(name_label, "%s ×%d", 
        pet->GetThing(index)->GetName(), pet->GetThing(index)->GetNum());
}

void LcdDisplay::UpdateItemUI(){
    DisplayLockGuard lock(this);
    ElectronicPet* pet = ElectronicPet::GetInstance();
    if(pet == nullptr) {
        ESP_LOGE(TAG, "ElectronicPet instance is null");
        return;
    }
    int num_of_things = pet->GetThingsNum();
    for(int i = 0; i < num_of_things; i++) {
        UpdateThingsCount(i);
    }
}
