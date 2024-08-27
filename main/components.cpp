#include "components.h"

static const char *TAG = "NANOKA";

// ���ӡ���ۡ��ұۡ���е�۵��̡�����̡���̨���̡���̨
std::map<int, std::pair<uint8_t, uint8_t>> motor = {
    {0, std::pair<uint8_t, uint8_t>({0x01, 0x00})},
    {1, std::pair<uint8_t, uint8_t>({0x01, 0x01})},
    {2, std::pair<uint8_t, uint8_t>({0x01, 0x02})},
    {3, std::pair<uint8_t, uint8_t>({0x00, 0x03})},
    {4, std::pair<uint8_t, uint8_t>({0x00, 0x04})},
    {5, std::pair<uint8_t, uint8_t>({0x01, 0x05})},
    {6, std::pair<uint8_t, uint8_t>({0x01, 0x06})},
};

////////////////////////////////////////////////  MUSIC ����  ///////////////////////////////////////////////////
void music_init(void)
{
    // �������������������������
    gpio_set_direction(MUSIC_PIN, GPIO_MODE_OUTPUT);

    // ʹ�� UART2 ��Ⱦ��Ļ
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
////////////////////////////////////////////////  �����ߵ�����  ///////////////////////////////////////////////////
void red_init(void)
{
    // �������������������������
    gpio_set_direction(RED_CTRL_PIN, GPIO_MODE_OUTPUT);
}

void red_control(int status)
{
    if (status)
        gpio_set_level(RED_CTRL_PIN, 1);
    else
        gpio_set_level(RED_CTRL_PIN, 0);
}
////////////////////////////////////////////////  MOTOR ����  ///////////////////////////////////////////////////
void control_init(void)
{
    // ʹ�� UART1 ��Ϊ�����ź�
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

// �����š�ģʽѡ�� (ABS��REL)������ֵ
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
    // ���ﲻ��һֱ������������������ (��Ƶ�ʱ�����ݲ�����)
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

        // ��������
        uart_write_bytes(CTRL_UART_NUM, prepare, 5);

        uint8_t ret = 0x00;
        int len = uart_read_bytes(CTRL_UART_NUM, &ret, 1, pdMS_TO_TICKS(100));

        if (len == 0)
        {
            ESP_LOGE(TAG, "We still found usart stall.");
            return -4;
        }
    }

    // ��ȡ����Ч����֮��
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

//////////////////////////////////////////////  WIFI ����  ////////////////////////////////////////////////
/* ��ϵģ���Ҫ�Ҹ� */
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        switch (event_id)
        {
        case WIFI_EVENT_STA_START: // WIFI��STAģʽ�����󴥷����¼�
            esp_wifi_connect();    // ����WIFI����
            break;
        case WIFI_EVENT_STA_CONNECTED: // WIFI����·�����󣬴������¼�
            ESP_LOGI(TAG, "connected to AP");
            break;
        case WIFI_EVENT_STA_DISCONNECTED: // WIFI��·�����Ͽ����Ӻ󴥷����¼�
            ESP_LOGI(TAG, "Disconnect reason: %d", event->reason);
            esp_wifi_connect(); // ��������
            ESP_LOGI(TAG, "connect to the AP fail,retry now");
            break;
        default:
            break;
        }
    }
    if (event_base == IP_EVENT) // IP����¼�
    {
        switch (event_id)
        {
        case IP_EVENT_STA_GOT_IP: // ֻ�л�ȡ��·���������IP������Ϊ��������·����
            ESP_LOGI(TAG, "get ip address");
            break;
        }
    }
}

