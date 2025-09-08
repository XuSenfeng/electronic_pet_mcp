# 电子宠物服务器 API 文档

## 项目概述

这是一个基于Django的电子宠物服务器项目，提供用户管理、宠物管理、商店系统、微信小程序支持等功能。

## 基础URL

```
https://xvsenfeng.top/
```

## 静态文件访问

```
http://xvsenfeng.top/uploads/{path}
```

---

## 1. 用户管理 API (`/users/`)

### 1.1 获取用户信息

- **URL**: `/users/user/`
- **方法**: `GET`
- **描述**: 获取用户基本信息
- **响应**:

```json
{
  "code": 200,
  "msg": "获取用户信息成功"
}
```

### 1.2 用户登录

- **URL**: `/users/user/login/`
- **方法**: `POST`
- **描述**: 微信小程序用户登录
- **请求体**:

```json
{
  "code": "微信登录code"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "用户已登录",
  "data": {
    "openID": "用户openID",
    "username": "用户名",
    "avatar": "头像URL",
    "is_VIP": false
  }
}
```

### 1.3 用户注册

- **URL**: `/users/user/register/`
- **方法**: `POST`
- **描述**: 用户注册（暂未实现）

### 1.4 用户登出

- **URL**: `/users/user/logout/`
- **方法**: `POST`
- **描述**: 用户登出（暂未实现）

### 1.5 上传用户头像

- **URL**: `/users/user/info/`
- **方法**: `POST`
- **描述**: 上传用户头像
- **请求体**:

```json
{
  "openid": "用户openID",
  "file_data": "base64编码的图片数据",
  "file_ext": "图片扩展名"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "上传头像成功",
  "data": {
    "avatar": "头像URL"
  }
}
```

### 1.6 更新用户信息

- **URL**: `/users/user/update/`
- **方法**: `POST`
- **描述**: 更新用户信息（暂未实现）

### 1.7 获取用户宠物列表

- **URL**: `/users/user/pet/`
- **方法**: `POST`
- **描述**: 获取用户的所有宠物
- **请求体**:

```json
{
  "openID": "用户openID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "获取宠物信息成功",
  "data": {
    "pet_list": [
      {
        "boardID": "宠物板ID",
        "name": "宠物名称"
      }
    ]
  }
}
```

---

## 2. 宠物管理 API (`/pets/`)

### 2.1 获取宠物信息

- **URL**: `/pets/pet/`
- **方法**: `GET`
- **参数**: `boardID` - 宠物板ID
- **描述**: 根据板ID获取宠物信息
- **响应**:

```json
{
  "code": 200,
  "msg": "获取宠物信息成功"
}
```

### 2.2 创建宠物

- **URL**: `/pets/pet/`
- **方法**: `POST`
- **描述**: 创建新宠物
- **请求体**:

```json
{
  "openID": "用户openID",
  "name": "宠物名称",
  "boardID": "宠物板ID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "创建宠物成功",
  "data": {
    "pet": "宠物ID",
    "shop": "商店ID"
  }
}
```

### 2.3 删除宠物

- **URL**: `/pets/pet/`
- **方法**: `DELETE`
- **参数**: 
    - `boardID` - 宠物板ID
    - `openID` - 用户openID
- **描述**: 删除宠物
- **响应**:

```json
{
  "code": 200,
  "msg": "删除宠物成功"
}
```

### 2.4 获取宠物状态

- **URL**: `/pets/pet/state/`
- **方法**: `GET`
- **参数**: `boardID` - 宠物板ID
- **描述**: 获取宠物当前状态
- **响应**:

```json
{
  "code": 200,
  "msg": "获取宠物状态成功",
  "data": {
    "pet": "宠物ID",
    "vigor": "活力值",
    "satiety": "饱食度",
    "happiness": "快乐值",
    "iq": "智商值",
    "level": "等级",
    "exp": "经验值"
  }
}
```

### 2.5 更新宠物状态

- **URL**: `/pets/pet/state/`
- **方法**: `POST`
- **描述**: 更新宠物状态
- **请求体**:

```json
{
  "boardID": "宠物板ID",
  "vigor": "活力值",
  "satiety": "饱食度",
  "happiness": "快乐值",
  "iq": "智商值",
  "level": "等级",
  "exp": "经验值"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "更新宠物状态成功",
  "data": {
    "pet": "宠物ID"
  }
}
```

