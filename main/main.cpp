/*
 *
 *	                   _ooOoo_
 *	                  o8888888o
 *	                  88" . "88
 *	                  (| -_- |)
 *	                  O\  =  /O
 *	               ____/`---'\____
 *	             .'  \\|     |//  `.
 *	            /  \\|||  :  |||//  \
 *	           /  _||||| -:- |||||-  \
 *	           |   | \\\  -  /// |   |
 *	           | \_|  ''\-/''  |   |
 *	           \  .-\__  `-`  ___/-. /
 *	         ___`. .'  /-.-\  `. . __
 *	      ."" '<  `.___\_<|>_/___.'  >'"".
 *	     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *	     \  \ `-.   \_ __\ /__ _/   .-` /  /
 *	======`-.____`-.___\_____/___.-`____.-'======
 *	                   `=-='
 *
 *
 *
 *		佛祖保佑在我负责这段代码的时候不要出问题
 *
 *					南无阿弥陀佛
 *
 */

/* 前排提示: 使用手机热点作为连接媒介时千万不要连 WLAN 否则会报 201 */
/* 初步怀疑是热点信道干扰或热点2.4G信号干扰 */

#include <cstdio>
#include <inttypes.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include "robotic_arm.h"

#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"

static const char *TAG = "NANOKA";

// 外部函数声明
extern void components_setup(void *);
extern void music_play(void);

// 心跳灯
#define LED_PIN GPIO_NUM_2

// 用于展示是否需要输出 UART 信息
bool play_flag = false;
// LVGL 刷屏信号
bool lvgl_reflush = true;
// MOTOR 执行信号
bool motor_flag = false;

// 缓存状态
uint8_t motor_id[LVGL_MOTOR_NUM] = {0, 1, 2, 3, 4, 5, 6};
uint8_t mode[LVGL_MOTOR_NUM] = {ABS, ABS, ABS, REL, REL, ABS, ABS};
// 当前数据
uint8_t opcode = 0x80;
// LVGL 控制数组: dim_1 表示电机选择, dim_2 表示标签选择
static uint8_t lvgl_display_pos[2] = {0x00, 0x00};

// 任务句柄
TaskHandle_t pxHeartLed, pxMain, pxInitialization, pxMusic, pxMotor, pxADC, pxX;

////////////////////////////////////////////////////////////////////////////////////////////////
// 任务函数
void ESP32_Init(void *);
void GPIO_Loop_Task(void *args);
void Button_Detect(void *);
void MUSIC_PLAY(void *);
void LVGL_DISPLAY(void *);
void MOTOR_EXEC(void *);
void ADC_UPDATE(void *);
void UART_TASK_RECIEVE(void *);

// 调试函数
void ROBOTIC_ARM_TEST(void *);
void printEspInfo(void *args);
extern "C" void ErrorHandler(void);

////////////////////////////////////////////////////////////////////////////////////////////////
// 主任务
extern "C" void app_main(void)
{
    // 进入初始化任务
    xTaskCreatePinnedToCore(ESP32_Init, "Initialzation", 4096, NULL, 1, &pxInitialization, 1);
}

