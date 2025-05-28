/*
 * @Descripttion: 
 * @Author: Xvsenfeng helloworldjiao@163.com
 * @LastEditors: Xvsenfeng helloworldjiao@163.com
 * Copyright (c) 2025 by helloworldjiao@163.com, All Rights Reserved. 
 */

#pragma once

#include <string>
#include "lvgl.h"
#include "electronic_base_thing.h"


class Food: public BaseThing {
private:
public:
    Food(const char * name, lv_image_dsc_t thing_pic, const char *thing_description, int *state,int num = 1)
        : BaseThing(name, thing_pic, thing_description, E_THING_TYPE_FOOD, num, state) {}
    void Use() override;
};



