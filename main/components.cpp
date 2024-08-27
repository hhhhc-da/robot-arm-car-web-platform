#include "components.h"

static const char *TAG = "NANOKA";

// 夹子、左臂、右臂、机械臂底盘、大底盘、云台底盘、云台
std::map<int, std::pair<uint8_t, uint8_t>> motor = {
    {0, std::pair<uint8_t, uint8_t>({0x01, 0x00})},
    {1, std::pair<uint8_t, uint8_t>({0x01, 0x01})},
    {2, std::pair<uint8_t, uint8_t>({0x01, 0x02})},
    {3, std::pair<uint8_t, uint8_t>({0x00, 0x03})},
    {4, std::pair<uint8_t, uint8_t>({0x00, 0x04})},
    {5, std::pair<uint8_t, uint8_t>({0x01, 0x05})},
    {6, std::pair<uint8_t, uint8_t>({0x01, 0x06})},
};

////////////////////////////////////////////////  MUSIC 驱动  ///////////////////////////////////////////////////
void music_init(void)
{
    // 控制输出，不启用上下拉电阻
    gpio_set_direction(MUSIC_PIN, GPIO_MODE_OUTPUT);

    // 使用 UART2 渲染屏幕
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(UART_NUM, 1024 * 2, 0, 0, NULL, 0);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_param_config(UART_NUM, &uart_config);
}

void music_play(void)
{
    gpio_set_level(MUSIC_PIN, 1);

    const char *data = "JUMP(1)\r\n";
    uart_write_bytes(UART_NUM, data, strlen(data));

    vTaskDelay(pdMS_TO_TICKS(22000));

    music_stop();
}

void music_stop(void)
{
    const char *data = "JUMP(0)\r\n";
    uart_write_bytes(UART_NUM, data, strlen(data));

    gpio_set_level(MUSIC_PIN, 0);
}
////////////////////////////////////////////////  红外线灯驱动  ///////////////////////////////////////////////////
void red_init(void)
{
    // 控制输出，不启用上下拉电阻
    gpio_set_direction(RED_CTRL_PIN, GPIO_MODE_OUTPUT);
}

