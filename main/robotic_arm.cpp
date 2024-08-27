#include "robotic_arm.h"

using namespace std;

static const char *TAG = "NANOKA";

// 机械臂夹取
int _arm_control_clip(int status)
{
    int ret = -1;
    // 0 表示收紧夹子
    if (status == 0)
    {
        std::vector<pack_t> data;
        // 两个并不会让 PCA9685 立刻执行
        data.push_back({motor_id[0], ABS, 0x00});

        ret = motor_control(data);
    }
    // 其他表示张开夹子
    else
    {
        std::vector<pack_t> data;
        data.push_back({motor_id[0], ABS, 0x18});

        ret = motor_control(data);
    }

    return ret;
}
// 探测器打开红外线
int _arm_detect_display(int status)
{
    red_control(status);
    return 0;
}
// 播放阴乐
int _arm_detect_music(void)
{
    play_flag = true;
    return 0;
}
// 机械臂参数计算
param_t _arm_parameters_compute(pos3d_t pos)
{
    /* 这里我们只计算左右臂参数, 依据 r、ψ, 不包括 θ */
    std::pair<uint8_t, uint8_t> ret = {0x4a, 0x55};
    // gamma = arccos(1 - r^2/ (2 * l^2))
    double gamma = (180 / M_PI) * acos(1 - pow(std::get<0>(pos), 2) / (2 * pow(LEN, 2)));
    // fai = (y - y0) + (180 - gamma) / 2
    ret.second = std::get<2>(pos) + Y_0 - (180 - gamma) / 2;
    // gamma = (y - y0) + (x - x0)
    ret.first = gamma - ret.second + Y_0 + X_0;

    return ret;
}
// 机械臂移动
int _arm_control_move(pos3d_t pos)
{
    int ret = -1;
    // 先移动底盘，然后确定位置 (底盘采用相对移动的方式)
    _arm_control_angel(std::get<1>(pos));
    // 然后计算机械臂位置, 并且计算绝对位置
    std::pair<uint8_t, uint8_t> data = _arm_parameters_compute(pos);

    std::vector<pack_t> send_data;
    send_data.push_back({motor_id[1], ABS, data.first});
    send_data.push_back({motor_id[2], ABS, data.second});

    ret = motor_control(send_data);

    return ret;
}
// 探测器移动
int _arm_detect_move(int theta, int angle)
{
    int ret = -1;
    std::vector<pack_t> data;
    data.push_back({motor_id[5], ABS, theta});
    data.push_back({motor_id[6], ABS, angle});

    return motor_control(data);
}
// 机械臂旋转 (任意角度都可以，但是要分批次才能实现)
int _arm_control_angel(int angel)
{
    int ret = -1;

    if (angel == 0)
        return 0;

    // 方向、半圈次数、剩余角度
    std::tuple<int, size_t, size_t> info;

    // 如果为 1 就是顺时针, 如果为 -1 就是逆时针
    std::get<0>(info) = angel / abs(angel);
    std::get<1>(info) = abs(angel) / 180;
    std::get<2>(info) = abs(angel) % 180;

    std::vector<pack_t> data[2];
    double percent = std::get<2>(info) / 180.0;
    uint8_t value = ceil(128 * percent);

    switch (std::get<0>(info))
    {
    case 1:
        data[0].push_back({motor_id[3], REL, 0xff});
        data[1].push_back({motor_id[3], REL, 0x80 + value});
        break;
    case -1:
        data[0].push_back({motor_id[3], REL, 0x01});
        data[1].push_back({motor_id[3], REL, 0x80 - value});
        break;
    default:
        // 计算错误
        return 1;
        break;
    }

    for (int i = 0; i < std::get<1>(info); ++i)
    {
        ret = motor_control(data[0]);
        // 执行错误
        if (ret != 0)
            return ret;
    }

    ret = motor_control(data[1]);
    // 执行错误
    if (ret != 0)
        return ret;

    return ret;
}

// 机械臂支撑平台旋转
int _arm_platform_control(int angel)
{
    int ret = -1;

    if (angel == 0)
        return 0;

    // 方向、半圈次数、剩余角度
    std::tuple<int, size_t, size_t> info;

    // 如果为 1 就是顺时针, 如果为 -1 就是逆时针
    std::get<0>(info) = angel / abs(angel);
    std::get<1>(info) = abs(angel) / 180;
    std::get<2>(info) = abs(angel) % 180;

    std::vector<pack_t> data[2];
    double percent = std::get<2>(info) / 180.0;
    uint8_t value = ceil(128 * percent);

    switch (std::get<0>(info))
    {
    case 1:
        data[0].push_back({motor_id[4], REL, 0xff});
        data[1].push_back({motor_id[4], REL, 0x80 + value});
        break;
    case -1:
        data[0].push_back({motor_id[4], REL, 0x01});
        data[1].push_back({motor_id[4], REL, 0x80 - value});
        break;
    default:
        // 计算错误
        return 1;
        break;
    }

    for (int i = 0; i < std::get<1>(info); ++i)
    {
        ret = motor_control(data[0]);
        // 执行错误
        if (ret != 0)
            return ret;
    }

    ret = motor_control(data[1]);
    // 执行错误
    if (ret != 0)
        return ret;

    return ret;
}

/* 高级API */

// 移动机械臂将物体移动到指定位置(2d, 只要求极坐标 (r, θ) 一致), h 与正常状态相同
int arm_move_cube_2d(pos3d_t old_pos, pos2d_t new_pos)
{
    pos3d_t final_pos = {new_pos.first, new_pos.second, 90};
    return arm_move_cube_3d(old_pos, final_pos);
}

