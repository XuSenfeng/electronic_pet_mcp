# XvSenfengAI桌宠

开源不易, 给个![0B533B9D](https://github.com/user-attachments/assets/83214bfa-0ffd-49ad-b87f-9b5c4d3ca938)吧

## 🎯 项目简介

XvSenfengAI桌宠是一个基于ESP32-S3的开源智能硬件项目，结合了人工智能、语音识别、自然语言处理等先进技术，通过MCP（Model Context Protocol）协议实现AI与硬件的深度整合，打造了一个具有情感交互能力的智能电子宠物。

> 基于小智AI二次开发

1. 利用小智原有的服务, 上手即用
2. 实现多设备的连接, 板子间, 和手机
3. 拓展一下娱乐属性
4. 压榨性能同时保持稳定

## ✨ 核心特性

基于小智AI进行二次开发, 包括但不限于实现以下部分的功能

+ 使用AI控制, 将AI集成到各种功能的控制中, 实现一个高度自定义的AI控制终端
+ 升级版自定义**闹钟提醒**功能, 可以实现精确定时提醒以及定时调用某些功能, 也可实现随机时间触发事件, 提高桌宠的交互性, 预留接口可自己拓展
+ **多设备联动**, 可以实现板子之间的通信(需要相互关注)以及手机和板子之间的联动, 搭建完善的AI物联网控制框架
+ 桌宠**养成系统**, 实现多种状态以及升级等功能的实现(目前功能打磨中)
+ **高度自定义**配套控制, 可以使用**微信小程序**, 也可以使用读取本地SD卡配置的方式, 实现高度用户自定义
+ 硬件(XvsenfengAI开发板)以及外壳全开源, 可自定义更改
+ **优化**原版小智的部分功能, 释放更多的flash
+ **完善的教程**, 后续推出完善文档以及视频教程(附带我自己的学习经历, 0基础可复刻), 充分利用小智原有框架实现AI+物联网项目

![mmexport1756607081683](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509101607750.jpg)

## 课程特点

+ 给出我自己的学习路线, 可以进行学习
+ 涉及面广
+ 全部开源
+ 有交流群(up还在上学, 只能保证尽量回答)

### 关于提问

1. 提问之前请确保你已经看过并完全理解视频内容，且严格按照视频操作执行。 
2. 提问之前请通过大语言模型（例如豆包，DeepSeek，腾讯元宝等）、搜索引擎等方式尝试自己解决。 
3. 提问时请避免使用“有人能帮我吗？”，“为什么会这样？”等无意义的问句。 
4. 提问时请尽可能地详细描述您所遇到的问题和您的目的，并配上截图，最好能配上您的尝试过程，这通常能节约不少时间。 
5. 如果您实在不会/不想截图，也请尽可能清晰地拍屏 (避免出现摩尔纹等影响阅读的情况) 
6. 大家的回答也许不能马上解决您的问题，请保持耐心，不要着急 
7. 提问之后，如果无人回应也请不要着急，更不要频繁找管理员/群主，毕竟大家只是热心网友，不是24小时客服，我们也有自己的事情要做。

### 开发中(XvsenfengAI开发板)

+ 低功耗
+ 增强宠物交互的随机性和**趣味性**
+ 充分使用**拓展接口**, 目前预期的模块有4g模块等
+ **小程序控制台功能拓展**, 实现不同的开发板之间的娱乐互动
+ **充分利用SD卡**: 实现lvgl和SD卡配合使用以及达成播放音乐等可以实现的功能
+ 板子和手机之间双向通信
+ 充分使用陀螺仪外设

## 项目结构

下面的我们实际使用到的文件部分

```
xiaozhi-
├── main/                    # 主程序代码
│   ├── application.cc       # 应用程序主逻辑
│   ├── electronic_pet/      # 电子宠物核心模块
│   ├── audio_processing/    # 音频处理模块
│   ├── display/            # 显示控制模块
│   └── boards/             # 硬件板级支持, 目前只是适配两个板子
├── components/             # 自定义的组件库
├── note/                   # 相关的代码教程
└── ota/                    # OTA 升级模块
```

## 🚀 快速开始

### 硬件准备
1. 选择支持的开发板（立创S3开发板、我的自定义开发板）
2. 一根可以正常烧录的数据线
3. 一台电脑(用于烧录)

### 烧录

#### 使用代码编译烧录

1. 安装 ESP-IDF 5.4.1 开发环境

我使用的是IDF的插件, 可以参考一下其他人的教程

![image-20250903123217312](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509031232361.png)

[Windows：VS Code IDE安装ESP-IDF【保姆级】_windows vscode安装esp-idf-CSDN博客](https://blog.csdn.net/zsyf33078/article/details/133834900)

2. 克隆项目代码, `git clone git@github.com:XuSenfeng/electronic_pet_mcp.git`
3. 配置编译选项

选择使用的芯片以及使用的端口

![image-20250903123133906](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509031231967.png)

![image-20250903122000840](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509031220987.png)

SD卡使用的用户, 需要配置一下可以使用长文件名. 否则会出现文件读取失败的报错

![image-20250903122731182](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509031227330.png)

4. 烧录

可以直接使用火苗实现

![image-20250903131552602](https://picture-01-1316374204.cos.ap-beijing.myqcloud.com/lenovo-picture/202509031315639.png)

#### 直接烧录

使用我分享的固件, 可以直接烧录下载, 使用[ESP32烧录bin](https://blog.csdn.net/Mark_md/article/details/123413873)的方式进行下载

### 实际操作

#### 获取boardID

这个编码是板子和微信小程序进行通信的凭证, 显示在说面界面

#### 界面切换

##### 立创

使用陀螺仪模拟按键, 可以上下左右倾斜板子, 实现不同的界面的切换, 不在主界面的时候, 可以使用按键返回主界面

#### XvsenfengAI

使用左边的按键, 是原本的小智里面的按键, 可以用于不同的小智对话状态切换

中间的按键功能是用于不同界面的切换, 可以进行功能界面的改变

右侧按键是返回键, 返回上一级界面或者主界面

### 配置使用
1. 连接 Wi-Fi 网络
2. 注册 [小智官方](xiaozhi.me) 账号
3. 小程序配置各种参数
4. 开始与电子宠物互动

## 应用场景

- **智能陪伴**：提供情感陪伴
- **教育工具**：通过互动游戏学习语言和知识
- **智能家居**：作为智能家居控制中心
- **娱乐设备**：提供有趣的互动娱乐体验
- **开发学习**：AI 硬件开发的入门项目

## 开源协议

本项目采用 Apache 2.0 开源协议，允许：
- ✅ 商业使用
- ✅ 修改和分发
- ✅ 专利使用
- ✅ 私人使用

> 在使用的时候需要标注使用的来源

## 📞 社区支持

- **QQ 群**：719932592
- **小智官方网站**：https://xiaozhi.me
- **GitHub**：https://github.com/XuSenfeng/electronic_pet_mcp
- **文档中心**：https://github.com/XuSenfeng/electronic_pet_mcp/tree/master/note

## 🎥 相关视频教程

- [【原创~2025微信小程序~完整项目】最新版微信小程序-零基础到企业级收费项目速成版【本人原创-高清纯享版-其它均为盗录低清版-支持正版-从我做起】_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1WgQdYNERe/?spm_id_from=333.1387.homepage.video_card.click)

- https://www.bilibili.com/video/BV1nNr7YZESq/?spm_id_from=333.1387.homepage.video_card.click

- [Python + MySQL 0基础从入门到精通 MySQL数据库实战精讲教程（2021精华版）_哔哩哔哩_bilibili](https://www.bilibili.com/video/BV1B34y1R7in/?spm_id_from=333.1387.search.video_card.click)

    