void red_control(int status)
{
    if (status)
        gpio_set_level(RED_CTRL_PIN, 1);
    else
        gpio_set_level(RED_CTRL_PIN, 0);
}
////////////////////////////////////////////////  MOTOR 驱动  ///////////////////////////////////////////////////
void control_init(void)
{
    // 使用 UART1 作为控制信号
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_driver_install(CTRL_UART_NUM, 1024 * 2, 0, 0, NULL, 0);
    uart_set_pin(CTRL_UART_NUM, CTRL_TXD_PIN, CTRL_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_param_config(CTRL_UART_NUM, &uart_config);
}

// 电机编号、模式选择 (ABS或REL)、具体值
int motor_control(std::vector<pack_t> data)
{
    uint8_t prepare[5];

    prepare[0] = data.size();
    prepare[1] = prepare[0] * 4;
    prepare[2] = 0x00;

    int16_t crc = prepare[0] + prepare[1] + prepare[2];
    prepare[3] = (uint8_t)(crc >> 8);
    prepare[4] = (uint8_t)(crc & 0xff);

    uart_write_bytes(CTRL_UART_NUM, prepare, 5);

    uint8_t ret = 0x00;
    // 这里不能一直阻塞，否则会出现问题 (设计的时候数据不定长)
    int len = uart_read_bytes(CTRL_UART_NUM, &ret, 1, pdMS_TO_TICKS(100));

    if (len == 0)
    {

        ESP_LOGW(TAG, "No valid data.");

        bool fixed = false;
        for (int i = 0; i < 20; ++i)
        {
            uart_write_bytes(CTRL_UART_NUM, prepare, 4);
            int len = uart_read_bytes(CTRL_UART_NUM, &ret, 1, pdMS_TO_TICKS(100));

            if (len != 0)
            {
                fixed = true;
                break;
            }
        }

        if (fixed)
        {
            ESP_LOGE(TAG, "Usart stall but we fixed it.");
        }
        else
        {
            ESP_LOGE(TAG, "We cannot fix usart stall.");
            return -2;
        }

        // 重试连接
        uart_write_bytes(CTRL_UART_NUM, prepare, 5);

        uint8_t ret = 0x00;
        int len = uart_read_bytes(CTRL_UART_NUM, &ret, 1, pdMS_TO_TICKS(100));

        if (len == 0)
        {
            ESP_LOGE(TAG, "We still found usart stall.");
            return -4;
        }
    }

    // 读取到有效数据之后
    if (ret == 0x2a)
    {
        ESP_LOGW(TAG, "Usart1 success.");

        uint8_t *stream = (uint8_t *)malloc(prepare[1] * sizeof(uint8_t));

        for (uint8_t i = 0; i < prepare[0]; ++i)
        {
            std::pair<uint8_t, uint8_t> info = motor[std::get<0>(data[i])];
            stream[4 * i] = info.first;
            stream[4 * i + 1] = info.second;

            stream[4 * i + 2] = std::get<1>(data[i]);
            stream[4 * i + 3] = std::get<2>(data[i]);
        }

        uart_write_bytes(CTRL_UART_NUM, stream, prepare[1]);

        uint8_t ret2;
        int len = uart_read_bytes(CTRL_UART_NUM, &ret2, 1, pdMS_TO_TICKS(20000));
        free(stream);

        if (len == 0)
        {
            ESP_LOGW(TAG, "Device execute failed.");
            return -3;
        }

        ESP_LOGW(TAG, "Device execute done.");
        return 0;
    }
    else
    {
        ESP_LOGW(TAG, "Usart1 failed.");
        return -1;
    }
}

//////////////////////////////////////////////  WIFI 驱动  ////////////////////////////////////////////////
/* 缝合的，不要乱改 */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        switch (event_id)
        {
        case WIFI_EVENT_STA_START: // WIFI以STA模式启动后触发此事件
            esp_wifi_connect();    // 启动WIFI连接
            break;
        case WIFI_EVENT_STA_CONNECTED: // WIFI连上路由器后，触发此事件
            ESP_LOGI(TAG, "connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED: // WIFI从路由器断开连接后触发此事件
            ESP_LOGI(TAG, "Disconnect reason: %d", event->reason);
            esp_wifi_connect(); // 继续重连
            ESP_LOGI(TAG, "connect to the AP fail,retry now");
            break;
        default:
            break;
        }
    }
    if (event_base == IP_EVENT) // IP相关事件
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP: // 只有获取到路由器分配的IP，才认为是连上了路由器
            ESP_LOGI(TAG, "get ip address");
            break;
        }
    }
}

esp_err_t wifi_sta_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                // 用于初始化tcpip协议栈
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建一个默认系统事件调度循环，之后可以注册回调函数来处理系统的一些事件
    esp_netif_create_default_wifi_sta();              // 使用默认配置创建STA对象

    // 初始化WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // WIFI配置
    wifi_config_t wifi_config =
        {
            .sta =
                {
                    .ssid = DEFAULT_WIFI_SSID,         // WIFI的SSID
                    .password = DEFAULT_WIFI_PASSWORD, // WIFI密码
                    .threshold = {
                        .authmode = WIFI_AUTH_WPA2_PSK}, // 加密方式

                    .pmf_cfg = {.capable = true, .required = false},
                },
        };

    // 启动WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // 设置工作模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 设置wifi配置
    ESP_ERROR_CHECK(esp_wifi_start());                               // 启动WIFI

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    return ESP_OK;
}

void handle_client_task(void *pt)
{
    int len;
    char rx_buffer[128]; // 接收的数据
    char resp[128];      // TX buffer
    int sock = (int)pt;  // 创建任务传过来的参数（sock句柄）
    int connecttag;      // 断开连接结束整个任务的标记
    while (1)
    {
        do
        {
            len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0); // 接收(有数据时>0)
            if (len < 0)
            {
                ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
            }
            else if (len == 0)
            {
                ESP_LOGW(TAG, "Connection closed");
                shutdown(sock, 0);
                close(sock);
                connecttag = 1;
            }
            else
            {
                rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
                ESP_LOGI(TAG, " Received %d bytes: %s", len, rx_buffer);

                sprintf(resp, "FATAL ERROR: CMD NOT EXECUTE.");
                // 前面这里要加上处理函数，但是要先进行解析
                int ret = arm_exec_cmd(rx_buffer, len, resp);

                // send() can return less bytes than supplied length. Send()返回的字节数可能少于提供的长度
                // Walk-around for robust implementation.
                int to_write = strlen(resp);
                while (to_write > 0)
                {
                    // 从这里给 Client 返回消息
                    int written = send(sock, resp, to_write, 0);
                    if (written < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    }
                    to_write -= written; // to_write = 0
                }
            }
        } while (len > 0); // 有数据接收就继续循环

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (connecttag)
        {
            break; // 跳出最外面的while
        }
    }
    vTaskDelete(NULL); // 删除整个task，不然会触发看门狗
}

