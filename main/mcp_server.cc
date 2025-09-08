/*
 * MCP Server Implementation
 * Reference: https://modelcontextprotocol.io/specification/2024-11-05
 */

#include "mcp_server.h"
#include <esp_log.h>
#include <esp_app_desc.h>
#include <algorithm>
#include <cstring>

#include "application.h"
#include "display.h"
#include "board.h"
#include "esp32_camera.h"

#define TAG "MCP"

McpServer::McpServer() {
}

McpServer::~McpServer() {
    for (auto tool : tools_) {
        delete tool;
    }
    tools_.clear();
}

void McpServer::AddCommonTools() {
    // To speed up the response time, we add the common tools to the beginning of
    // the tools list to utilize the prompt cache.
    // Backup the original tools list and restore it after adding the common tools.
    auto original_tools = std::move(tools_);
    auto& board = Board::GetInstance();

    AddTool("self.get_device_status",
        "Provides the real-time information of the device, including the current status of the audio speaker, screen, battery, network, pet's happyness, socer, game state etc.\n"
        "When a user inquires about a specific status, you must use the tool to obtain the latest status.\n"
        "You can also use this tool get weather pet can upgrade or get what it is doing.\n"
        "Use this tool for: \n"
        "1. Answering questions about current condition (e.g. what is the current volume of the audio speaker?)\n"
        "2. As the first step to control the device (e.g. turn up / down the volume of the audio speaker, etc.)\n"
        "3. As the first step to control the Pet, get the status of the pet. (e.g. The current satiety level of the pet, etc.)\n",
        PropertyList(),
        [&board](const PropertyList& properties) -> ReturnValue {
            return board.GetDeviceStatusJson();
        });

    AddTool("self.pet.get_follow_list",
        "返回宠物的关注列表。使用此工具可以获取当前宠物关注的所有对象（如其他宠物、用户等）的详细信息。",
        PropertyList(),
        [&board](const PropertyList& properties) -> ReturnValue {
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            return pet->GetFocusListJson();
        });

    AddTool("self.pet.SendMessageToFollow",
        "使用关注列表中的名字向指定对象发送消息。参数包括目标名字（name）和消息内容（message）。\n"
        "如果关注列表中存在该名字，则向其发送消息，否则返回失败。",
        PropertyList({
            Property("name", kPropertyTypeString),
            Property("message", kPropertyTypeString)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            std::string target_name = properties["name"].value<std::string>();
            std::string message = properties["message"].value<std::string>();
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            // 查找关注列表
            std::string target_boardID;
            bool found = false;
            auto focus_list = pet->GetFocusList();
            // 查找目标并发送
            for (const auto& focus : focus_list) {
                if (focus.name == target_name) {
                    target_boardID = focus.boardID;
                    found = true;
                    break;
                }
            }
            if (!found) {
                return "{\"success\": false, \"message\": \"未找到该关注对象\"}";
            }
            // 构造消息JSON
            auto root = cJSON_CreateObject();
            cJSON_AddNumberToObject(root, "type", 2); // 2表示关注对象消息
            cJSON_AddStringToObject(root, "msg", message.c_str());
            cJSON_AddStringToObject(root, "from", pet->GetBoardID().c_str());
            std::string json_str = cJSON_PrintUnformatted(root);
            cJSON_Delete(root);
            ESP_LOGI(TAG, "Send message to follow: %s", json_str.c_str());
            // 清除消息发送事件
            xEventGroupClearBits(pet->message_send_event_, MESSAGE_SEND_EVENT);
            pet->GetClient()->Publish_Message(target_boardID, json_str);
            int ret = xEventGroupWaitBits(pet->message_send_event_, MESSAGE_SEND_EVENT, pdFALSE, pdTRUE, 1000);
            if(ret == pdPASS){
                ESP_LOGI(TAG, "消息已发送, 对方已收到");
                return "{\"success\": true, \"message\": \"消息已发送\"}";
            }else{
                ESP_LOGE(TAG, "消息发送失败, 可能对方已下线");
                return "{\"success\": false, \"message\": \"消息发送失败, 可能对方已下线\"}";
            }
        });


    AddTool("self.audio_speaker.set_volume", 
        "Set the volume of the audio speaker. If the current volume is unknown, you must call `self.get_device_status` tool first and then call this tool.",
        PropertyList({
            Property("volume", kPropertyTypeInteger, 0, 100)
        }), 
        [&board](const PropertyList& properties) -> ReturnValue {
            auto codec = board.GetAudioCodec();
            codec->SetOutputVolume(properties["volume"].value<int>());
            return true;
        });
    
    auto backlight = board.GetBacklight();
    if (backlight) {
        AddTool("self.screen.set_brightness",
            "Set the brightness of the screen.",
            PropertyList({
                Property("brightness", kPropertyTypeInteger, 0, 100)
            }),
            [backlight](const PropertyList& properties) -> ReturnValue {
                uint8_t brightness = static_cast<uint8_t>(properties["brightness"].value<int>());
                backlight->SetBrightness(brightness, true);
                return true;
            });
    }

#ifndef CONFIG_BOARD_TYPE_XVSENFAI
    AddTool("self.camera.take_photo",
        "Take a photo and explain it. Use this tool after the user asks you to see something.\n"
        "Args:\n"
        "  `question`: The question that you want to ask about the photo.\n"
        "Return:\n"
        "  A JSON object that provides the photo information.",
        PropertyList({
            Property("question", kPropertyTypeString)
        }),
        [](const PropertyList& properties) -> ReturnValue {

            camera_config_t config = {};
            config.ledc_channel = LEDC_CHANNEL_2;  // LEDC通道选择  用于生成XCLK时钟 但是S3不用
            config.ledc_timer = LEDC_TIMER_2; // LEDC timer选择  用于生成XCLK时钟 但是S3不用
            config.pin_d0 = CAMERA_PIN_D0;
            config.pin_d1 = CAMERA_PIN_D1;
            config.pin_d2 = CAMERA_PIN_D2;
            config.pin_d3 = CAMERA_PIN_D3;
            config.pin_d4 = CAMERA_PIN_D4;
            config.pin_d5 = CAMERA_PIN_D5;
            config.pin_d6 = CAMERA_PIN_D6;
            config.pin_d7 = CAMERA_PIN_D7;
            config.pin_xclk = CAMERA_PIN_XCLK;
            config.pin_pclk = CAMERA_PIN_PCLK;
            config.pin_vsync = CAMERA_PIN_VSYNC;
            config.pin_href = CAMERA_PIN_HREF;
            config.pin_sccb_sda = -1;   // 这里写-1 表示使用已经初始化的I2C接口
            config.pin_sccb_scl = CAMERA_PIN_SIOC;
            config.sccb_i2c_port = 1;
            config.pin_pwdn = CAMERA_PIN_PWDN;
            config.pin_reset = CAMERA_PIN_RESET;
            config.xclk_freq_hz = XCLK_FREQ_HZ;
            config.pixel_format = PIXFORMAT_RGB565;
            config.frame_size = FRAMESIZE_VGA;
            config.jpeg_quality = 12;
            config.fb_count = 1;
            config.fb_location = CAMERA_FB_IN_PSRAM;
            config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
            Esp32Camera* camera_ = nullptr;
            camera_ = new Esp32Camera(config);

            if (!camera_->Capture()) {
                return "{\"success\": false, \"message\": \"Failed to capture photo\"}";
            }
            auto question = properties["question"].value<std::string>();
            std::string result = camera_->Explain(question);
            // ESP_LOGI(TAG, "Camera explain result: %s", result.c_str());
            delete  camera_;
            return result;
        });
#endif

    AddTool("self.pet.SetAction",
        "让喵喵去做一些事情, 比如睡觉, 散步, 学习, 工作之类的, 通常为*你去*开头\n示例```用户:你现在去睡觉\n设置参数为2睡觉```"
        "设置状态的时候必须使用这个函数刷新喵喵的状态, 这个函数会更新喵喵的状态, 并且会在屏幕上显示喵喵正在做什么.\n"
        "Args:\n"
        "`ActionNum` : 0:空闲, 什么都不做, 2:睡大觉喽, 3:出来散步, 4:洗个澡, 5:工作赚钱, 6:学习一会, 7:听音乐",
        PropertyList({
            Property("ActionNum", kPropertyTypeInteger, 0, E_PET_ACTION_NUMBER - 1)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            int action = static_cast<uint8_t>(properties["ActionNum"].value<int>());
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return false;
            }
            if (action >= E_PET_ACTION_NUMBER) {
                ESP_LOGE(TAG, "Invalid action number: %d", action);
                return false;
            }
            pet->SetAction(action);
            ESP_LOGI(TAG, "Set action to %d", action);
            return true;
        });
    
    AddTool("self.pet.SetGameHP",
        "During the game mode, use this function for updating the health value display. This function must be called every time for updating, with the attribute being an integer from 0 to 100.\n"
        "Every time change HP, you must use this tool to update the HP value.\n",
        PropertyList({
            Property("HP", kPropertyTypeInteger, 0, 100)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            auto display = Board::GetInstance().GetDisplay();
            int hp = static_cast<uint8_t>(properties["HP"].value<int>());
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            if(pet->isGame() == false) {
                ESP_LOGE(TAG, "Not in game mode");
                return "{\"success\": false, \"message\": \"Not in game mode\"}";
            }
            pet->setGameState(E_PET_GAME_STATE_HP, hp);
            display->UpdateGameStateGui();
            return "{\"success\": true, \"message\": \"HP updated successfully\"}";
        });

    AddTool("self.pet.SetGameEnergy",
        "During the game mode, use this function for updating the energy value display. This function must be called every time for updating, with the attribute being an integer from 0 to 100.\n"
        "Every time change Energy, you must use this tool to update the Energy value.\n",
        PropertyList({
            Property("Energy", kPropertyTypeInteger, 0, 100)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            int energy = static_cast<uint8_t>(properties["Energy"].value<int>());
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            if(pet->isGame() == false) {
                ESP_LOGE(TAG, "Not in game mode");
                return "{\"success\": false, \"message\": \"Not in game mode\"}";
            }
            pet->setGameState(E_PET_GAME_STATE_ENERGY, energy);
            auto display = Board::GetInstance().GetDisplay();
            display->UpdateGameStateGui();
            return "{\"success\": true, \"message\": \"Energy updated successfully\"}";
        });

    AddTool("self.pet.SetGameScore",
        "During the game mode, use this function for updating the energy value display. This function must be called every time for updating, with the attribute being an integer from 0 to 9999.\n"
        "Every time change Score, you must use this tool to update the Score value.\n",
        PropertyList({
            Property("Score", kPropertyTypeInteger, 0, 9999)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            int score = static_cast<uint8_t>(properties["Score"].value<int>());
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            if(pet->isGame() == false) {
                ESP_LOGE(TAG, "Not in game mode");
                return "{\"success\": false, \"message\": \"Not in game mode\"}";
            }
            pet->setGameState(E_PET_GAME_STATE_SCORE, score);
            auto display = Board::GetInstance().GetDisplay();
            display->UpdateGameStateGui();
            return "{\"success\": true, \"message\": \"Score updated successfully\"}";
        });

    AddTool("self.pet.SetGameFame",
        "During the game mode, use this function for updating the energy value display. This function must be called every time for updating, with the attribute being an integer from 0 to 999.\n"
        "Every time change Fame, you must use this tool to update the Fame value.\n",
        PropertyList({
            Property("Fame", kPropertyTypeInteger, 0, 999)
        }),
        [](const PropertyList& properties) -> ReturnValue {
            int fame = static_cast<uint8_t>(properties["Fame"].value<int>());
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            if(pet->isGame() == false) {
                ESP_LOGE(TAG, "Not in game mode");
                return "{\"success\": false, \"message\": \"Not in game mode\"}";
            }
            pet->setGameState(E_PET_FAME_STATE_FAME, fame);
            auto display = Board::GetInstance().GetDisplay();
            display->UpdateGameStateGui();
            return "{\"success\": true, \"message\": \"Fame updated successfully\"}";
        });

    AddTool("self.pet.GetUpdateTask",
        "Upgrade task, call this function to trigger the upgrade event when an upgrade is currently available.\n"
        "This function will return a string that describes the upgrade task, such as \"You need to answer 3 questions correctly to upgrade\".\n",
        PropertyList(),
        [](const PropertyList& properties) -> ReturnValue {
            ElectronicPet* pet = ElectronicPet::GetInstance();
            std::string task = pet->GetUpdateTask();
            return "{\"success\": true, \"task\": \"" + task + "\"}";
        });


    AddTool("self.pet.Upgrade",
        "After the user completes the upgrade task, call this function to trigger the upgrade event.\n"
        "This function will return the describes of the upgrade result\n",
        PropertyList(),
        [](const PropertyList& properties) -> ReturnValue {
            ElectronicPet* pet = ElectronicPet::GetInstance();
            if (pet == nullptr) {
                ESP_LOGE(TAG, "ElectronicPet instance is null");
                return "{\"success\": false, \"message\": \"Pet is death\"}";
            }
            if(pet->Upgrade()){
                return "{\"success\": true, \"message\": \"Upgrade success\"}";
            } else {
                return "{\"success\": false, \"message\": \"Upgrade failed\"}";
            }
        });
    // Restore the original tools list to the end of the tools list
    tools_.insert(tools_.end(), original_tools.begin(), original_tools.end());
}

void McpServer::AddTool(McpTool* tool) {
    // Prevent adding duplicate tools
    if (std::find_if(tools_.begin(), tools_.end(), [tool](const McpTool* t) { return t->name() == tool->name(); }) != tools_.end()) {
        ESP_LOGW(TAG, "Tool %s already added", tool->name().c_str());
        return;
    }

    ESP_LOGI(TAG, "Add tool: %s", tool->name().c_str());
    tools_.push_back(tool);
}

void McpServer::AddTool(const std::string& name, const std::string& description, const PropertyList& properties, std::function<ReturnValue(const PropertyList&)> callback) {
    AddTool(new McpTool(name, description, properties, callback));
}

void McpServer::ParseMessage(const std::string& message) {
    cJSON* json = cJSON_Parse(message.c_str());
    if (json == nullptr) {
        ESP_LOGE(TAG, "Failed to parse MCP message: %s", message.c_str());
        return;
    }
    ParseMessage(json);
    cJSON_Delete(json);
}

void McpServer::ParseCapabilities(const cJSON* capabilities) {
    auto vision = cJSON_GetObjectItem(capabilities, "vision");
    if (cJSON_IsObject(vision)) {
        auto url = cJSON_GetObjectItem(vision, "url");
        auto token = cJSON_GetObjectItem(vision, "token");
        if (cJSON_IsString(url)) {
            // auto camera = Board::GetInstance().GetCamera();
            // if (camera) {
            std::string url_str = std::string(url->valuestring);
            std::string token_str;
            if (cJSON_IsString(token)) {
                token_str = std::string(token->valuestring);
            }
            Esp32Camera::SetExplainUrlStatic(url_str, token_str);
            ESP_LOGI(TAG, "Set camera explain URL: %s, token: %s", url_str.c_str(), token_str.c_str());
            // }
        }
    }
}

void McpServer::ParseMessage(const cJSON* json) {
    // Check JSONRPC version
    auto version = cJSON_GetObjectItem(json, "jsonrpc");
    if (version == nullptr || !cJSON_IsString(version) || strcmp(version->valuestring, "2.0") != 0) {
        ESP_LOGE(TAG, "Invalid JSONRPC version: %s", version ? version->valuestring : "null");
        return;
    }
    
    // Check method
    auto method = cJSON_GetObjectItem(json, "method");
    if (method == nullptr || !cJSON_IsString(method)) {
        ESP_LOGE(TAG, "Missing method");
        return;
    }
    
    auto method_str = std::string(method->valuestring);
    if (method_str.find("notifications") == 0) {
        return;
    }
    
    // Check params
    auto params = cJSON_GetObjectItem(json, "params");
    if (params != nullptr && !cJSON_IsObject(params)) {
        ESP_LOGE(TAG, "Invalid params for method: %s", method_str.c_str());
        return;
    }

    auto id = cJSON_GetObjectItem(json, "id");
    if (id == nullptr || !cJSON_IsNumber(id)) {
        ESP_LOGE(TAG, "Invalid id for method: %s", method_str.c_str());
        return;
    }
    auto id_int = id->valueint;
    
    if (method_str == "initialize") {
        if (cJSON_IsObject(params)) {
            auto capabilities = cJSON_GetObjectItem(params, "capabilities");
            if (cJSON_IsObject(capabilities)) {
                ParseCapabilities(capabilities);
            }
        }
        auto app_desc = esp_app_get_description();
        std::string message = "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"" BOARD_NAME "\",\"version\":\"";
        message += app_desc->version;
        message += "\"}}";
        ReplyResult(id_int, message);
    } else if (method_str == "tools/list") {
        std::string cursor_str = "";
        if (params != nullptr) {
            auto cursor = cJSON_GetObjectItem(params, "cursor");
            if (cJSON_IsString(cursor)) {
                cursor_str = std::string(cursor->valuestring);
            }
        }
        GetToolsList(id_int, cursor_str);
    } else if (method_str == "tools/call") {
        if (!cJSON_IsObject(params)) {
            ESP_LOGE(TAG, "tools/call: Missing params");
            ReplyError(id_int, "Missing params");
            return;
        }
        auto tool_name = cJSON_GetObjectItem(params, "name");
        if (!cJSON_IsString(tool_name)) {
            ESP_LOGE(TAG, "tools/call: Missing name");
            ReplyError(id_int, "Missing name");
            return;
        }
        auto tool_arguments = cJSON_GetObjectItem(params, "arguments");
        if (tool_arguments != nullptr && !cJSON_IsObject(tool_arguments)) {
            ESP_LOGE(TAG, "tools/call: Invalid arguments");
            ReplyError(id_int, "Invalid arguments");
            return;
        }
        DoToolCall(id_int, std::string(tool_name->valuestring), tool_arguments);
    } else {
        ESP_LOGE(TAG, "Method not implemented: %s", method_str.c_str());
        ReplyError(id_int, "Method not implemented: " + method_str);
    }
}

void McpServer::ReplyResult(int id, const std::string& result) {
    std::string payload = "{\"jsonrpc\":\"2.0\",\"id\":";
    payload += std::to_string(id) + ",\"result\":";
    payload += result;
    payload += "}";
    Application::GetInstance().SendMcpMessage(payload);
}

void McpServer::ReplyError(int id, const std::string& message) {
    std::string payload = "{\"jsonrpc\":\"2.0\",\"id\":";
    payload += std::to_string(id);
    payload += ",\"error\":{\"message\":\"";
    payload += message;
    payload += "\"}}";
    Application::GetInstance().SendMcpMessage(payload);
}

void McpServer::GetToolsList(int id, const std::string& cursor) {
    const int max_payload_size = 8000;
    std::string json = "{\"tools\":[";
    
    bool found_cursor = cursor.empty();
    auto it = tools_.begin();
    std::string next_cursor = "";
    
    while (it != tools_.end()) {
        // 如果我们还没有找到起始位置，继续搜索
        if (!found_cursor) {
            if ((*it)->name() == cursor) {
                found_cursor = true;
            } else {
                ++it;
                continue;
            }
        }
        
        // 添加tool前检查大小
        std::string tool_json = (*it)->to_json() + ",";
        if (json.length() + tool_json.length() + 30 > max_payload_size) {
            // 如果添加这个tool会超出大小限制，设置next_cursor并退出循环
            next_cursor = (*it)->name();
            break;
        }
        
        json += tool_json;
        ++it;
    }
    
    if (json.back() == ',') {
        json.pop_back();
    }
    
    if (json.back() == '[' && !tools_.empty()) {
        // 如果没有添加任何tool，返回错误
        ESP_LOGE(TAG, "tools/list: Failed to add tool %s because of payload size limit", next_cursor.c_str());
        ReplyError(id, "Failed to add tool " + next_cursor + " because of payload size limit");
        return;
    }

    if (next_cursor.empty()) {
        json += "]}";
    } else {
        json += "],\"nextCursor\":\"" + next_cursor + "\"}";
    }
    
    ReplyResult(id, json);
}

void McpServer::DoToolCall(int id, const std::string& tool_name, const cJSON* tool_arguments) {
    auto tool_iter = std::find_if(tools_.begin(), tools_.end(), 
                                 [&tool_name](const McpTool* tool) { 
                                     return tool->name() == tool_name; 
                                 });
    
    if (tool_iter == tools_.end()) {
        ESP_LOGE(TAG, "tools/call: Unknown tool: %s", tool_name.c_str());
        ReplyError(id, "Unknown tool: " + tool_name);
        return;
    }

    PropertyList arguments = (*tool_iter)->properties();
    for (auto& argument : arguments) {
        bool found = false;
        if (cJSON_IsObject(tool_arguments)) {
            auto value = cJSON_GetObjectItem(tool_arguments, argument.name().c_str());
            if (argument.type() == kPropertyTypeBoolean && cJSON_IsBool(value)) {
                argument.set_value<bool>(value->valueint == 1);
                found = true;
            } else if (argument.type() == kPropertyTypeInteger && cJSON_IsNumber(value)) {
                argument.set_value<int>(value->valueint);
                found = true;
            } else if (argument.type() == kPropertyTypeString && cJSON_IsString(value)) {
                argument.set_value<std::string>(value->valuestring);
                found = true;
            }
        }

        if (!argument.has_default_value() && !found) {
            ESP_LOGE(TAG, "tools/call: Missing valid argument: %s", argument.name().c_str());
            ReplyError(id, "Missing valid argument: " + argument.name());
            return;
        }
    }

    Application::GetInstance().Schedule([this, id, tool_iter, arguments = std::move(arguments)]() {
        try {
            ReplyResult(id, (*tool_iter)->Call(arguments));
        } catch (const std::runtime_error& e) {
            ESP_LOGE(TAG, "tools/call: %s", e.what());
            ReplyError(id, e.what());
        }
    });
}