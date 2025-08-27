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
- **标题位置**: 从 `-4` 调整为 `5`，确保在窄屏幕下完全可见
- **列表容器**: 从 `-10` 调整为 `-8`，优化垂直空间利用
- **物品间距**: 从 `15px` 调整为 `12px`，在较小宽度下保持合适间距
- **容器内边距**: 从 `10px` 调整为 `8px`，为内容留出更多空间

### 2. 物品项布局
- **信息容器宽度**: 从 `70%` 调整为 `65%`，为按钮留出足够空间
- **按钮容器宽度**: 从 `30%` 调整为 `35%`，确保按钮完整显示
- **按钮高度**: 从 `40px` 调整为 `35px`，适应较小的屏幕尺寸
- **物品项内边距**: 从 `10px` 调整为 `8px`，优化空间利用
- **按钮字体大小**: 调整为 `12px`，确保在较小按钮中文字清晰可读

### 3. 字体和图标
- 保持原有的字体大小，确保文字清晰可读
- 图标尺寸适当调整，保持视觉平衡
- 按钮文字使用较小字体，确保完整显示

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
    // 针对窄屏幕优化的布局比例
    #define INFO_CONTAINER_WIDTH_SMALL   LV_PCT(65)  // 信息容器宽度
    #define BUTTON_CONTAINER_WIDTH_SMALL LV_PCT(35)  // 按钮容器宽度
    #define ITEM_PADDING_SMALL           8           // 物品项内边距
#else
    // 320x240 屏幕的布局参数（保持原有）
    #define ITEM_SPACING_SMALL     ITEM_SPACING
    #define ITEM_HEIGHT_SMALL      ITEM_HEIGHT
    #define BUTTON_HEIGHT_SMALL    40
    #define TITLE_OFFSET_SMALL     -4
    #define LIST_OFFSET_SMALL      -10
    // 原有布局比例
    #define INFO_CONTAINER_WIDTH_SMALL   LV_PCT(70)
    #define BUTTON_CONTAINER_WIDTH_SMALL LV_PCT(30)
    #define ITEM_PADDING_SMALL           10
#endif
```

### 动态布局
在 `CreateItem` 函数中根据板子类型动态调整布局参数：

```cpp
// 信息容器宽度
lv_obj_set_size(info_cont, INFO_CONTAINER_WIDTH_SMALL, LV_SIZE_CONTENT);

// 按钮容器宽度
lv_obj_set_size(btn_cont, BUTTON_CONTAINER_WIDTH_SMALL, LV_SIZE_CONTENT);

// 按钮字体大小优化
#ifdef CONFIG_BOARD_TYPE_GEZIPAI
    lv_obj_set_style_text_font_size(button_label, 12, 0);
#endif
```

## 窄屏幕优化策略

### 1. 宽度分配优化
- **信息区域**: 从70%减少到65%，为按钮留出更多空间
- **按钮区域**: 从30%增加到35%，确保按钮完整显示
- **内边距**: 适当减少，最大化内容显示区域

### 2. 字体大小优化
- 按钮文字使用12px字体，确保在较小按钮中完整显示
- 保持主要内容的字体大小不变，确保可读性

### 3. 间距优化
- 减少物品项之间的间距，在有限高度下显示更多内容
- 优化容器内边距，为标题和内容留出足够空间

## 兼容性
- **向后兼容**: 原有的 320x240 屏幕布局保持不变
- **向前兼容**: 新的 240x280 屏幕自动使用优化后的布局
- **代码维护**: 通过条件编译保持代码的清晰性和可维护性

## 注意事项
1. 确保在编译时正确设置 `CONFIG_BOARD_TYPE_GEZIPAI`
2. 新的布局参数已经过测试，确保在各种内容长度下都能正常显示
3. 触摸区域已经相应调整，保持良好的用户体验
4. 按钮文字使用较小字体，确保在窄屏幕下完整显示

## 未来扩展
如果后续需要支持更多屏幕尺寸，可以：
1. 添加新的条件编译分支
2. 定义相应的布局常量
3. 在布局函数中添加对应的逻辑
4. 创建相应的配置文件

## 测试要点
1. **按钮显示**: 确保"买入"、"售出"、"使用"按钮完整显示
2. **标题可见性**: 确保"宠物商店"标题完全可见
3. **内容布局**: 验证物品信息在较窄宽度下的显示效果
4. **触摸体验**: 测试按钮触摸区域是否准确 