### 2.6 获取日程列表

- **URL**: `/pets/schedule/`
- **方法**: `GET`
- **参数**: `boardID` - 宠物板ID
- **描述**: 获取宠物的日程安排
- **响应**:

```json
{
  "code": 200,
  "msg": "获取日程成功",
  "data": [
    {
      "id": "日程ID",
      "tm_sec": "秒",
      "tm_min": "分钟",
      "tm_hour": "小时",
      "re_mday": "重复天数",
      "re_mon": "重复月份",
      "re_year": "重复年份",
      "re_wday": "重复星期",
      "random_l": "随机下限",
      "random_h": "随机上限",
      "function_id": "功能ID",
      "message": "消息内容"
    }
  ]
}
```

### 2.7 创建日程

- **URL**: `/pets/schedule/`
- **方法**: `POST`
- **描述**: 为宠物创建新日程
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "宠物板ID",
  "event": {
    "tm_sec": "秒",
    "tm_min": "分钟",
    "tm_hour": "小时",
    "re_mday": "重复天数",
    "re_mon": "重复月份",
    "re_year": "重复年份",
    "re_wday": "重复星期",
    "random_l": "随机下限",
    "random_h": "随机上限",
    "function_id": "功能ID",
    "message": "消息内容"
  }
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "创建日程成功",
  "data": {
    "schedule": "日程ID"
  }
}
```

### 2.8 删除日程

- **URL**: `/pets/schedule/`
- **方法**: `DELETE`
- **描述**: 删除指定日程
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "宠物板ID",
  "id": "日程ID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "删除日程成功"
}
```

### 2.9 获取关注列表

- **URL**: `/pets/focus/`
- **方法**: `GET`
- **参数**: `boardID` - 宠物板ID
- **描述**: 获取宠物的关注列表
- **响应**:

```json
{
  "code": 200,
  "msg": "获取关注列表成功",
  "data": [
    {
      "id": "被关注宠物板ID",
      "name": "被关注宠物名称",
      "is_mutual": "是否互相关注"
    }
  ]
}
```

### 2.10 关注宠物

- **URL**: `/pets/focus/`
- **方法**: `PUT`
- **描述**: 关注其他宠物
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "当前宠物板ID",
  "focusID": "要关注的宠物板ID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "关注成功"
}
```

### 2.11 取消关注

- **URL**: `/pets/focus/`
- **方法**: `DELETE`
- **描述**: 取消关注宠物
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "当前宠物板ID",
  "focusID": "要取消关注的宠物板ID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "取消关注成功"
}
```

### 2.12 获取提示列表

- **URL**: `/pets/prompts/`
- **方法**: `GET`
- **参数**: `boardID` - 宠物板ID
- **描述**: 获取宠物的提示列表
- **响应**:

```json
{
  "code": 200,
  "msg": "获取提示成功",
  "data": [
    {
      "id": "提示ID",
      "title": "提示标题",
      "description": "提示描述",
      "prompt": "提示内容"
    }
  ]
}
```

### 2.13 添加提示

