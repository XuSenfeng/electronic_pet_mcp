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

class ElectronicPet {
private:
    std::mutex mutex_;
    state_t state_[E_PET_STATE_NUMBER];     // 当前状态
    int game_state_[E_PET_GAME_STATE_NUMBER]; // 游戏状态
    
    std::vector<BaseThing *> things_; // 物品列表
    std::vector<FocueInfo> focus_list_; // 关注列表
    electronic_pet_action_e action_; // 当前行动
    electronic_pet_action_e action_last_; // 上次行动

    int current_game = 0;
    int is_game_ = 0;
    int level_ = 0; // 等级
    int experience_ = 0; // 经验值
    

    PMQTT_Clinet *client_; // MQTT客户端
    bool use_web_server_ = false;
    std::string boardID;

public:
    std::vector<GameInfo> games_;
    static int state_time_change_[E_PET_ACTION_NUMBER][E_PET_STATE_NUMBER]; // 不同状态下边宠物的状态变化
    static std::string action_name_[E_PET_ACTION_NUMBER]; // 不同状态下边宠物的状态变化
    ElectronicPetTimer *timer;
    static ElectronicPet* MyPet;
    ElectronicPet();
    ~ElectronicPet();

    PMQTT_Clinet *GetClient() { return client_; }


    int GetAction() const { return action_; }
    void SetAction(int action);
    int GetState(int state) const { return state_[state].value; }
    void SetState(int state, int value);
    void SetStateName(int state, const char* name) { strcpy(state_[state].name, name); }
    char *GetStateName(int state) { return state_[state].name; }
    char *GetActionName() { return (char *)action_name_[action_].c_str(); }
    void ReadCsvThings();
    void ReadCsvFood(int *i);
    void ReadCsvGames();
    int GetThingsNum() { return things_.size(); }
    BaseThing *GetThing(int i) { return things_[i]; }
    int GetCurrentGame() { return current_game; }
    void SetCurrentGame(int game) { current_game = game; }
    void ReturnLastAction();
    inline int isGame() const { return is_game_; }
    inline void setGame(int game) { is_game_ = game; }
    void setGameState(int state, int value) { game_state_[state] = value; }
    int getGameState(int state) const { return game_state_[state]; }
    void StateEventDeal();
    int getLevel() { return level_; }
    int getExperience() { return experience_; }
    bool CreatOneThing(char *name, char *description, int vigor_, int satiety_, int happiness_, int money_, int iq_, int level, int *i);
    std::string GetStateDescriptor();

    inline bool isUpGraded(){return experience_ >= level_ * level_;}
    std::string ReadUpgradeTaskCsv(int level);
    
    static ElectronicPet* GetInstance();

    void change_statue(int *change_state);
    std::string GetUpdateTask(void);
    bool Upgrade(void);


    std::string GetBoardID(void);
    void ReadWebThings(void);
    void ReadWebFood(int *thing_num);
    void ReadWebGames(void);
    void ReadWebFocus(void);
    std::string GetFocusListJson(void);
    std::vector<FocueInfo> GetFocusList(void){return focus_list_;};
};