void tcp_server_config(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    int keepAlive = 1;
    int keepIdle = KEEPALIVE_IDLE;
    int keepInterval = KEEPALIVE_INTERVAL;
    int keepCount = KEEPALIVE_COUNT;
    struct sockaddr_storage dest_addr;
    // 如果返回的是IPV4地址（AF_INET在taskcreate里就传参了 pvParameters ->(void *)AF_INET ）
    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY); // 将一个32位数从主机字节顺序转换成网络字节顺序（返回值是一个32位的网络字节顺序）
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT); // 将一个16位数从主机字节顺序转换成网络字节顺序（返回值是一个16位的网络字节顺序）
        ip_protocol = IPPROTO_IP;
    }
#ifdef CONFIG_EXAMPLE_IPV6
    else if (addr_family == AF_INET6)
    {
        struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
        bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
        dest_addr_ip6->sin6_family = AF_INET6;
        dest_addr_ip6->sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }
#endif
    ESP_LOGI(TAG, "1.Initializing socket..........");
    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0)
    {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 获取的嵌套字，选择所在的协议层，设置的选项（允许复用本地地址和端口），存放选项待设置的新值的缓冲区，缓冲区长度
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif
    ESP_LOGI(TAG, "Socket created");

    ESP_LOGI(TAG, "2.Binding port..........");
    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0)
    {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    ESP_LOGI(TAG, "3.Listening client..........");
    err = listen(listen_sock, 1); // 同一时间最多可以收到1个客户端请求
    if (err != 0)
    {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket listening");
    ESP_LOGI(TAG, "4.Create pipe..........");
    while (1)
    {
        struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
        socklen_t addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len); // 等待客户端连接，如果一直没有客户端连接将一直阻塞（成功：返回嵌套字 失败：return -1）
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));      // SOL_SOCKET通用套接字选项；SO_KEEPALIVE保持连接；keepalive非0值，开启keepalive属性
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));      // IPPROTO_TCP TCP选项；TCP_KEEPIDLE 设置允许没有数据传输时（空闲）的时间
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int)); // TCP_KEEPINTVL 当超过允许的空闲时间仍然没有数据传输时，每隔TCP_KEEPINTVL设定的时间发送一次探测包
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));      // TCP_KEEPCNT 允许发送探测包最多次数

        // Convert ip address to string 将ip地址转换为字符串
        // 判断存放地址sockaddr_storage结构体里的ss_family是否为ipv4的地址
        if (source_addr.ss_family == PF_INET)
        {
            // 把地址sin_addr转成字符串存到缓冲区addr_str映射到lwip并返回地址
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
        }
#ifdef CONFIG_EXAMPLE_IPV6
        else if (source_addr.ss_family == PF_INET6)
        {
            inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
#endif
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        xTaskCreate(handle_client_task, "handle_client_task", 4096, (void *)sock, 5, NULL);
        // shutdown(sock, 0);
        // ESP_LOGE(TAG,"shutdown");
        // close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void usertcp_server(void)
{
#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_config, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(tcp_server_config, "tcp_server", 4096, (void *)AF_INET6, 5, NULL);
#endif
}

void components_setup(void *)
{
    ESP_LOGW(TAG, "Components setup start.");

    // 初始化 NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    wifi_sta_init();

    red_init();
    music_init();
    control_init();

    const char *data = "JUMP(0)\r\n";
    uart_write_bytes(UART_NUM, data, strlen(data));

    // wifi_init_sta();

    usertcp_server();
    ESP_LOGW(TAG, "Initialization Done.");
}
