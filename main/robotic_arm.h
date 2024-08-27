#ifndef _ROBOTIC_ARM_H
#define _ROBOTIC_ARM_H

#include "components.h"

#define CLOSE 0
#define OPEN 1

// 左臂初始值 (竖直情况下)
#define X_0 0x36
// 右臂初始值 (竖直情况下)
#define Y_0 0x55
// 机械臂长 (单位: mm)
#define LEN 80
// 初始值
#define MOTOR_0_INIT 0x00
#define MOTOR_1_INIT 0x4a
#define MOTOR_2_INIT 0x55
#define MOTOR_3_INIT 0x00

extern uint8_t motor_id[LVGL_MOTOR_NUM];
extern bool play_flag;

// 球坐标系 r、θ、ψ
typedef std::tuple<double, double, double> pos3d_t;
typedef std::pair<double, double> pos2d_t;
typedef std::pair<uint8_t, uint8_t> param_t;

// 机械臂夹取, 0 收紧、1 张开
int _arm_control_clip(int status);
// 机械臂旋转
int _arm_control_angel(int angel);
// 机械臂支撑平台旋转
int _arm_platform_control(int angel);
// 机械臂参数计算
param_t _arm_parameters_compute(pos3d_t pos);
// 机械臂移动
int _arm_control_move(pos3d_t r);
// 探测器移动
int _arm_detect_move(int theta, int angle);
// 探测器打开红外线
int _arm_detect_display(int status);
// 播放阴乐
int _arm_detect_music(void);

/* 高级API */
// 移动机械臂将物体移动到指定位置(2d, 只要求 (x, y) 坐标一致)
int arm_move_cube_2d(pos3d_t old_pos, pos2d_t new_pos);
// 移动机械臂将物体移动到指定位置(3d, 要求 (x, y, z) 坐标一致)
int arm_move_cube_3d(pos3d_t old_pos, pos3d_t new_pos);
// 机械臂复位
int arm_reset(void);
// 机械臂执行指令
int arm_exec_cmd(const char *cmd, int size, char *resp);

#endif