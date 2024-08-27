#ifndef _ROBOTIC_ARM_H
#define _ROBOTIC_ARM_H

#include "components.h"

#define CLOSE 0
#define OPEN 1

// ��۳�ʼֵ (��ֱ�����)
#define X_0 0x36
// �ұ۳�ʼֵ (��ֱ�����)
#define Y_0 0x55
// ��е�۳� (��λ: mm)
#define LEN 80
// ��ʼֵ
#define MOTOR_0_INIT 0x00
#define MOTOR_1_INIT 0x4a
#define MOTOR_2_INIT 0x55
#define MOTOR_3_INIT 0x00

extern uint8_t motor_id[LVGL_MOTOR_NUM];
extern bool play_flag;

// ������ϵ r���ȡ���
typedef std::tuple<double, double, double> pos3d_t;
typedef std::pair<double, double> pos2d_t;
typedef std::pair<uint8_t, uint8_t> param_t;

// ��е�ۼ�ȡ, 0 �ս���1 �ſ�
int _arm_control_clip(int status);
// ��е����ת
int _arm_control_angel(int angel);
// ��е��֧��ƽ̨��ת
int _arm_platform_control(int angel);
// ��е�۲�������
param_t _arm_parameters_compute(pos3d_t pos);
// ��е���ƶ�
int _arm_control_move(pos3d_t r);
// ̽�����ƶ�
int _arm_detect_move(int theta, int angle);
// ̽�����򿪺�����
int _arm_detect_display(int status);
// ��������
int _arm_detect_music(void);

/* �߼�API */
// �ƶ���е�۽������ƶ���ָ��λ��(2d, ֻҪ�� (x, y) ����һ��)
int arm_move_cube_2d(pos3d_t old_pos, pos2d_t new_pos);
// �ƶ���е�۽������ƶ���ָ��λ��(3d, Ҫ�� (x, y, z) ����һ��)
int arm_move_cube_3d(pos3d_t old_pos, pos3d_t new_pos);
// ��е�۸�λ
int arm_reset(void);
// ��е��ִ��ָ��
int arm_exec_cmd(const char *cmd, int size, char *resp);

#endif