esp_err_t wifi_sta_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());                // ���ڳ�ʼ��tcpipЭ��ջ
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // ����һ��Ĭ��ϵͳ�¼�����ѭ����֮�����ע��ص�����������ϵͳ��һЩ�¼�
    esp_netif_create_default_wifi_sta();              // ʹ��Ĭ�����ô���STA����

    // ��ʼ��WIFI
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // ע���¼�
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // WIFI����
    wifi_config_t wifi_config =
        {
            .sta =
                {
                    .ssid = DEFAULT_WIFI_SSID,         // WIFI��SSID
                    .password = DEFAULT_WIFI_PASSWORD, // WIFI����
                    .threshold = {
                        .authmode = WIFI_AUTH_WPA2_PSK}, // ���ܷ�ʽ

                    .pmf_cfg = {.capable = true, .required = false},
                },
        };

    // ����WIFI
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));               // ���ù���ģʽΪSTA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // ����wifi����
    ESP_ERROR_CHECK(esp_wifi_start());                               // ����WIFI

    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    return ESP_OK;
}

void handle_client_task(void *pt)
{
    int len;
    char rx_buffer[128]; // ���յ�����
    char resp[128];      // TX buffer
    int sock = (int)pt;  // �������񴫹����Ĳ�����sock�����
    int connecttag;      // �Ͽ����ӽ�����������ı��
    while (1)
    {
        do
        {
            len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0); // ����(������ʱ>0)
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
                // ǰ������Ҫ���ϴ�����������Ҫ�Ƚ��н���
                int ret = arm_exec_cmd(rx_buffer, len, resp);

                // send() can return less bytes than supplied length. Send()���ص��ֽ������������ṩ�ĳ���
                // Walk-around for robust implementation.
                int to_write = strlen(resp);
                while (to_write > 0)
                {
                    // ������� Client ������Ϣ
                    int written = send(sock, resp, to_write, 0);
                    if (written < 0)
                    {
                        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    }
                    to_write -= written; // to_write = 0
                }
            }
        } while (len > 0); // �����ݽ��վͼ���ѭ��

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (connecttag)
        {
            break; // �����������while
        }
    }
    vTaskDelete(NULL); // ɾ������task����Ȼ�ᴥ�����Ź�
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
    // ������ص���IPV4��ַ��AF_INET��taskcreate��ʹ����� pvParameters ->(void *)AF_INET ��
    if (addr_family == AF_INET)
    {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY); // ��һ��32λ���������ֽ�˳��ת���������ֽ�˳�򣨷���ֵ��һ��32λ�������ֽ�˳��
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT); // ��һ��16λ���������ֽ�˳��ת���������ֽ�˳�򣨷���ֵ��һ��16λ�������ֽ�˳��
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
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // ��ȡ��Ƕ���֣�ѡ�����ڵ�Э��㣬���õ�ѡ������ñ��ص�ַ�Ͷ˿ڣ������ѡ������õ���ֵ�Ļ�����������������
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
    err = listen(listen_sock, 1); // ͬһʱ���������յ�1���ͻ�������
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
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len); // �ȴ��ͻ������ӣ����һֱû�пͻ������ӽ�һֱ�������ɹ�������Ƕ���� ʧ�ܣ�return -1��
        if (sock < 0)
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));      // SOL_SOCKETͨ���׽���ѡ�SO_KEEPALIVE�������ӣ�keepalive��0ֵ������keepalive����
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));      // IPPROTO_TCP TCPѡ�TCP_KEEPIDLE ��������û�����ݴ���ʱ�����У���ʱ��
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int)); // TCP_KEEPINTVL ����������Ŀ���ʱ����Ȼû�����ݴ���ʱ��ÿ��TCP_KEEPINTVL�趨��ʱ�䷢��һ��̽���
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));      // TCP_KEEPCNT ������̽���������

        // Convert ip address to string ��ip��ַת��Ϊ�ַ���
        // �жϴ�ŵ�ַsockaddr_storage�ṹ�����ss_family�Ƿ�Ϊipv4�ĵ�ַ
        if (source_addr.ss_family == PF_INET)
        {
            // �ѵ�ַsin_addrת���ַ����浽������addr_strӳ�䵽lwip�����ص�ַ
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

    // ��ʼ�� NVS
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
