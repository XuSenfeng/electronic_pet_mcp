/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */
#pragma once

typedef enum{
    E_PET_ACTION_IDLE = 0, // 空闲
    E_PET_ACTION_PLAY, // 玩耍
    E_PET_ACTION_SLEEP, // 睡觉
    E_PET_ACTION_WALK, // 散步
    E_PET_ACTION_BATH, // 洗澡
    E_PET_ACTION_WORK, // 工作
    E_PET_ACTION_STUDY, // 学习
    E_PET_ACTION_PLAY_MUSIC, // 听音乐
    E_PET_ACTION_NUMBER // 状态数量
}electronic_pet_action_e;

typedef enum{
    E_PET_STATE_VIGIR = 0, // 精力
    E_PET_STATE_SATITY, // 饱食度
    E_PET_STATE_HAPPINESS, // 快乐度
    // E_PET_STATE_GOOD_LINE 之前的数值用于判断状态好坏
    E_PET_STATE_IQ, // 智商
    // E_PET_DIVIDING_LINE 之前的数字有上限100
    E_PET_STATE_MONEY, // 金钱
    E_PET_STATE_NUMBER // 状态数量
}electronic_pet_state_e;

typedef enum{
    E_PET_GAME_STATE_HP = 0, // 血量
    E_PET_GAME_STATE_SCORE, // 分数
    E_PET_GAME_STATE_ENERGY, // 能量
    E_PET_FAME_STATE_FAME, // 名声
    E_PET_GAME_STATE_NUMBER // 状态数量
}electronic_pet_game_state_e;

#define E_PET_STATE_DIVIDING_LINE 4 // 分割线
#define E_PET_STATE_GOOD_LINE 3 // 分割线