// GPIO_NUM_13 中断函数，不能带有触发中断的函数，并且要放在 RAM 内
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t trigger_gpio = (uint32_t)arg;

    switch (trigger_gpio)
    {
    // 音乐控制
    case (uint32_t)MUSIC_TRIGGER_PIN:
        play_flag = true;
        break;
    // LVGL 按键控制
    case (uint32_t)LVGL_W:
        if (!lvgl_reflush)
        {
            lvgl_display_pos[1] = (lvgl_display_pos[1] + LVGL_CONBOBOX_NUM - 1) % LVGL_CONBOBOX_NUM;
        }
        lvgl_reflush = true;
        break;
    case (uint32_t)LVGL_A:
        if (!lvgl_reflush)
        {
            // 如果要控制电机
            if (lvgl_display_pos[1] == 0x00)
                lvgl_display_pos[0] = (lvgl_display_pos[0] + LVGL_MOTOR_NUM - 1) % LVGL_MOTOR_NUM;
            // 如果要控制 MODE
            else if (lvgl_display_pos[1] == 0x01)
            {
                if (mode[lvgl_display_pos[0]] == ABS)
                    mode[lvgl_display_pos[0]] = REL;
                else
                    mode[lvgl_display_pos[0]] = ABS;
            }
            // 如果要控制操作数
            else
            {
                if (opcode > 0x00)
                    opcode -= 0x01;
            }
        }
        // 置位刷新
        lvgl_reflush = true;
        break;
    case (uint32_t)LVGL_S:
        if (!lvgl_reflush)
        {
            lvgl_display_pos[1] = (lvgl_display_pos[1] + LVGL_CONBOBOX_NUM + 1) % LVGL_CONBOBOX_NUM;
        }
        lvgl_reflush = true;
        break;
    case (uint32_t)LVGL_D:
        if (!lvgl_reflush)
        {
            if (lvgl_display_pos[1] == 0x00)
            {
                lvgl_display_pos[0] = (lvgl_display_pos[0] + LVGL_MOTOR_NUM + 1) % LVGL_MOTOR_NUM;
            }
            else if (lvgl_display_pos[1] == 0x01)
            {
                if (mode[lvgl_display_pos[0]] == ABS)
                    mode[lvgl_display_pos[0]] = REL;
                else
                    mode[lvgl_display_pos[0]] = ABS;
            }
            // 如果要控制操作数
            else
            {
                if (opcode < 0xff)
                    opcode += 0x01;
            }
        }
        lvgl_reflush = true;
        break;
    // 电机执行
    case (uint32_t)LVGL_ENTER:
        motor_flag = true;
        break;
    default:
        ESP_LOGE(TAG, "Function \"gpio_isr_handler\" execute failed.");
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
// ESP32 初始化函数
void ESP32_Init(void *)
{
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    // 控件初始化
    components_setup(nullptr);

    adc1_config_width(ADC_WIDTH_BIT_12);
    // 配置ADC 到 GPIO_NUM_33, 量程 0 - 3.3V 电压
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

    // 创建任务
    ESP_LOGE(TAG, "Task creating.");
    xTaskCreatePinnedToCore(GPIO_Loop_Task, "Heart LED Task", 2048, NULL, 1, &pxHeartLed, 1);
    xTaskCreatePinnedToCore(ADC_UPDATE, "ADC Update Task", 2048, NULL, 1, &pxADC, 1);
    xTaskCreatePinnedToCore(Button_Detect, "Main Detect Task", 2048, NULL, 1, &pxMain, 1);

    vTaskDelay(pdMS_TO_TICKS(100));
    // 注册音乐播放按键
    gpio_reset_pin(MUSIC_TRIGGER_PIN);
    gpio_set_direction(MUSIC_TRIGGER_PIN, GPIO_MODE_INPUT);
    gpio_set_pull_mode(MUSIC_TRIGGER_PIN, GPIO_PULLUP_ONLY);
    gpio_install_isr_service(0);
    gpio_set_intr_type(MUSIC_TRIGGER_PIN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(MUSIC_TRIGGER_PIN, gpio_isr_handler, (void *)MUSIC_TRIGGER_PIN);

    gpio_reset_pin(LVGL_W);
    gpio_set_direction(LVGL_W, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LVGL_W, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(LVGL_W, GPIO_INTR_LOW_LEVEL);
    gpio_isr_handler_add(LVGL_W, gpio_isr_handler, (void *)LVGL_W);

    gpio_reset_pin(LVGL_A);
    gpio_set_direction(LVGL_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LVGL_A, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(LVGL_A, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LVGL_A, gpio_isr_handler, (void *)LVGL_A);

    gpio_reset_pin(LVGL_S);
    gpio_set_direction(LVGL_S, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LVGL_S, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(LVGL_S, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LVGL_S, gpio_isr_handler, (void *)LVGL_S);

    gpio_reset_pin(LVGL_D);
    gpio_set_direction(LVGL_D, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LVGL_D, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(LVGL_D, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LVGL_D, gpio_isr_handler, (void *)LVGL_D);

    gpio_reset_pin(LVGL_ENTER);
    gpio_set_direction(LVGL_ENTER, GPIO_MODE_INPUT);
    gpio_set_pull_mode(LVGL_ENTER, GPIO_PULLUP_ONLY);
    gpio_set_intr_type(LVGL_ENTER, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(LVGL_ENTER, gpio_isr_handler, (void *)LVGL_ENTER);

    // 打印 chip 信息
    // xTaskCreatePinnedToCore(printEspInfo, "Initialzation", 2048, NULL, 1, &pxInitialization, 1);
    // ESP_LOGE(TAG, "ESP32 initialization exit.");

    // xTaskCreatePinnedToCore(ROBOTIC_ARM_TEST, "Robotic Arm Task", 4096, NULL, 1, &pxX, 1);
    // ESP_LOGE(TAG, "Created robotic_arm task.");

    ESP_LOGE(TAG, "ESP32 initialization done.");
    vTaskDelete(nullptr);
}

// 心跳灯任务
void GPIO_Loop_Task(void *args)
{
    while (true)
    {
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(LED_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(700));
    }
}

// 单次音乐播放任务
void MUSIC_PLAY(void *)
{
    if (pxADC != nullptr)
    {
        vTaskDelete(pxADC);
        pxADC = nullptr;

        music_play();

        xTaskCreatePinnedToCore(ADC_UPDATE, "ADC Update Task", 2048, NULL, 1, &pxADC, 1);
    }
    vTaskDelete(nullptr);
}

// LVGL 刷屏任务
void LVGL_DISPLAY(void *)
{
    uint8_t id = motor_id[lvgl_display_pos[0]];
    char *mode_str;
    if (mode[lvgl_display_pos[0]] == ABS)
    {
        mode_str = "Abs";
    }
    else
    {
        mode_str = "Rel";
    }

    char str[128];
    sprintf(str, "SET_TXT(4,'');SET_TXT(5,'');SET_TXT(6,'');SET_NUM(0,%u,1);SET_TXT(7,'%s');SET_TXT(8,'%02x');SET_TXT(%u,'->');\r\n", (unsigned)id, mode_str, opcode, 4 + (unsigned)lvgl_display_pos[1]);
    uart_write_bytes(UART_NUM, str, strlen(str));

    vTaskDelete(nullptr);
}

// ADC 电压读取任务
void ADC_UPDATE(void *)
{
    while (true)
    {
        // 只要没有选中这一行，我们就通过 ADC 来修改 opcode
        if (lvgl_display_pos[1] != 0x02)
        {
            int raw = adc1_get_raw(ADC1_CHANNEL_5);
            opcode = (uint8_t)round(raw / 4095.0 * 256);
            // ESP_LOGE(TAG, "opcode: %x", opcode);
        }

        lvgl_reflush = true;
        // 不要太快，会阻塞按钮响应
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// 按键中断检测任务
void Button_Detect(void *)
{
    while (true)
    {
        // 如果要求播放音乐
        if (play_flag == true)
        {
            ESP_LOGW(TAG, "Music task created.");
            xTaskCreatePinnedToCore(MUSIC_PLAY, "Music Play Task", 2048, NULL, 1, &pxMusic, 1);
        }
        // 如果要求刷屏
        if (lvgl_reflush == true)
        {
            // ESP_LOGW(TAG, "LVGL reflush task created.");
            xTaskCreatePinnedToCore(LVGL_DISPLAY, "LVGL Display Task", 2048, NULL, 1, &pxMusic, 1);
        }
        if (motor_flag == true)
        {
            ESP_LOGW(TAG, "Motor task created.");
            xTaskCreatePinnedToCore(MOTOR_EXEC, "Motor Exec Task", 2048, NULL, 1, &pxMotor, 1);
        }

        // 延时阻塞，用于防止多次创建任务, 这个值千万不要乱改
        vTaskDelay(pdMS_TO_TICKS(25));
        play_flag = false;
        lvgl_reflush = false;
        motor_flag = false;

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// 电机执行任务
void MOTOR_EXEC(void *)
{
    std::vector<pack_t> data;
    data.push_back({motor_id[lvgl_display_pos[0]], mode[lvgl_display_pos[0]], opcode});

    int ret = -1;
    ret = motor_control(data);
    ESP_LOGW(TAG, "MOTOR_TEST return: %d", ret);

    vTaskDelete(nullptr);
}

// 读取命令函数
void UART_TASK_RECIEVE(void *)
{
}

// 机械臂API测试任务
void ROBOTIC_ARM_TEST(void *)
{
    int ret = -1;
    // ret = arm_reset();
    // if (ret != 0)
    // {
    //     ESP_LOGE(TAG, "arm_reset: %d", ret);
    //     vTaskDelete(nullptr);
    // }

    while (true)
    {
        ret = -1;

        // 参数计算测试
        // std::pair<uint8_t, uint8_t> ret;
        // ret = _arm_parameters_compute({80, 0, 120});
        // ESP_LOGW(TAG, "{80, x, 120} params: (%d, %d)", ret.first, ret.second);
        // ret = _arm_parameters_compute({80, 0, 90});
        // ESP_LOGW(TAG, "{80, x, 90} params: (%d, %d)", ret.first, ret.second);
        // ret = _arm_parameters_compute({138.56, 0, 120});
        // ESP_LOGW(TAG, "{128.56, x, 120} params: (%d, %d)", ret.first, ret.second);

        // 上电访问缓冲时间 & 循环间隔时间
        vTaskDelay(pdMS_TO_TICKS(2000));

        // 3D 球坐标系移动
        // ret = arm_move_cube_3d({80, 45, 125}, {80, -45, 90});

        // 2D 极坐标系移动
        ret = arm_move_cube_2d({80, 45, 125}, {138.65, -45});

        if (ret != 0)
        {
            ESP_LOGE(TAG, "Error ROBOTIC_ARM_TEST task crush.");
            break;
        }
    }

    vTaskDelete(nullptr);
}

extern "C" void ErrorHandler(void)
{
    while (1)
    {
        printf("Fatal error!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void printEspInfo(void *args)
{
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    vTaskDelete(nullptr);
}