- **URL**: `/pets/prompts/`
- **方法**: `POST`
- **描述**: 为宠物添加新提示
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "宠物板ID",
  "title": "提示标题",
  "description": "提示描述",
  "prompt": "提示内容"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "添加提示成功"
}
```

### 2.14 删除提示

- **URL**: `/pets/prompts/`
- **方法**: `DELETE`
- **描述**: 删除指定提示
- **请求体**:

```json
{
  "openID": "用户openID",
  "boardID": "宠物板ID",
  "id": "提示ID"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "删除提示成功"
}
```

---

## 3. 商店管理 API (`/shops/`)

### 3.1 获取商店物品列表

- **URL**: `/shops/shoplists/`
- **方法**: `GET`
- **参数**: 
    - `boardID` - 宠物板ID（必需）
    - `pageNum` - 页码（可选，每页10条）
- **描述**: 获取商店的物品列表
- **响应**:

```json
[
  {
    "id": "物品ID",
    "name": "物品名称",
    "money": "价格",
    "description": "物品描述",
    "num": "数量",
    "vigor": "活力值",
    "satiety": "饱食度",
    "happiness": "快乐值",
    "iq": "智商值",
    "level": "等级"
  }
]
```

### 3.2 添加物品到商店

- **URL**: `/shops/shoplists/`
- **方法**: `POST`
- **描述**: 向商店添加新物品
- **请求体**:

```json
{
  "boardID": "宠物板ID",
  "new_thing": {
    "name": "物品名称",
    "money": "价格",
    "description": "物品描述",
    "num": "数量",
    "vigor": "活力值",
    "satiety": "饱食度",
    "happiness": "快乐值",
    "iq": "智商值",
    "level": "等级"
  }
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "物品添加成功"
}
```

### 3.3 删除商店物品

- **URL**: `/shops/shoplists/`
- **方法**: `DELETE`
- **参数**: 
    - `thingID` - 物品ID
    - `boardID` - 宠物板ID
- **描述**: 从商店删除指定物品
- **响应**:

```json
{
  "code": 200,
  "msg": "thing deleted successfully"
}
```

---

## 4. 微信小程序 API (`/wechatapp/`)

### 4.1 获取通知列表

- **URL**: `/wechatapp/noticeslist/`
- **方法**: `GET`
- **描述**: 获取系统通知列表
- **响应**:

```json
[
  {
    "id": "通知ID",
    "title": "通知标题",
    "link": "链接地址",
    "isExternal": "是否外部链接",
    "is_link": "是否为链接"
  }
]
```

### 4.2 获取欢迎页

- **URL**: `/wechatapp/welcome/`
- **方法**: `GET`
- **描述**: 获取欢迎页图片
- **响应**:

```json
{
  "code": 200,
  "msg": "success",
  "data": "图片URL"
}
```

### 4.3 获取轮播图

- **URL**: `/wechatapp/banner/`
- **方法**: `GET`
- **描述**: 获取轮播图列表（最多3张）
- **响应**:

```json
{
  "code": 200,
  "msg": "success",
  "data": ["图片URL1", "图片URL2", "图片URL3"]
}
```

### 4.4 提交建议

- **URL**: `/wechatapp/suggestion/`
- **方法**: `POST`
- **描述**: 用户提交建议
- **请求体**:

```json
{
  "openID": "用户openID",
  "name": "姓名",
  "content": "建议内容",
  "additional_info": "附加信息",
  "email_or_phone": "邮箱或电话",
  "budget": "预算"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "success",
  "data": "建议ID"
}
```

### 4.5 提交意见

- **URL**: `/wechatapp/advice/`
- **方法**: `POST`
- **描述**: 用户提交意见反馈
- **请求体**:

```json
{
  "openID": "用户openID",
  "type": "意见类型",
  "content": "意见内容",
  "email_or_phone": "邮箱或电话"
}
```

- **响应**:

```json
{
  "code": 200,
  "msg": "success",
  "data": "意见ID"
}
```

### 4.6 获取版本信息

- **URL**: `/wechatapp/version/`
- **方法**: `GET`
- **描述**: 获取应用版本信息
- **响应**:

```json
{
  "code": 200,
  "msg": "success",
  "data": {
    "lichuang": {
      "file": "文件URL",
      "version": "版本号"
    },
    "XvsenfengAI": {
      "file": "文件URL",
      "version": "版本号"
    }
  }
}
```

---

## 5. MQTT 通信 API (`/mqtt/`)

### 5.1 发送MQTT消息

- **URL**: `/mqtt/mqtt/`
- **方法**: `POST`
- **描述**: 发送MQTT消息到指定主题
- **请求体**:

```json
{
  "topic": "MQTT主题",
  "msg": "消息内容（可以是字符串、对象或数组）"
}
```

- **响应**:

```json
{
  "code": "返回码",
  "msg": "成功发送"
}
```

---

## 错误码说明

| 错误码 | 说明           |
| ------ | -------------- |
| 200    | 请求成功       |
| 201    | 创建成功       |
| 400    | 请求参数错误   |
| 404    | 资源未找到     |
| 500    | 服务器内部错误 |

## 注意事项

1. 文件上传接口需要将文件转换为base64格式
2. 分页查询默认每页10条记录
3. 用户权限验证基于openID
4. VIP用户最多可拥有7只宠物，普通用户最多3只
5. 每个宠物最多可添加30条提示
6. 商店物品数量上限为100个

## 更新日志

- 2025-09-7: 初始版本API文档创建