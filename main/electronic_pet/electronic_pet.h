/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Please set LastEditors
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once
#include "electronic_mqtt.h"
#include <atomic>
#include <string>
#include "electronic_food.h"
#include "electronic_pet_timer.h"
#include <vector>
#include "string.h"
#include "electronic_config.h"
typedef struct {
    int value;
    char name[30];
}state_t;

// 游戏信息结构
typedef struct {
    std::string name;
    std::string desc;
    std::string message;
} GameInfo;

typedef struct {
    std::string name;
    std::string boardID;
    bool mutual_focus;
} FocueInfo;

#define MESSAGE_SEND_EVENT (1 << 0)
#define HTTP_EVENT (1 << 1)

class ElectronicPet {
private:
    std::mutex mutex_;
    state_t state_[E_PET_STATE_NUMBER];     // 当前状态
    int game_state_[E_PET_GAME_STATE_NUMBER]; // 游戏状态
    
    std::vector<BaseThing *> things_; // 物品列表
    std::vector<FocueInfo> focus_list_; // 关注列表
    electronic_pet_action_e action_; // 当前行动
    electronic_pet_action_e action_last_; // 上次行动

    int current_game_ = 0;
    int is_game_ = 0;
    int level_ = 0; // 等级
    int experience_ = 0; // 经验值
    

    PMQTT_Clinet *client_; // MQTT客户端
    bool use_web_server_ = false;
    std::string boardID;

public:
    EventGroupHandle_t message_send_event_ = nullptr;
    std::vector<GameInfo> games_;
    static int state_time_change_[E_PET_ACTION_NUMBER][E_PET_STATE_NUMBER]; // 不同状态下边宠物的状态变化
    static std::string action_name_[E_PET_ACTION_NUMBER]; // 不同状态下边宠物的状态变化
    ElectronicPetTimer *timer;
    static ElectronicPet* MyPet;
    ElectronicPet();
    ~ElectronicPet();

    PMQTT_Clinet *GetClient() { return client_; }


    int GetAction() const { return action_; } // 获取当前的行为
    void SetAction(int action); // 设置当前的行为
    int GetState(int state) const { return state_[state].value; } // 获取当前的某一个状态
    void SetState(int state, int value); // 设置当前的某一个状态
    void SetStateName(int state, const char* name) { strcpy(state_[state].name, name); } // 设置状态的名字
    char *GetStateName(int state) { return state_[state].name; } // 获取某个状态的名字
    char *GetActionName() { return (char *)action_name_[action_].c_str(); } // 获取当前行为的名字
    // 对当前的状态实现加减
    void change_statue(int *change_state);


    // 从SD卡读取csv文件
    void ReadCsv();  // 从SD卡读取csv文件
    // 读取csv的食物
    void ReadCsvFood(int *i);
    // 创建一个物品
    bool CreatOneThing(char *name, char *description, int vigor_, int satiety_, int happiness_, int money_, int iq_, int level, int *i);
    // 读取游戏的配置文件
    void ReadCsvGames();
    // 获取有几个物品
    int GetThingsNum() { return things_.size();}
    // 获取某个物品
    BaseThing *GetThing(int i) { if (i < 0 || i >= things_.size()) return nullptr;return things_[i]; }
    // 获取当前的游戏索引
    int GetCurrentGame() { return current_game_; }
    // 设置当前的游戏索引
    void SetCurrentGame(int game) { current_game_ = game; }
    // 返回之前的状态, 主要用于游戏返回的时候
    void ReturnLastAction();


    // 设置游戏状态(用于处理游戏时候的特殊判断
    inline int isGame() const { return action_ == E_PET_ACTION_PLAY; }
    // 设置游戏的马哥粗略的
    void setGameState(int state, int value) { game_state_[state] = value; }
    int getGameState(int state) const { return game_state_[state]; }
    // 处理一下状态变换以后的回复
    void StateEventDeal();


    // 经验以及升级
    int getLevel() { return level_; }
    int getExperience() { return experience_; }
    // 判断是不是可以升级(升级部分是不完善的)
    inline bool isUpGraded(){return experience_ >= level_ * level_;}
    std::string ReadUpgradeTaskCsv(int level);
    std::string GetUpdateTask(void);
    bool Upgrade(void);
    
    // 在其他文件用于获取宠物句柄
    static ElectronicPet* GetInstance();


    // 获取板子的ID用于和微信小程序配合
    std::string GetBoardID(void);
    // 读取各种消息
    void ReadWebThings(void);
    void ReadWebFood(int *thing_num);
    void ReadWebGames(void);
    void ReadWebFocus(void);
    std::string GetFocusListJson(void);
    std::vector<FocueInfo> GetFocusList(void){return focus_list_;};
    // 更新当前的状态
    void UploadState(void);
};
