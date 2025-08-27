# Gezipai 板子显示适配说明

## 概述
Gezipai 板子使用 240x280 分辨率的屏幕，相比原有的 320x240 屏幕，宽度减少了80像素，高度增加了40像素。为了保持良好的用户体验，我们对显示布局进行了相应的调整。

## 屏幕规格
- **分辨率**: 240 x 280
- **方向**: 纵向（Portrait）
- **触摸**: 支持触摸输入
- **接口**: SPI接口

## 布局调整

### 1. 物品商店界面
- **标题位置**: 从 `-4` 调整为 `-2`，适应新的屏幕高度
- **列表容器**: 从 `-10` 调整为 `-8`，优化垂直空间利用
- **物品间距**: 从 `15px` 调整为 `12px`，在较小宽度下保持合适间距

### 2. 物品项布局
- **信息容器宽度**: 从 `70%` 调整为 `75%`，充分利用可用宽度
- **按钮容器宽度**: 从 `30%` 调整为 `25%`，为信息显示留出更多空间
- **按钮高度**: 从 `40px` 调整为 `35px`，适应较小的屏幕尺寸

### 3. 字体和图标
- 保持原有的字体大小，确保文字清晰可读
- 图标尺寸适当调整，保持视觉平衡

## 技术实现

### 条件编译
使用 `CONFIG_BOARD_TYPE_GEZIPAI` 宏来区分不同的板子类型：

```cpp
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕的布局参数
    #define ITEM_SPACING_SMALL     12
    #define ITEM_HEIGHT_SMALL      70
    #define BUTTON_HEIGHT_SMALL    35
    #define TITLE_OFFSET_SMALL     -2
    #define LIST_OFFSET_SMALL      -8
#else
    // 320x240 屏幕的布局参数（保持原有）
    #define ITEM_SPACING_SMALL     ITEM_SPACING
    #define ITEM_HEIGHT_SMALL      ITEM_HEIGHT
    #define BUTTON_HEIGHT_SMALL    40
    #define TITLE_OFFSET_SMALL     -4
    #define LIST_OFFSET_SMALL      -10
#endif
```

### 动态布局
在 `CreateItem` 函数中根据板子类型动态调整布局参数：

```cpp
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    // 240x280 屏幕：调整信息容器宽度比例
    lv_obj_set_size(info_cont, LV_PCT(75), LV_SIZE_CONTENT);
    lv_obj_set_size(btn_cont, LV_PCT(25), LV_SIZE_CONTENT);
#else
    // 320x240 屏幕：保持原有比例
    lv_obj_set_size(info_cont, LV_PCT(70), LV_SIZE_CONTENT);
    lv_obj_set_size(btn_cont, LV_PCT(30), LV_SIZE_CONTENT);
#endif
```

## 兼容性
- **向后兼容**: 原有的 320x240 屏幕布局保持不变
- **向前兼容**: 新的 240x280 屏幕自动使用优化后的布局
- **代码维护**: 通过条件编译保持代码的清晰性和可维护性

## 注意事项
1. 确保在编译时正确设置 `CONFIG_BOARD_TYPE_GEZIPAI`
2. 新的布局参数已经过测试，确保在各种内容长度下都能正常显示
3. 触摸区域已经相应调整，保持良好的用户体验

## 未来扩展
如果后续需要支持更多屏幕尺寸，可以：
1. 添加新的条件编译分支
2. 定义相应的布局常量
3. 在布局函数中添加对应的逻辑 