// 移动机械臂将物体移动到指定位置(3d, 要求 (r、θ、ψ) 坐标一致)
int arm_move_cube_3d(pos3d_t old_pos, pos3d_t new_pos)
{
    int ret = -1;
    ret = _arm_control_clip(1);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "_arm_control_clip(1) failed.");
        return ret;
    }

    ret = _arm_control_move(old_pos);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "_arm_control_move(old_pos) failed.");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
    ret = _arm_control_clip(0);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "_arm_control_clip(0) failed.");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    ret = _arm_control_move(new_pos);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "_arm_control_move(new_pos) failed.");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    ret = _arm_control_clip(1);
    if (ret != 0)
    {
        ESP_LOGE(TAG, "_arm_control_clip(1) failed.");
        return ret;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    ret = arm_reset();
    if (ret != 0)
    {
        ESP_LOGE(TAG, "arm_reset() failed.");
        return ret;
    }

    return ret;
}

// 机械臂复位
int arm_reset(void)
{
    int ret = -1;

    // 向 MCU 发送控制数据
    std::vector<pack_t> data;
    data.push_back({motor_id[0], ABS, 0x18});
    data.push_back({motor_id[1], ABS, MOTOR_1_INIT});
    data.push_back({motor_id[2], ABS, MOTOR_2_INIT});
    data.push_back({motor_id[3], ABS, MOTOR_3_INIT});

    ret = motor_control(data);
    if (ret != 0)
        return ret;

    data.clear();
    data.push_back({motor_id[0], ABS, MOTOR_0_INIT});

    ret = motor_control(data);
    return ret;
}

// 机械臂执行指令
int arm_exec_cmd(const char *cmd, int size, char *resp)
{
    ESP_LOGI(TAG, "EXEC command \"%s\".", cmd);

    std::string json_str(cmd);

    picojson::value v;
    std::string err = picojson::parse(v, json_str);
    if (!err.empty())
    {
        ESP_LOGI(TAG, "JSON parse error: %s.", err.c_str());
        sprintf(resp, "JSON parse error: %s.", err.c_str());
        return -1;
    }

    if (!v.is<picojson::object>())
    {
        ESP_LOGI(TAG, "JSON is not an object.");
        sprintf(resp, "JSON is not an object.");
        return -2;
    }

    /* {"cmd":"xxx", "params_num":3, "params":[15,12,26]} */
    picojson::object &obj = v.get<picojson::object>();
    // 检查所有 Key 是否存在
    if (!(obj.find("cmd") != obj.end() && (obj.find("params_num") != obj.end())))
    {
        ESP_LOGI(TAG, "JSON keys not found.");
        sprintf(resp, "JSON keys not found.");
        return -3;
    }

    // ESP_LOGI(TAG, "Obj loading...");

    std::string ncmd = obj["cmd"].get<std::string>();
    int len = static_cast<int>(obj["params_num"].get<double>());

    // ESP_LOGI(TAG, "Get \"len\": %d", len);
    picojson::array params;

    if (obj.find("params") != obj.end())
    {
        params = obj["params"].get<picojson::array>();
    }
    else
    {
        if (len != 0)
        {
            ESP_LOGI(TAG, "JSON keys not found.");
            sprintf(resp, "JSON keys not found.");
            return -3;
        }
    }
    std::vector<int> buf;

    // ESP_LOGI(TAG, "Switch ncmd.");
    // {"cmd":"arm_move","params_num":3,"params":[80,0,120]}
    if (ncmd == "arm_move")
    {
        if (len == 3)
        {
            for (int i = 0; i < len; ++i)
                buf.push_back(static_cast<int>(params[i].get<double>()));

            _arm_control_move({buf[0], buf[1], buf[2]});
        }
        else
            goto RESET_LABEL;
    }
    // {"cmd":"detect_move","params_num":2,"params":[90,90]}
    else if (ncmd == "detect_move")
    {
        if (len == 2)
        {
            for (int i = 0; i < len; ++i)
                buf.push_back(static_cast<int>(params[i].get<double>()));

            _arm_detect_move(buf[0], buf[1]);
        }
        else
            goto RESET_LABEL;
    }
    // {"cmd":"platform","params_num":1, "params":[45]}
    else if (ncmd == "platform")
    {
        if (len == 1)
        {
            for (int i = 0; i < len; ++i)
                buf.push_back(static_cast<int>(params[i].get<double>()));

            _arm_platform_control(buf[0]);
        }
        else
            goto RESET_LABEL;
    }
    // {"cmd":"arm_transpose","params_num":6,"params":[80,0,120,90,0,120]}
    else if (ncmd == "arm_transpose")
    {
        if (len == 6)
        {
            for (int i = 0; i < len; ++i)
                buf.push_back(static_cast<int>(params[i].get<double>()));

            arm_move_cube_3d({buf[0], buf[1], buf[2]}, {buf[3], buf[4], buf[5]});
        }
        else
            goto RESET_LABEL;
    }
    // {"cmd":"music","params_num":0}
    else if (ncmd == "music")
    {
        _arm_detect_music();
    }
    // {"cmd":"red","params_num":1, "params":[1]}
    else if (ncmd == "red")
    {
        if (len == 1){
            for (int i = 0; i < len; ++i)
                buf.push_back(static_cast<int>(params[i].get<double>()));

            _arm_detect_display(buf[0]);
        }
        else
            goto RESET_LABEL;
    }
    else
    {
    RESET_LABEL:
        ESP_LOGI(TAG, "Fatal error: Reset.");
        sprintf(resp, "Fatal error: Reset.");
        arm_reset();
    }

    sprintf(resp, "CMD execute done.");
    return 0;
}