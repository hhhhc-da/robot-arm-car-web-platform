#ifndef _COMPONENTS_H
#define _COMPONENTS_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <utility>
#include <map>
#include <set>
#include <tuple>
#include <exception>
#include <cmath>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/gptimer.h"
#include "driver/uart.h"
#include "driver/adc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"

#include "esp_task_wdt.h"
#include "esp_log.h"
#include "sys/param.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
// #include "protocol_examples_common.h"
// #include "addr_from_stdin.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include <nvs_flash.h>

#include "picojson.h"

typedef std::tuple<int, uint8_t, uint8_t> pack_t;
// �����е��ִ�к���
extern int arm_exec_cmd(const char *cmd, int size, char *resp);

/////////////////////////////////////////////  MUSIC  //////////////////////////////////////////////////
#ifndef _MUSIC_H
#define _MUSIC_H

// ǿ�������ֵĿ�������
#define MUSIC_PIN GPIO_NUM_19

#ifndef MUSIC_TRIGGER_PIN
// ���ֿ�������
#define MUSIC_TRIGGER_PIN GPIO_NUM_25
#endif

void music_init(void);
void music_play(void);
void music_stop(void);
#endif

/////////////////////////////////////////////  CTRL  //////////////////////////////////////////////////
#ifndef _CTRL_H
#define _CTRL_H

// �������ܽ�
#define CTRL_UART_NUM UART_NUM_1
#define CTRL_TXD_PIN GPIO_NUM_18
#define CTRL_RXD_PIN GPIO_NUM_5

#define ABS (uint8_t)0x00
#define REL (uint8_t)0x01

void control_init(void);
int motor_control(std::vector<pack_t> data);

#endif

/////////////////////////////////////////////  LVGL  //////////////////////////////////////////////////
#ifndef _LVGL_H
#define _LVGL_H

// LVGL ��������
#define LVGL_MOTOR_NUM 7
#define LVGL_CONBOBOX_NUM 3

// �����ܽ�
#define UART_NUM UART_NUM_2
#define TXD_PIN GPIO_NUM_17
#define RXD_PIN GPIO_NUM_16

#define LVGL_W GPIO_NUM_13
#define LVGL_A GPIO_NUM_14
#define LVGL_S GPIO_NUM_12
#define LVGL_D GPIO_NUM_27
#define LVGL_ENTER GPIO_NUM_26
#define LVGL_ADC GPIO_NUM_33

#endif

void components_setup(void *);

#endif

/////////////////////////////////////////////  WIFI  //////////////////////////////////////////////////
#ifndef _WIFI_H
#define _WIFI_H

#define PORT 8080            // CONFIG_EXAMPLE_PORT//�˿ں�
#define KEEPALIVE_IDLE 5     // CONFIG_EXAMPLE_KEEPALIVE_IDLE //tcp�������������û�����ݷ��͵Ļ�����ú���keepalive̽����飨s��
#define KEEPALIVE_INTERVAL 5 // CONFIG_EXAMPLE_KEEPALIVE_INTERVAL//tcpǰ������̽��֮���ʱ������s��
#define KEEPALIVE_COUNT 5    // CONFIG_EXAMPLE_KEEPALIVE_COUNT //tcp�ر�һ���ǻ�Ծ����֮ǰ��������Դ�����s��

#define CONFIG_EXAMPLE_IPV4

#define DEFAULT_WIFI_SSID "Hin"
#define DEFAULT_WIFI_PASSWORD "12345678n"

#endif

////////////////////////////////////////////  �����ߵ�  //////////////////////////////////////////////
#ifndef _RED_H
#define _RED_H

#define RED_ON 1
#define RED_OFF 0

// �����ߵƿ��ƽ��� GPIO21
#define RED_CTRL_PIN GPIO_NUM_21

void red_init(void);
void red_control(int status);

#endif